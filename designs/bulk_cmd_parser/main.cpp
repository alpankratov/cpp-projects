#include "bulk.h"
#include "block_queue.h"
#include <iostream>
#include <vector>
#include <thread>

int main(const int argc, char* argv[])
{
    // handling static block size argument edge cases
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <block-size>\n";
        return 1;
    }

    long long tmp = 0;
    try { tmp = std::stoll(argv[1]); }
    catch (...) {
        std::cerr << "Invalid block size: " << argv[1] << '\n';
        return 1;
    }
    if (tmp <= 0) {
        std::cerr << "Block size must be a positive integer.\n";
        return 1;
    }
    const auto block_size = static_cast<std::size_t>(tmp);

    // main functionality
    BlockBuilder builder(block_size);
    ConsoleSink console_sink;
    FileSink    file_sink;

    BlockQueue<Block> file_queue;
    BlockQueue<Block> console_queue;

    std::thread t_console(console_worker, std::ref(console_queue), &console_sink);
    std::thread t1_file(file_worker, std::ref(file_queue), &file_sink);
    std::thread t2_file(file_worker, std::ref(file_queue), &file_sink);
    const std::vector<IBlockSink*> sinks = { &console_sink, &file_sink };

    std::string line;
    while (std::getline(std::cin, line)) {
        Block output_block;
        if (builder.feed_line(line, output_block)) {
            console_queue.push(output_block);
            file_queue.push(output_block);
        }

    }

    Block final_block;
    if (builder.finish(final_block)) {
        console_queue.push(final_block);
        file_queue.push(final_block);
    }

    // Stop queue and wait for workers to finish
    console_queue.stop();
    t_console.join();
    file_queue.stop();
    t1_file.join();
    t2_file.join();

    return 0;
}