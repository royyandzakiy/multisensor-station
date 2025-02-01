#pragma once
#include <array>
#include <optional>
#include <cstddef>
#include <utility> // For std::move
#ifdef PLATFORM_ESP32
#include <esp_log.h>
#endif

template <typename T, std::size_t BufferSize>
class CircularBuffer {
public:
    CircularBuffer() : head_(0), tail_(0), size_(0) {}

    // Push an item into the buffer (supports both copyable and movable types)
    bool push(const T& item) {
        std::size_t next_head = (head_ + 1) % (BufferSize + 1);

        if (is_max()) {
            // Buffer is full, move tail forward to overwrite the oldest item
            tail_ = (tail_ + 1) % (BufferSize + 1);
            --size_; // Decrease size since we are overwriting an item
        }

        buffer_[head_] = item;
        head_ = next_head;
        ++size_;
#ifdef PLATFORM_ESP32
        ESP_LOGI("CircularBuffer", "total count: %d", size_);
#endif
        return true;
    }

    bool push(T&& item) {
        std::size_t next_head = (head_ + 1) % (BufferSize + 1);
        
        if (is_max()) {
            // Buffer is full, move tail forward to overwrite the oldest item
            tail_ = (tail_ + 1) % (BufferSize + 1);
            --size_; // Decrease size since we are overwriting an item
        }
        
        buffer_[head_] = std::move(item);
        head_ = next_head;
        ++size_;
#ifdef PLATFORM_ESP32
        ESP_LOGI("CircularBuffer", "total count: %d", size_);
#endif
        return true;
    }

    // Pop an item from the buffer
    std::optional<T> pop() {
        if (is_empty()) {
            return std::nullopt; // Buffer is empty
        }
        
        std::optional<T> item = std::move(buffer_[tail_]);
        tail_ = (tail_ + 1) % (BufferSize + 1);
        --size_;
#ifdef PLATFORM_ESP32
        ESP_LOGI("CircularBuffer", "total count: %d", size_);
#endif
        return item;
    }

    // Check if the buffer is empty
    bool is_empty() const {
        return head_ == tail_;
    }

    // Check if the buffer is full
    bool is_max() const {
        return (head_ + 1) % (BufferSize + 1) == tail_;
    }

    size_t size() const {
        return size_;
    }

    // Access the buffer without modifying it (for debugging purposes)
    const std::optional<T>& at(std::size_t index) const {
        std::size_t actual_index = (tail_ + index) % (BufferSize + 1);
        return buffer_[actual_index];
    }

private:
    std::array<std::optional<T>, BufferSize + 1> buffer_; // Buffer to hold items of type T
    std::size_t head_; // Index of the next write position
    std::size_t tail_; // Index of the next read position
    std::size_t size_; // Current number of items in the buffer
};