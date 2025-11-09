#pragma once
#include <atomic>
#include <vector>
#include <cstddef>
#include <type_traits>

namespace utils {

template<typename T>
class LockFreeQueue {
public:
    explicit LockFreeQueue(size_t capacityPow2 = 1024)
        : cap_(roundUpPow2(capacityPow2)),
          mask_(cap_ - 1),
          buffer_(cap_),
          head_(0),
          tail_(0)
    {
        for (size_t i = 0; i < cap_; ++i) {
            buffer_[i].seq.store(i, std::memory_order_relaxed);
        }
    }

    bool push(const T& v) { return emplace(v); }
    bool push(T&& v)      { return emplace(std::move(v)); }

    bool pop(T& out) {
        size_t head = head_.load(std::memory_order_acquire);
        Cell& c = buffer_[head & mask_];

        size_t expected = head + 1;
        size_t seq = c.seq.load(std::memory_order_acquire);

        if (seq != expected) {
            return false;
        }

        out = std::move(c.value);
        c.seq.store(head + cap_, std::memory_order_release);
        head_.store(head + 1, std::memory_order_relaxed);
        return true;
    }

private:
    struct Cell {
        std::atomic<size_t> seq;
        T value;
    };

    template<typename U>
    bool emplace(U&& v) {
        size_t pos = tail_.fetch_add(1, std::memory_order_acq_rel);
        Cell& c = buffer_[pos & mask_];

        size_t expected = pos;
        size_t seq = c.seq.load(std::memory_order_acquire);

        if (seq != expected) {
            return false;
        }

        c.value = std::forward<U>(v);

        c.seq.store(pos + 1, std::memory_order_release);
        return true;
    }

    static size_t roundUpPow2(size_t x) {
        if (x < 2) return 2;
        --x;
        x |= x>>1;
        x |= x>>2;
        x |= x>>4;
        x |= x>>8;
        x |= x>>16;
        x |= x>>32;
        return x+1;
    }

    const size_t cap_;
    const size_t mask_;
    std::vector<Cell> buffer_;

    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

}
