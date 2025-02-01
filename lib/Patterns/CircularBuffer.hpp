#pragma once
#include <array>
#include <optional>
#include <cstddef>
#include <utility> // For std::move
#include <esp_log.h>

static int count___ = 0;

template <typename T, std::size_t BufferSize>
class CircularBuffer {
public:
    CircularBuffer() : head_(0), tail_(0) {}

    // Push an item into the buffer (supports both copyable and movable types)
    bool push(const T& item) {
        std::size_t next_head = (head_ + 1) % (BufferSize + 1);

        if (next_head == tail_) {
            return false; // Buffer is full
        }

        ESP_LOGI("CircularBuffer", "total count: %d", ++count___);

        buffer_[head_] = item;
        head_ = next_head;
        return true;
    }

    bool push(T&& item) {
        std::size_t next_head = (head_ + 1) % (BufferSize + 1);
        
        if (next_head == tail_) {
            return false; // Buffer is full
        }
        ESP_LOGI("CircularBuffer", "total count: %d", ++count___);
        
        buffer_[head_] = std::move(item);
        head_ = next_head;
        return true;
    }

    // Pop an item from the buffer
    std::optional<T> pop() {
        if (tail_ == head_) {
            return std::nullopt; // Buffer is empty
        }
        
        ESP_LOGI("CircularBuffer", "total count: %d", --count___);
        std::optional<T> item = std::move(buffer_[tail_]);
        tail_ = (tail_ + 1) % BufferSize;
        return item;
    }

    // Check if the buffer is empty
    bool is_empty() const {
        return head_ == tail_;
    }

    // Check if the buffer is full
    bool is_full() const {
        return (head_ + 1) % (BufferSize + 1) == tail_;
    }

private:
    std::array<std::optional<T>, BufferSize> buffer_; // Buffer to hold items of type T
    std::size_t head_; // Index of the next write position
    std::size_t tail_; // Index of the next read position
};
