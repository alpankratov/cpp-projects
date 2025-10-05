#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

// Finished block of commands
struct Block {
    std::vector<std::string> commands;
    std::time_t timestamp; // epoch seconds of the first command to include in log file name
};

// Abstraction of Block consumers
class IBlockSink {
public:
    virtual ~IBlockSink() = default;
    virtual void consume(const Block &) = 0;
};

// Concrete Consol consumer - Responsible for displaying a Block on console
class ConsoleSink final : public IBlockSink {
public:
    void consume(const Block &b) override {
        std::cout << "bulk: ";
        for (std::size_t i = 0; i < b.commands.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << b.commands[i];
        }
        std::cout << '\n';
    };
};

// Concrete File consumer - Responsible for saving a Block in a log file
class FileSink final : public IBlockSink {
public:
    void consume(const Block &b) override {
        std::ostringstream fn;
        fn << "bulk" << b.timestamp << ".log";

        std::ofstream out(fn.str(), std::ios::out | std::ios::trunc);
        if (!out) {
            std::cerr << "Error: could not open file " << fn.str() << '\n';
            return;
        }

        out << "bulk: ";
        for (std::size_t i = 0; i < b.commands.size(); ++i) {
            if (i) out << ", ";
            out << b.commands[i];
        }
        out << '\n';
    };
};

// Class responsible for building a block - the
class BlockBuilder {
public:
    explicit BlockBuilder(const std::size_t static_block_size)
        : static_block_size_(static_block_size) {
    }

    /* Process one input line. Returns true if a block became ready
     *  (i.e. the caller should forward it to the sinks). */
    bool feed_line(const std::string &line, Block &output_block) {
        // ---------- brace handling ----------
        if (line == "{") {
            if (brace_depth_ == 0) {
                // entering outermost dynamic block
                // Flush any pending static block before switching mode
                if (mode_ == Mode::STATIC && !commands_.empty()) {
                    output_block = {commands_, first_command_timestamp_};
                    reset_current_block();
                    mode_ = Mode::DYNAMIC; // will be set again below
                    ++brace_depth_;
                    return true; // a static block is ready
                }
                // start a fresh dynamic block
                reset_current_block();
                mode_ = Mode::DYNAMIC;
            }
            ++brace_depth_;
            return false; // '{' itself is not a command
        }

        if (line == "}") {
            if (brace_depth_ > 0) {
                --brace_depth_;
                if (brace_depth_ == 0) {
                    // closing outermost dynamic block
                    if (!commands_.empty()) {
                        output_block = {commands_, first_command_timestamp_};
                        reset_current_block();
                        mode_ = Mode::STATIC;
                        return true; // dynamic block finished
                    }
                    // empty dynamic block – just switch back
                    reset_current_block();
                    mode_ = Mode::STATIC;
                }
            }
            // stray '}' (depth == 0) is ignored as a normal line
            return false;
        }

        // regular command
        if (!block_started_) {
            first_command_timestamp_ = std::time(nullptr); // timestamp of first command
            block_started_ = true;
        }
        commands_.push_back(line);

        if (mode_ == Mode::STATIC) {
            ++static_block_command_count_;
            if (static_block_command_count_ == static_block_size_) {
                // static block is full
                output_block = {commands_, first_command_timestamp_};
                reset_current_block(); // prepare for next static block
                return true;
            }
        }
        // In DYNAMIC mode we simply keep collecting
        return false;
    }

    /** Called when EOF is reached – flushes a possible trailing static
     *  block. Returns true if a block was produced. */
    bool finish(Block &output_block) {
        if (mode_ == Mode::STATIC && !commands_.empty()) {
            output_block = {commands_, first_command_timestamp_};
            reset_current_block();
            return true;
        }
        // If we are still inside a dynamic block we discard it (spec requirement)
        return false;
    }

    void reset_current_block() {
        commands_.clear();
        first_command_timestamp_ = 0;
        block_started_ = false;
        static_block_command_count_ = 0;
    }

private:
    enum class Mode { STATIC, DYNAMIC };

    const std::size_t static_block_size_; // N from the command line
    Mode mode_ = Mode::STATIC;
    int brace_depth_ = 0; // nesting level of {}
    std::size_t static_block_command_count_ = 0; // #commands collected in static mode
    std::vector<std::string> commands_; // commands of the current block
    std::time_t first_command_timestamp_ = 0; // timestamp of first command
    bool block_started_ = false; // true after first real command
};
