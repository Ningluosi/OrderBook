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
        : cap_(roundUpPow2(capacityPow2)), mask_(cap_ - 1),
          buffer_(cap_), head_(0), tail_(0) {}

    bool push(const T& v) { return emplace(v); }
    bool push(T&& v)      { return emplace(std::move(v)); }

    bool pop(T& out) {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tailCache_.load(std::memory_order_relaxed);
        if (head == tail) {
            tail = tail_.load(std::memory_order_acquire);
            tailCache_.store(tail, std::memory_order_relaxed);
            if (head == tail) return false;
        }
        out = std::move(buffer_[head & mask_]);
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

private:
    template<typename U>
    bool emplace(U&& v) {
        size_t tail = tail_.fetch_add(1, std::memory_order_acq_rel);
        size_t head = headCache_.load(std::memory_order_relaxed);
        if (tail - head >= cap_) {
            tail_.fetch_sub(1, std::memory_order_acq_rel);
            head = head_.load(std::memory_order_acquire);
            headCache_.store(head, std::memory_order_relaxed);

            if (tail - head >= cap_) return false;

            size_t t2 = tail_.fetch_add(1, std::memory_order_acq_rel);
            if (t2 - head >= cap_) {
                tail_.fetch_sub(1, std::memory_order_acq_rel);
                return false;
            }
            buffer_[t2 & mask_] = std::forward<U>(v);
            return true;
        }
        buffer_[tail & mask_] = std::forward<U>(v);
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
    std::vector<T> buffer_;

    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;

    std::atomic<size_t> headCache_{0};
    std::atomic<size_t> tailCache_{0};
};

}
