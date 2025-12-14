#include "async.h"
#include "../bulk.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace async {

namespace {
struct Context {
    std::size_t bulk_size;
    BlockBuilder builder;
    std::string buffer; // stores partial line between receives
    std::vector<std::unique_ptr<IBlockSink>> sinks;

    explicit Context(std::size_t bulk)
        : bulk_size(bulk), builder(bulk) {
        sinks.emplace_back(std::make_unique<ConsoleSink>());
        sinks.emplace_back(std::make_unique<FileSink>());
    }

    void dispatch(const Block &b) {
        for (auto &s : sinks) s->consume(b);
    }
};

// Some implementations of the base tasks assume a single global static block size.
// To avoid deep refactoring, if different sizes are requested, return nullptr (as in the requirements).
std::optional<std::size_t> g_bulk_size;
}

handle_t connect(std::size_t bulk) {
    if (!g_bulk_size.has_value()) {
        g_bulk_size = bulk;
    } else if (g_bulk_size.value() != bulk) {
        return nullptr; // not supported as per requirements
    }

    auto *ctx = new Context(bulk);
    return static_cast<handle_t>(ctx);
}

void receive(handle_t handle, const char *data, std::size_t size) {
    if (!handle || !data || size == 0) return;
    auto *ctx = static_cast<Context *>(handle);

    ctx->buffer.append(data, size);

    std::size_t pos = 0;
    while (true) {
        auto nl = ctx->buffer.find('\n', pos);
        if (nl == std::string::npos) {
            // keep the remaining tail (from pos) for the next receive
            if (pos > 0) ctx->buffer.erase(0, pos);
            break;
        }
        std::string line;
        line.assign(ctx->buffer, pos, nl - pos);
        pos = nl + 1;

        if (line.empty()) {
            // ignore empty lines
            continue;
        }

        Block out;
        if (ctx->builder.feed_line(line, out)) {
            ctx->dispatch(out);
        }
    }
}

void disconnect(handle_t handle) {
    if (!handle) return;
    auto *ctx = static_cast<Context *>(handle);

    // On disconnect, process a possible final line without trailing '\n'
    if (!ctx->buffer.empty()) {
        std::string line = std::move(ctx->buffer);
        ctx->buffer.clear();
        if (!line.empty()) {
            Block out;
            if (ctx->builder.feed_line(line, out)) {
                ctx->dispatch(out);
            }
        }
    }

    // Finish pending static block
    Block out;
    if (ctx->builder.finish(out)) {
        ctx->dispatch(out);
    }

    delete ctx;
}

}
