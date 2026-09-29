/**
 * @file test_avector.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/AVector.hpp"
#include "core/Event.hpp"

#include <gtest/gtest.h>
#include <numeric>
#include <memory>

namespace TestAVector
{

class AVectorTest : public ::testing::Test
{
protected:
    Core::AVector<int, 10> vec;
};

TEST_F(AVectorTest, DefaultConstructor)
{
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST_F(AVectorTest, PushBack) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

TEST_F(AVectorTest, PushBackCapacityLimit) 
{
    for (int i = 0; i < 15; ++i) {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), 10);
    EXPECT_EQ(vec[9], 9);
}

TEST_F(AVectorTest, Erase) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    vec.erase(1);
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 3);
}

TEST_F(AVectorTest, EraseFirst) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    vec.erase(static_cast<int>(0));
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], 2);
    EXPECT_EQ(vec[1], 3);
}

TEST_F(AVectorTest, EraseLast) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    vec.erase(2);
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

TEST_F(AVectorTest, EraseInvalidIndex) 
{
    vec.push_back(1);
    vec.push_back(2);
    
    vec.erase(10);
    
    EXPECT_EQ(vec.size(), 2);
}

TEST_F(AVectorTest, PopBack) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    EXPECT_EQ(vec.size(), 3);
    
    vec.pop_back();
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[vec.size()-1], 2);

    vec.push_back(4);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[vec.size()-1], 4);
}


TEST_F(AVectorTest, Back) 
{
    vec.push_back(1);
    vec.push_back(4);
    vec.push_back(6);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.back(), 6);
    
    vec.pop_back();
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec.back(), 4);

    vec.push_back(8);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.back(), 8);
}

TEST_F(AVectorTest, Clear) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    vec.clear();
    
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST_F(AVectorTest, Iterator) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    int sum = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it) 
    {
        sum += *it;
    }
    
    EXPECT_EQ(sum, 6);
}

TEST_F(AVectorTest, RangeBasedFor) 
{
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    
    int sum = 0;
    for (const auto& val : vec) 
    {
        sum += val;
    }
    
    EXPECT_EQ(sum, 60);
}

TEST_F(AVectorTest, EraseIterator) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    
    auto it = vec.erase(vec.begin() + 1);
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(vec[1], 3);
}

TEST_F(AVectorTest, EraseConstIterator) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    
    const auto& const_vec = vec;
    auto const_it = const_vec.cbegin() + 1;
    
    auto it = vec.erase(const_it);
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 3);
    EXPECT_EQ(vec[2], 4);
}

TEST_F(AVectorTest, EraseRange) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);
    
    auto it = vec.erase(vec.begin() + 1, vec.begin() + 4);
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(*it, 5);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 5);
}

TEST_F(AVectorTest, EraseRangeFromBeginning) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    
    auto it = vec.erase(vec.begin(), vec.begin() + 2);
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(vec[0], 3);
    EXPECT_EQ(vec[1], 4);
}

TEST_F(AVectorTest, EraseRangeToEnd) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    
    auto it = vec.erase(vec.begin() + 2, vec.end());
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(it, vec.end());
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

TEST_F(AVectorTest, EraseEntireRange) 
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    auto it = vec.erase(vec.begin(), vec.end());
    
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(it, vec.end());
}

TEST_F(AVectorTest, EraseEmptyRange)
{
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    auto it = vec.erase(vec.begin() + 1, vec.begin() + 1);
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(*it, 2);
}

TEST(AVectorCompatibility, Accumulator) 
{
    Core::AVector<int, 5> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    int asum = std::accumulate(vec.begin(), vec.end(), 0);
    
    EXPECT_EQ(asum, 6);
}

TEST(AVectorCompatibility, Capacity) 
{
    Core::AVector<int, 7> vec;

    EXPECT_EQ(vec.capacity(), 7);
}

TEST(AVectorWeakPtrTest, WeakPtrStorage)
{
    Core::AVector<std::weak_ptr<int>, 5> weakVec;
    
    auto sp1 = std::make_shared<int>(42);
    auto sp2 = std::make_shared<int>(84);
    
    weakVec.push_back(sp1);
    weakVec.push_back(sp2);
    
    EXPECT_EQ(weakVec.size(), 2);
    EXPECT_EQ(*weakVec[0].lock(), 42);
    EXPECT_EQ(*weakVec[1].lock(), 84);
}

TEST(AVectorWeakPtrTest, ExpiredWeakPtr)
{
    Core::AVector<std::weak_ptr<int>, 5> weakVector;
    
    {
        auto shared_p = std::make_shared<int>(100);
        weakVector.push_back(shared_p);
    }
    
    EXPECT_TRUE(weakVector[0].expired());
}

TEST(AVectorMoveTest, PushBackActuallyMoves) 
{
    struct MoveTracker 
    {
        int value{0};
        bool was_moved_from = false;

        MoveTracker() = default;
        
        explicit MoveTracker(int v) : value(v) {}
        
        MoveTracker(MoveTracker&& other) noexcept 
            : value(other.value), 
              was_moved_from(false) 
        {
            other.was_moved_from = true;
        }
        
        MoveTracker& operator=(MoveTracker&& other) noexcept
        {
            value = other.value;
            was_moved_from = false;
            other.was_moved_from = true;
            return *this;
        }
        
        MoveTracker(const MoveTracker&) = delete;
        MoveTracker& operator=(const MoveTracker&) = delete;
    };
    
    Core::AVector<MoveTracker, 10> vec;
    MoveTracker item(42);
    
    vec.push_back(std::move(item));
    
    EXPECT_TRUE(item.was_moved_from);
    EXPECT_EQ(vec[0].value, 42);
    EXPECT_FALSE(vec[0].was_moved_from);
}

TEST(AVectorMoveTest, PushBackUniquePtr) 
{
    Core::AVector<std::unique_ptr<int>, 10> vec;
    
    auto ptr = std::make_unique<int>(100);
    vec.push_back(std::move(ptr));
    
    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(*vec[0], 100);
}

// tests that push_back is marked noexcept when the type T has a noexcept copy assignment operator
TEST(AVectorNoexceptTest, PushBackNoexceptForNoexceptTypes) 
{
    Core::AVector<int, 10> vec;
    // primitive types have a noexcept copy assignment !
    EXPECT_TRUE(noexcept(vec.push_back(5)));
}

TEST(AVectorNoexceptTest, PushBackNotNoexceptForThrowingTypes) 
{
    struct ThrowingCopy
    {
        ThrowingCopy() = default;
        ThrowingCopy(const ThrowingCopy&) noexcept(false) {}
        ThrowingCopy& operator=(const ThrowingCopy&) noexcept(false) { return *this; }
    };
    
    Core::AVector<ThrowingCopy, 10> vec;
    ThrowingCopy item;
    EXPECT_FALSE(noexcept(vec.push_back(item)));
}

TEST(AVectorNoexceptTest, PushBackNotNoexceptForString) {
    Core::AVector<std::string, 10> vec;
    std::string str = "test";
    // std::string copy assignment is NOT noexcept
    EXPECT_FALSE(noexcept(vec.push_back(str)));
}

TEST(AVectorNoexceptTest, PushBackMoveNoexceptForString) {
    Core::AVector<std::string, 10> vec;
    std::string str = "test";
    // std::string move assignment IS noexcept
    EXPECT_TRUE(noexcept(vec.push_back(std::move(str))));
}

TEST(AVectorNoexceptTest, PushBackMoveNoexceptForNoexceptTypes) 
{
    Core::AVector<int, 10> vec;
    EXPECT_TRUE(noexcept(vec.push_back(5)));
}

TEST(AVectorNoexceptTest, PushBackMoveNotNoexceptForThrowingTypes) 
{
    struct ThrowingMove {
        ThrowingMove() = default;
        ThrowingMove(ThrowingMove&&) noexcept(false) {}
        ThrowingMove& operator=(ThrowingMove&&) noexcept(false) { return *this; }
    };
    
    Core::AVector<ThrowingMove, 10> vec;
    ThrowingMove item;
    EXPECT_FALSE(noexcept(vec.push_back(std::move(item))));
}


TEST_F(AVectorTest, ResizeNoChange) {
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    vec.resize(3);
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

TEST_F(AVectorTest, ResizeShrink) {
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);
    
    vec.resize(2);
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

TEST_F(AVectorTest, ResizeGrow) {
    vec.push_back(1);
    vec.push_back(2);
    
    vec.resize(5);
    
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 0);  // Default constructed int
    EXPECT_EQ(vec[3], 0);
    EXPECT_EQ(vec[4], 0);
}

TEST_F(AVectorTest, ResizeToZero) {
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    vec.resize(0);
    
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST_F(AVectorTest, ResizeFromEmpty) {
    vec.resize(3);
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 0);
    EXPECT_EQ(vec[2], 0);
}

TEST_F(AVectorTest, ResizeBeyondCapacity) {
    vec.resize(15);  // Capacity is 10
    
    EXPECT_EQ(vec.size(), 10);  // Should cap at capacity
}

TEST(AVectorNoexceptTest, ResizeIsNoexcept) {
    Core::AVector<int, 10> vec;
    EXPECT_TRUE(noexcept(vec.resize(5)));
}

}// namespace TestAVector
