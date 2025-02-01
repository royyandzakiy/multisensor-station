#include <gtest/gtest.h>
#include "CircularBuffer.hpp" // Include the generic CircularBuffer header

// Fixture for testing copyable types
template <typename T, std::size_t BufferSize>
class CopyableCircularBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer = std::make_unique<CircularBuffer<T, BufferSize>>();
    }

    void TearDown() override {
        buffer.reset();
    }

    std::unique_ptr<CircularBuffer<T, BufferSize>> buffer;
};

// Test with int
using IntBufferTest = CopyableCircularBufferTest<int, 5>;

TEST_F(IntBufferTest, should_push_and_pop_int_items) {
    EXPECT_TRUE(buffer->push(42));
    EXPECT_TRUE(buffer->push(100));

    auto popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(popped_item.value(), 42);

    popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(popped_item.value(), 100);

    EXPECT_TRUE(buffer->is_empty());
}

// TEST_F(IntBufferTest, should_fail_to_push_when_buffer_is_full) {
//     for (int i = 0; i < 5; ++i) {
//         EXPECT_TRUE(buffer->push(i));
//     }

//     EXPECT_TRUE(buffer->is_full());
//     EXPECT_FALSE(buffer->push(99)); // Buffer is full, should fail
// }

// Test with std::string
using StringBufferTest = CopyableCircularBufferTest<std::string, 3>;

TEST_F(StringBufferTest, should_push_and_pop_string_items) {
    EXPECT_TRUE(buffer->push("hello"));
    EXPECT_TRUE(buffer->push("world"));

    auto popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(popped_item.value(), "hello");

    popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(popped_item.value(), "world");

    EXPECT_TRUE(buffer->is_empty());
}

TEST_F(StringBufferTest, should_return_nullopt_when_popping_from_empty_buffer) {
    EXPECT_TRUE(buffer->is_empty());
    auto popped_item = buffer->pop();
    EXPECT_FALSE(popped_item.has_value());
}

// =============================================================

// Fixture for testing movable types
template <typename T, std::size_t BufferSize>
class MovableCircularBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer = std::make_unique<CircularBuffer<T, BufferSize>>();
    }

    void TearDown() override {
        buffer.reset();
    }

    std::unique_ptr<CircularBuffer<T, BufferSize>> buffer;
};

// Test with std::unique_ptr
using UniquePtrBufferTest = MovableCircularBufferTest<std::unique_ptr<int>, 3>;

TEST_F(UniquePtrBufferTest, should_push_and_pop_unique_ptr_items) {
    auto ptr1 = std::make_unique<int>(42);
    auto ptr2 = std::make_unique<int>(100);

    EXPECT_TRUE(buffer->push(std::move(ptr1)));
    EXPECT_TRUE(buffer->push(std::move(ptr2)));

    auto popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(**popped_item, 42);

    popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(**popped_item, 100);

    EXPECT_TRUE(buffer->is_empty());
}

TEST_F(UniquePtrBufferTest, should_fail_to_push_when_buffer_is_full) {
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(buffer->push(std::make_unique<int>(i)));
    }

    EXPECT_TRUE(buffer->is_full());
    EXPECT_FALSE(buffer->push(std::make_unique<int>(99))); // Buffer is full, should fail
}

// Test with std::shared_ptr
using SharedPtrBufferTest = MovableCircularBufferTest<std::shared_ptr<int>, 3>;

TEST_F(SharedPtrBufferTest, should_push_and_pop_shared_ptr_items) {
    auto ptr1 = std::make_shared<int>(42);
    auto ptr2 = std::make_shared<int>(100);

    EXPECT_TRUE(buffer->push(ptr1));
    EXPECT_TRUE(buffer->push(ptr2));

    auto popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(**popped_item, 42);

    popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(**popped_item, 100);

    EXPECT_TRUE(buffer->is_empty());
}

TEST_F(SharedPtrBufferTest, should_retain_ownership_when_popping_shared_ptr) {
    auto ptr = std::make_shared<int>(42);
    EXPECT_TRUE(buffer->push(ptr));

    // Verify that the reference count increased after pushing
    EXPECT_EQ(ptr.use_count(), 2); // ptr + buffer's copy

    auto popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_EQ(**popped_item, 42);

    // Verify that the reference count is now 2 (ptr + popped_item)
    EXPECT_EQ(ptr.use_count(), 2);

    // Reset popped_item to release its reference
    popped_item.reset();

    // Verify that the reference count is now 1 (only ptr remains)
    EXPECT_EQ(ptr.use_count(), 1);
}

// =============================================================

#include <gtest/gtest.h>
#include "CircularBuffer.hpp" // Include the generic CircularBuffer header
#include <array> // For std::array
#include <cstring> // For std::strncpy

// Fixture for testing C-style arrays
template <typename T, std::size_t BufferSize>
class CharArrayCircularBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer = std::make_unique<CircularBuffer<T, BufferSize>>();
    }

    void TearDown() override {
        buffer.reset();
    }

    std::unique_ptr<CircularBuffer<T, BufferSize>> buffer;
};

// Test with std::array<char, 64> (simulates char buffer[64])
using CharArrayBufferTest = CharArrayCircularBufferTest<std::array<char, 64>, 3>;

TEST_F(CharArrayBufferTest, should_push_and_pop_char_array_items) {
    std::array<char, 64> item1 = {};
    std::array<char, 64> item2 = {};

    // Copy data into the arrays
    std::strncpy(item1.data(), "hello", 64);
    std::strncpy(item2.data(), "world", 64);

    EXPECT_TRUE(buffer->push(item1));
    EXPECT_TRUE(buffer->push(item2));

    auto popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_STREQ(popped_item.value().data(), "hello");

    popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_STREQ(popped_item.value().data(), "world");

    EXPECT_TRUE(buffer->is_empty());
}

TEST_F(CharArrayBufferTest, should_fail_to_push_when_buffer_is_full) {
    std::array<char, 64> item1 = {};
    std::array<char, 64> item2 = {};
    std::array<char, 64> item3 = {};
    std::array<char, 64> item4 = {};

    // Copy data into the arrays
    std::strncpy(item1.data(), "item1", 64);
    std::strncpy(item2.data(), "item2", 64);
    std::strncpy(item3.data(), "item3", 64);
    std::strncpy(item4.data(), "item4", 64);

    EXPECT_TRUE(buffer->push(item1));
    EXPECT_TRUE(buffer->push(item2));
    EXPECT_TRUE(buffer->push(item3)); // Buffer is now full
    EXPECT_FALSE(buffer->push(item4)); // Should fail to push
    EXPECT_TRUE(buffer->is_full());
}

TEST_F(CharArrayBufferTest, should_return_nullopt_when_popping_from_empty_buffer) {
    EXPECT_TRUE(buffer->is_empty());
    auto popped_item = buffer->pop();
    EXPECT_FALSE(popped_item.has_value());
}

TEST_F(CharArrayBufferTest, should_handle_large_char_arrays) {
    std::array<char, 64> large_item = {};
    const char* long_string = "This is a very long string that should fit in the char array.";

    // Copy data into the array
    std::strncpy(large_item.data(), long_string, 64);

    EXPECT_TRUE(buffer->push(large_item));

    auto popped_item = buffer->pop();
    EXPECT_TRUE(popped_item.has_value());
    EXPECT_STREQ(popped_item.value().data(), long_string);
}