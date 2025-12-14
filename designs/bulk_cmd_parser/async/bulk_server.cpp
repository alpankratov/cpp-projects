#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <ctime>

#include "../bulk.h"

using boost::asio::ip::tcp;

namespace
{
    // Shared sinks used by all sessions/threads
    struct Dispatcher
    {
        std::vector<std::unique_ptr<IBlockSink>> sinks;

        Dispatcher()
        {
            sinks.emplace_back(std::make_unique<ConsoleSink>());
            sinks.emplace_back(std::make_unique<FileSink>());
        }

        void dispatch(const Block& b)
        {
            for (auto& s : sinks) s->consume(b);
        }
    };

    // Aggregates STATIC-mode commands across all sessions
    class GlobalStaticAggregator
    {
    public:
        explicit GlobalStaticAggregator(std::size_t bulk_size, Dispatcher& dispatcher)
            : bulk_size_(bulk_size), dispatcher_(dispatcher)
        {
        }

        void add_command(const std::string& cmd, std::time_t ts)
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (current_.commands.empty()) current_.timestamp = ts;
            current_.commands.push_back(cmd);
            if (current_.commands.size() == bulk_size_)
            {
                dispatcher_.dispatch(current_);
                current_ = Block{}; // reset
            }
        }

        // Called when some session enters dynamic mode to avoid mixing
        void flush_if_pending()
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (!current_.commands.empty())
            {
                dispatcher_.dispatch(current_);
                current_ = Block{};
            }
        }

        // Optional, for controlled shutdowns
        void flush_on_shutdown()
        {
            flush_if_pending();
        }

    private:
        const std::size_t bulk_size_;
        Dispatcher& dispatcher_;
        std::mutex mtx_;
        Block current_{};
    };

    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        Session(tcp::socket socket, GlobalStaticAggregator& agg, Dispatcher& disp)
            : socket_(std::move(socket)), aggregator_(agg), dispatcher_(disp)
        {
        }

        void start() { do_read_line(); }

    private:
        void do_read_line()
        {
            auto self = shared_from_this();
            boost::asio::async_read_until(socket_, buffer_, '\n',
                                          [this, self](const boost::system::error_code& ec,
                                                       std::size_t bytes_transferred)
                                          {
                                              if (!ec)
                                              {
                                                  std::istream is(&buffer_);
                                                  std::string line;
                                                  std::getline(is, line); // extracts without '\n'
                                                  handle_line(line);
                                                  do_read_line();
                                              }
                                              else
                                              {
                                                  // EOF or error: process any remaining partial data as a final line
                                                  if (buffer_.size() > 0)
                                                  {
                                                      std::string tail{
                                                          boost::asio::buffer_cast<const char*>(buffer_.data()),
                                                          buffer_.size()
                                                      };
                                                      buffer_.consume(buffer_.size());
                                                      if (!tail.empty()) handle_line(tail);
                                                  }
                                                  on_disconnect();
                                              }
                                          });
        }

        void handle_line(const std::string& line)
        {
            if (line.empty()) return; // ignore empty lines

            if (line == "{")
            {
                if (brace_depth_ == 0)
                {
                    // entering outermost dynamic block — make sure global static batch is flushed
                    aggregator_.flush_if_pending();
                    // start fresh dynamic block
                    dynamic_block_ = Block{};
                }
                ++brace_depth_;
                return; // not a command
            }

            if (line == "}")
            {
                if (brace_depth_ > 0)
                {
                    --brace_depth_;
                    if (brace_depth_ == 0)
                    {
                        // close outermost dynamic block
                        if (!dynamic_block_.commands.empty())
                        {
                            dispatcher_.dispatch(dynamic_block_);
                            dynamic_block_ = Block{};
                        }
                        else
                        {
                            // empty dynamic block — nothing to emit
                        }
                    }
                }
                return; // not a command
            }

            // regular command
            const std::time_t now_ts = std::time(nullptr);
            if (brace_depth_ > 0)
            {
                if (dynamic_block_.commands.empty()) dynamic_block_.timestamp = now_ts;
                dynamic_block_.commands.push_back(line);
            }
            else
            {
                aggregator_.add_command(line, now_ts);
            }
        }

        void on_disconnect()
        {
            // If we are inside dynamic block, we do NOT auto-close it per spec; dynamic block is discarded
            // However, if it is currently open and has collected commands, we simply drop them.
            // For static mode there is nothing to do – aggregator keeps its partial state.
            boost::system::error_code ignored;
            socket_.shutdown(tcp::socket::shutdown_both, ignored);
            socket_.close(ignored);
        }

    private:
        tcp::socket socket_;
        boost::asio::streambuf buffer_;
        int brace_depth_ = 0;
        Block dynamic_block_{};
        GlobalStaticAggregator& aggregator_;
        Dispatcher& dispatcher_;
    };

    class Server
    {
    public:
        Server(boost::asio::io_context& io, unsigned short port, std::size_t bulk_size)
            : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)),
              aggregator_(bulk_size, dispatcher_)
        {
            do_accept();
        }

        void flush_on_shutdown() { aggregator_.flush_on_shutdown(); }

    private:
        void do_accept()
        {
            acceptor_.async_accept([this](const boost::system::error_code& ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<Session>(std::move(socket), aggregator_, dispatcher_)->start();
                }
                // keep accepting regardless of error to avoid stopping server on transient issues
                do_accept();
            });
        }

        boost::asio::io_context& io_;
        tcp::acceptor acceptor_;
        Dispatcher dispatcher_;
        GlobalStaticAggregator aggregator_;
    };
} // namespace

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: bulk_server <port> <bulk_size>\n";
        return 1;
    }
    const unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));
    const std::size_t bulk_size = static_cast<std::size_t>(std::stoul(argv[2]));
    try
    {
        boost::asio::io_context io;
        Server server(io, port, bulk_size);
        io.run();
        server.flush_on_shutdown();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "bulk_server error: " << ex.what() << '\n';
        return 2;
    }
    return 0;
}
