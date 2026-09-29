/**
 * @file test_amap.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>
#include "core/AMap.hpp"
// #include <unordered_map>

namespace TestAMap
{

// alias for tests
using Map = Core::AMap<std::string, int, 100>;
// using Map = std::unordered_map<std::string, int>;

// ================= basics =======================

TEST(AMap, InsertAndFind)
{
    Map m;

    auto [it, ok] = m.insert({"key", 42});
    EXPECT_TRUE(ok);
    EXPECT_NE(it, m.end());
    EXPECT_EQ(it->first, "key");
    EXPECT_EQ(it->second, 42);

    auto fit = m.find("key");
    EXPECT_NE(fit, m.end());
    EXPECT_EQ(fit->second, 42);
}

TEST(AMap, InsertDuplicate)
{
    Map m;

    m.insert({"key", 1});
    auto [it, ok] = m.insert({"key", 2});

    EXPECT_FALSE(ok);
    EXPECT_EQ(it->second, 1);
}

TEST(AMap, Emplace)
{
    Map m;

    auto [it, ok] = m.emplace("key", 123);
    EXPECT_TRUE(ok);
    EXPECT_EQ(it->second, 123);
}

TEST(AMap, FindConst)
{
    Map m;
    m.insert({"a", 10});

    const Map& cm = m;

    auto it = cm.find("a");
    EXPECT_NE(it, cm.end());
    EXPECT_EQ(it->second, 10);
}

TEST(AMap, HeterogeneousFind)
{
    Map m;
    m.insert({"abc", 7});

    const char* key = "abc";
    auto it = m.find(key);

    EXPECT_NE(it, m.end());
    EXPECT_EQ(it->second, 7);
}

TEST(AMap, OperatorAccessInsert)
{
    Map m;

    m["x"] = 5;
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(m.find("x")->second, 5);
}

TEST(AMap, OperatorAccessExisting)
{
    Map m;
    m.insert({"x", 1});

    m["x"] = 9;
    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(m.find("x")->second, 9);
}

TEST(AMap, EraseByKey)
{
    Map m;
    m.insert({"a", 1});
    m.insert({"b", 2});

    bool removed = m.erase("a");

    EXPECT_TRUE(removed);
    EXPECT_EQ(m.find("a"), m.end());
    EXPECT_EQ(m.size(), 1);
}

TEST(AMap, EraseByIterator)
{
    Map m;
    m.insert({"a", 1});
    m.insert({"b", 2});

    auto it = m.find("a");
    auto next = m.erase(it);

    EXPECT_EQ(m.size(), 1);
    EXPECT_EQ(m.find("a"), m.end());
    EXPECT_TRUE(next == m.begin() || next == m.end());
}

TEST(AMap, Contains)
{
    Map m;
    m.insert({"k", 3});

    EXPECT_TRUE(m.contains("k"));
    EXPECT_FALSE(m.contains("x"));
}

TEST(AMap, CapacityLimit)
{
    Core::AMap<int, int, 2> m;
    // std::unordered_map<int, int> m;

    EXPECT_TRUE(m.insert({1,1}).second);
    EXPECT_TRUE(m.insert({2,2}).second);

    auto [it, ok] = m.insert({3,3});
    EXPECT_FALSE(ok);
    EXPECT_EQ(it, m.end());
}

TEST(AMap, IteratorRangeLoop)
{
    Map m;
    m.insert({"a", 1});
    m.insert({"b", 2});

    int sum = 0;
    for (const auto& [k, v] : m)
        sum += v;

    EXPECT_EQ(sum, 3);
}

TEST(AMap, EraseInvalidIterator)
{
    Map m;
    m.insert({"a", 1});

    auto it = m.find("a");
    ASSERT_NE(it, m.end());

    auto res = m.erase(it);
    EXPECT_EQ(res, m.end());
}

// ================= operator[] =================

TEST(AMap_OperatorBracket, InsertNewKey)
{
    Map m;
    m["hello"] = 10;
    EXPECT_EQ(m["hello"], 10);
    EXPECT_EQ(m.size(), 1);
}

TEST(AMap_OperatorBracket, UpdateExistingKey)
{
    Map m;
    m["hello"] = 10;
    m["hello"] = 20;
    EXPECT_EQ(m["hello"], 20);
    EXPECT_EQ(m.size(), 1);
}

TEST(AMap_OperatorBracket, DefaultValueOnNewKey)
{
    Map m;
    EXPECT_EQ(m["hello"], 0);
    EXPECT_EQ(m.size(), 1);
}

#ifndef NDEBUG 
// EXPECT_DEATH requires the binary to be built with assert enabled — won't fire in release !
TEST(AMap_OperatorBracket, FullMapAssertFires)
{
    Core::AMap<std::string, int, 2> m;
    m["a"] = 1;
    m["b"] = 2;
    EXPECT_DEATH(m["c"] = 3, ".*");
}
#endif

TEST(AMap_OperatorBracket, ConstAccessExistingKey)
{
    Map m;
    m["hello"] = 10;
    const auto& cm = m;
    EXPECT_EQ(cm["hello"], 10);
}

TEST(AMap_OperatorBracket, ConstAccessMissingKeyReturnsDummy)
{
    Map m;
    const auto& cm = m;
    EXPECT_EQ(cm["missing"], 0);
    EXPECT_EQ(cm.size(), 0);
}

TEST(AMap_OperatorBracket, At_Key)
{
    Map m;
    m["hello"] = 10;
    auto& cm = m;
    EXPECT_EQ(cm.at("hello"), 10);
}

TEST(AMap_OperatorBracket, Const_At_Key)
{
    Map m;
    m["hello_const"] = 10;
    const auto& cm = m;
    EXPECT_EQ(cm.at("hello_const"), 10);
}

TEST(AMap_OperatorBracket, Const_At_MissingKeyReturnsDummy)
{
    Map m;
    const auto& cm = m;
    EXPECT_EQ(cm.at("missing"), 0);
    EXPECT_EQ(cm.size(), 0);
}

// ================= insert =================

TEST(AMap_Insert, InsertNewEntry)
{
    Map m;
    auto [it, inserted] = m.insert({"hello", 10});
    EXPECT_TRUE(inserted);
    EXPECT_EQ(it->first, "hello");
    EXPECT_EQ(it->second, 10);
}

TEST(AMap_Insert, InsertDuplicateKey)
{
    Map m;
    m.insert({"hello", 10});
    auto [it, inserted] = m.insert({"hello", 99});
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it->second, 10);
}

TEST(AMap_Insert, InsertOnFullMap)
{
    Core::AMap<std::string, int, 2> m;
    m.insert({"a", 1});
    m.insert({"b", 2});
    auto [it, inserted] = m.insert({"c", 3});
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it, m.end());
}

TEST(AMap_Insert, InsertExistingKeyOnFullMap)
{
    Core::AMap<std::string, int, 2> m;
    m.insert({"a", 1});
    m.insert({"b", 2});
    auto [it, inserted] = m.insert({"a", 99});
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it->second, 1);
}

TEST(AMap_Insert, InsertFillsToCapacity)
{
    Core::AMap<std::string, int, 3> m;
    EXPECT_TRUE(m.insert({"a", 1}).second);
    EXPECT_TRUE(m.insert({"b", 2}).second);
    EXPECT_TRUE(m.insert({"c", 3}).second);
    EXPECT_TRUE(m.full());
}

// ================= emplace =================

TEST(AMap_Emplace, EmplaceNewEntry)
{
    Map m;
    auto [it, inserted] = m.emplace("hello", 10);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(it->first, "hello");
    EXPECT_EQ(it->second, 10);
}

TEST(AMap_Emplace, EmplaceDuplicateKey)
{
    Map m;
    m.emplace("hello", 10);
    auto [it, inserted] = m.emplace("hello", 99);
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it->second, 10);
}

TEST(AMap_Emplace, EmplaceOnFullMap)
{
    Core::AMap<std::string, int, 2> m;
    m.emplace("a", 1);
    m.emplace("b", 2);
    auto [it, inserted] = m.emplace("c", 3);
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it, m.end());
}

TEST(AMap_Emplace, EmplaceReturnedIteratorIsValid)
{
    Map m;
    auto [it, inserted] = m.emplace("hello", 42);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(it, m.begin());
    EXPECT_EQ(it->second, 42);
}

TEST(AMap_Emplace, EmplaceSizeIncrementsOnlyOnInsertion)
{
    Map m;
    m.emplace("hello", 10);
    m.emplace("hello", 20);
    EXPECT_EQ(m.size(), 1);
}

// ================= erase =================

TEST(AMap_Erase, EraseByKeyExists)
{
    Map m;
    m.insert({"hello", 10});
    EXPECT_TRUE(m.erase("hello"));
    EXPECT_FALSE(m.contains("hello"));
    EXPECT_EQ(m.size(), 0);
}

TEST(AMap_Erase, EraseByKeyMissing)
{
    Map m;
    m.insert({"hello", 10});
    EXPECT_FALSE(m.erase("missing"));
    EXPECT_EQ(m.size(), 1);
}

TEST(AMap_Erase, EraseByKeySwapsWithLast)
{
    Map m;
    m.insert({"a", 1});
    m.insert({"b", 2});
    m.insert({"c", 3});
    m.erase("a");
    EXPECT_FALSE(m.contains("a"));
    EXPECT_TRUE(m.contains("b"));
    EXPECT_TRUE(m.contains("c"));
    EXPECT_EQ(m.size(), 2);
}

TEST(AMap_Erase, EraseLastElement)
{
    Map m;
    m.insert({"a", 1});
    m.insert({"b", 2});
    m.erase("b");
    EXPECT_FALSE(m.contains("b"));
    EXPECT_TRUE(m.contains("a"));
    EXPECT_EQ(m.size(), 1);
}

TEST(AMap_Erase, EraseByIterator)
{
    Map m;
    m.insert({"a", 1});
    m.insert({"b", 2});
    auto it = m.find("a");
    ASSERT_NE(it, m.end());
    m.erase(it);
    EXPECT_FALSE(m.contains("a"));
    EXPECT_EQ(m.size(), 1);
}

TEST(AMap_Erase, EraseByIteratorReturnsNextValid)
{
    Map m;
    m.insert({"a", 1});
    m.insert({"b", 2});
    m.insert({"c", 3});
    auto it = m.find("a");
    auto next = m.erase(it);
    EXPECT_NE(next, m.end());
}

TEST(AMap_Erase, EraseByIteratorOnLastReturnsEnd)
{
    Map m;
    m.insert({"a", 1});
    auto it = m.begin();
    auto result = m.erase(it);
    EXPECT_EQ(result, m.end());
    EXPECT_EQ(m.size(), 0);
}

TEST(AMap_Erase, EraseByConstIterator)
{
    Map m;
    m.insert({"a", 1});
    m.insert({"b", 2});
    auto it = m.cbegin();
    m.erase(it);
    EXPECT_EQ(m.size(), 1);
}

TEST(AMap_Erase, EraseFromEmptyMap)
{
    Map m;
    EXPECT_FALSE(m.erase("missing"));
}

}// namespace TestAMap