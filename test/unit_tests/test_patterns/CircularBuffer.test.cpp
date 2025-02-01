#include <gtest/gtest.h>
#include <gmock/gmock.h>
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

// =============================================================

// Fixture for CircularBuffer tests
class CircularBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize a CircularBuffer with a maximum size of 5
        buffer = std::make_unique<CircularBuffer<std::string, 5>>();
    }

    void TearDown() override {
        // Clean up (if needed)
        buffer.reset();
    }

    std::unique_ptr<CircularBuffer<std::string, 5>> buffer;
};

// Test case: Buffer should contain last 5 items when 10 items are pushed given buffer size is 5
TEST_F(CircularBufferTest, should_contain_last_5_items_when_10_items_are_pushed_given_buffer_size_is_5) {
    // Given: A buffer with a maximum size of 5
    ASSERT_EQ(buffer->size(), 0);

    // When: 10 items are pushed into the buffer
    for (int i = 1; i <= 10; ++i) {
        buffer->push("Item " + std::to_string(i));
    }

    // Then: The buffer should contain the last 5 items
    EXPECT_EQ(buffer->size(), 5);
    EXPECT_EQ(buffer->at(0).value(), "Item 6");
    EXPECT_EQ(buffer->at(1).value(), "Item 7");
    EXPECT_EQ(buffer->at(2).value(), "Item 8");
    EXPECT_EQ(buffer->at(3).value(), "Item 9");
    EXPECT_EQ(buffer->at(4).value(), "Item 10");
}

// Test case: Buffer should return items in FIFO order when items are popped given items were pushed
TEST_F(CircularBufferTest, should_return_items_in_fifo_order_when_items_are_popped_given_items_were_pushed) {
    // Given: A buffer with 3 items pushed
    buffer->push("Item 1");
    buffer->push("Item 2");
    buffer->push("Item 3");
    ASSERT_EQ(buffer->size(), 3);

    // When: Items are popped from the buffer
    std::string item1 = buffer->pop().value();
    std::string item2 = buffer->pop().value();
    std::string item3 = buffer->pop().value();

    // Then: The items should be returned in FIFO order
    EXPECT_EQ(item1, "Item 1");
    EXPECT_EQ(item2, "Item 2");
    EXPECT_EQ(item3, "Item 3");
    EXPECT_TRUE(buffer->is_empty());
}

// Test case: Buffer should overwrite oldest items when more than 5 items are pushed given buffer size is 5
TEST_F(CircularBufferTest, should_overwrite_oldest_items_when_more_than_5_items_are_pushed_given_buffer_size_is_5) {
    // Given: A buffer with a maximum size of 5
    ASSERT_EQ(buffer->size(), 0);

    // When: 7 items are pushed into the buffer
    for (int i = 1; i <= 7; ++i) {
        buffer->push("Item " + std::to_string(i));
    }

    // Then: The buffer should contain the last 5 items
    EXPECT_EQ(buffer->size(), 5);
    EXPECT_EQ(buffer->at(0).value(), "Item 3");
    EXPECT_EQ(buffer->at(1).value(), "Item 4");
    EXPECT_EQ(buffer->at(2).value(), "Item 5");
    EXPECT_EQ(buffer->at(3).value(), "Item 6");
    EXPECT_EQ(buffer->at(4).value(), "Item 7");
}

// Test case: Buffer should return nullopt when popped given buffer is empty
TEST_F(CircularBufferTest, should_return_nullopt_when_popped_given_buffer_is_empty) {
    // Given: An empty buffer
    ASSERT_TRUE(buffer->is_empty());

    // When: An item is popped from the buffer
    auto item = buffer->pop();

    // Then: The result should be nullopt
    EXPECT_FALSE(item.has_value());
}

// Test case: Buffer should be full when 5 items are pushed given buffer size is 5
TEST_F(CircularBufferTest, should_be_full_when_5_items_are_pushed_given_buffer_size_is_5) {
    // Given: A buffer with a maximum size of 5
    ASSERT_EQ(buffer->size(), 0);

    // When: 5 items are pushed into the buffer
    for (int i = 1; i <= 5; ++i) {
        buffer->push("Item " + std::to_string(i));
    }

    // Then: The buffer should be full
    EXPECT_TRUE(buffer->is_max());

    // When: One more item is pushed
    buffer->push("Item 6");

    // Then: The buffer should still be full, and the oldest item should be overwritten
    EXPECT_TRUE(buffer->is_max());
    EXPECT_EQ(buffer->at(0).value(), "Item 2");
    EXPECT_EQ(buffer->at(1).value(), "Item 3");
    EXPECT_EQ(buffer->at(2).value(), "Item 4");
    EXPECT_EQ(buffer->at(3).value(), "Item 5");
    EXPECT_EQ(buffer->at(4).value(), "Item 6");
}