#include "bulk.h"
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class BlockQueue {
public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        cv_.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
        
        if (stopped_ && queue_.empty()) {
            return false;
        }
        
        item = queue_.front();
        queue_.pop();
        return true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
};

// Worker function for file sinks
void file_worker(BlockQueue<Block>& queue, FileSink* sink) {
    Block block;
    while (queue.pop(block)) {
        sink->consume(block);
    }
}

// Worker function for console sink - not used at the moment
void console_worker(BlockQueue<Block>& queue, ConsoleSink* sink) {
    Block block;
    while (queue.pop(block)) {
        sink->consume(block);
    }
}