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

// TEST_F(UniquePtrBufferTest, should_fail_to_push_when_buffer_is_full) {
//     for (int i = 0; i < 3; ++i) {
//         EXPECT_TRUE(buffer->push(std::make_unique<int>(i)));
//     }

//     EXPECT_TRUE(buffer->is_full());
//     EXPECT_FALSE(buffer->push(std::make_unique<int>(99))); // Buffer is full, should fail
// }

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

// TEST_F(SharedPtrBufferTest, should_retain_ownership_when_popping_shared_ptr) {
//     auto ptr = std::make_shared<int>(42);
//     EXPECT_TRUE(buffer->push(ptr));

//     auto popped_item = buffer->pop();
//     EXPECT_TRUE(popped_item.has_value());
//     EXPECT_EQ(**popped_item, 42);

//     // Verify that the original shared_ptr still has ownership
//     EXPECT_EQ(ptr.use_count(), 1);
// }