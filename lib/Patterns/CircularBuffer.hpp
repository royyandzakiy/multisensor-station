// #include <array>
// #include <atomic>
// #include <optional>
// #include <cstddef>

// template <typename T, std::size_t BufferSize>
// class CircularBuffer {
// public:
//     CircularBuffer() : head_(0), tail_(0) {}

//     // Push an item into the buffer
//     bool push(const T& item) {
//         std::size_t head = head_.load(std::memory_order_relaxed);
//         std::size_t next_head = (head + 1) % BufferSize;

//         if (next_head == tail_.load(std::memory_order_acquire)) {
//             return false; // Buffer is full
//         }

//         buffer_[head] = item; // Copy the item into the buffer
//         head_.store(next_head, std::memory_order_release);
//         return true;
//     }

//     // Pop an item from the buffer
//     std::optional<T> pop() {
//         std::size_t tail = tail_.load(std::memory_order_relaxed);

//         if (tail == head_.load(std::memory_order_acquire)) {
//             return std::nullopt; // Buffer is empty
//         }

//         T item = buffer_[tail]; // Retrieve the item
//         tail_.store((tail + 1) % BufferSize, std::memory_order_release);
//         return item;
//     }

//     // Check if the buffer is empty
//     bool is_empty() const {
//         return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
//     }

//     // Check if the buffer is full
//     bool is_full() const {
//         std::size_t next_head = (head_.load(std::memory_order_relaxed) + 1) % BufferSize;
//         return next_head == tail_.load(std::memory_order_acquire);
//     }

// private:
//     std::array<T, BufferSize> buffer_; // Buffer to hold items of type T
//     std::atomic<std::size_t> head_;    // Index of the next write position
//     std::atomic<std::size_t> tail_;    // Index of the next read position
// };

#include <array>
#include <atomic>
#include <optional>
#include <cstddef>
#include <memory> // For std::unique_ptr, std::shared_ptr
#include <utility> // For std::forward, std::move

template <typename T, std::size_t BufferSize>
class CircularBuffer {
public:
    CircularBuffer() : head_(0), tail_(0) {}

    // Push an item into the buffer (supports both copyable and movable types)
    template <typename U>
    bool push(U&& item) {
        std::size_t head = head_.load(std::memory_order_relaxed);
        std::size_t next_head = (head + 1) % BufferSize;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false; // Buffer is full
        }

        // Use perfect forwarding to handle both lvalues and rvalues
        buffer_[head] = std::forward<U>(item);
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    // Pop an item from the buffer
    std::optional<T> pop() {
        std::size_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return std::nullopt; // Buffer is empty
        }

        // Move the item out of the buffer
        std::optional<T> item = std::move(buffer_[tail]);
        tail_.store((tail + 1) % BufferSize, std::memory_order_release);
        return item;
    }

    // Check if the buffer is empty
    bool is_empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    // Check if the buffer is full
    bool is_full() const {
        std::size_t next_head = (head_.load(std::memory_order_relaxed) + 1) % BufferSize;
        return next_head == tail_.load(std::memory_order_acquire);
    }

private:
    std::array<std::optional<T>, BufferSize> buffer_; // Buffer to hold items of type T
    std::atomic<std::size_t> head_; // Index of the next write position
    std::atomic<std::size_t> tail_; // Index of the next read position
};