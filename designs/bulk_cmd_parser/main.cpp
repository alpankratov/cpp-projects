#include "bulk.h"
#include <iostream>
#include <vector>
#include <memory>

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

    const std::vector<IBlockSink*> sinks = { &console_sink, &file_sink };

    std::string line;
    while (std::getline(std::cin, line)) {
        Block output_block;
        if (builder.feed_line(line, output_block)) {
            for (auto* s : sinks) s->consume(output_block);
        }
    }

    Block final_block;
    if (builder.finish(final_block)) {
        for (auto* s : sinks) s->consume(final_block);
    }

    return 0;
}