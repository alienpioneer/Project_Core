/**
 * @file test_config_loader.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>
#include <fstream>

#include "core/ConfigLoader.hpp"

namespace TestConfigLoader
{

using namespace Core;

static ConfigLoader::ConfigResult loadFromString(const std::string& content)
{
    ConfigLoader::ConfigResult res;
    ConfigLoader::parse(content, res);
    return res;
}

// ===================== PARSING =====================

TEST(ConfigLoader, BasicParsing)
{
    auto res = loadFromString(
        "[NET]\n"
        "port = 8080\n"
        "host = localhost\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int port{};
    EXPECT_TRUE(res.config->getInt("NET", "port", port));
    EXPECT_EQ(port, 8080);

    EXPECT_EQ(res.config->get("NET", "host"), "localhost");
}

TEST(ConfigLoader, ParseEmptyLines)
{
    auto res = loadFromString(
        "\n"
        "   \n"
        "[A]\n"
        "\n"
        "k=1\n"
        "\n"
    );

    EXPECT_EQ(res.error, ConfigError::Ok);

    int v{};
    EXPECT_TRUE(res.config->getInt("A", "k", v));
    EXPECT_EQ(v, 1);
}

TEST(ConfigLoader, ParseEmptyKeyExplicit)
{
    ConfigLoader::ConfigResult res;
    ConfigLoader::parse(
        "[A]\n"
        "   = value\n",
        res
    );

    EXPECT_EQ(res.error, ConfigError::MalformedLine);
    EXPECT_EQ(res.errorLine, 2);
}

// Only feasible if container capacity is bounded (AVector / custom map) !
TEST(ConfigLoader, ParseCapacityError)
{
    ConfigLoader::ConfigResult res;

    std::string content;
    for (int i = 0; i < 1024; ++i)
    {
        content += "[S" + std::to_string(i) + "]\n";
    }

    ConfigLoader::parse(content, res);

    // Accept either Ok (if unbounded) or CapacityError (bounded container)
    EXPECT_TRUE(res.error == ConfigError::Ok ||
                res.error == ConfigError::CapacityError);
}

TEST(ConfigLoader, ParseValueBackslashEscape)
{
    auto res = loadFromString(
        "[E]\n"
        "v = \"a\\\\b\"\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);
    EXPECT_EQ(res.config->get("E", "v"), "a\\b");
}

TEST(ConfigLoader, QuotedValues)
{
    auto res = loadFromString(
        "[MSG]\n"
        "val = \"0x1A2, P, S\"\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);
    EXPECT_EQ(res.config->get("MSG", "val"), "0x1A2, P, S");
}

TEST(ConfigLoader, InlineComments)
{
    auto res = loadFromString(
        "[A]\n"
        "key = 42 ; comment\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int v{};
    EXPECT_TRUE(res.config->getInt("A", "key", v));
    EXPECT_EQ(v, 42);
}

TEST(ConfigLoader, CommentsInsideQuotes)
{
    auto res = loadFromString(
        "[A]\n"
        "key = \"value ; not comment\"\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);
    EXPECT_EQ(res.config->get("A", "key"), "value ; not comment");
}

TEST(ConfigLoader, EmptyValues)
{
    auto res = loadFromString(
        "[A]\n"
        "k1 =\n"
        "k2 = \"\"\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);
    EXPECT_EQ(res.config->get("A", "k1"), "");
    EXPECT_EQ(res.config->get("A", "k2"), "");
}

TEST(ConfigLoader, Escapes)
{
    auto res = loadFromString(
        "[E]\n"
        "n = \"line1\\nline2\"\n"
        "t = \"a\\tb\"\n"
        "q = \"a\\\"b\"\n"
        "x = \"a\\x\"\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    EXPECT_EQ(res.config->get("E", "n"), "line1\nline2");
    EXPECT_EQ(res.config->get("E", "t"), "a\tb");
    EXPECT_EQ(res.config->get("E", "q"), "a\"b");
    EXPECT_EQ(res.config->get("E", "x"), "ax");
}

TEST(ConfigLoader, NumericParsing)
{
    auto res = loadFromString(
        "[N]\n"
        "dec = 42\n"
        "hex = 0x1A\n"
        "bin = 0b1010\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int d{}, h{}, b{};
    EXPECT_TRUE(res.config->getInt("N", "dec", d));
    EXPECT_TRUE(res.config->getInt("N", "hex", h));
    EXPECT_TRUE(res.config->getInt("N", "bin", b));

    EXPECT_EQ(d, 42);
    EXPECT_EQ(h, 26);
    EXPECT_EQ(b, 10);
}

TEST(ConfigLoader, BoolParsing)
{
    auto res = loadFromString(
        "[B]\n"
        "t1 = true\n"
        "t2 = 1\n"
        "f1 = false\n"
        "f2 = 0\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    bool v{};
    EXPECT_TRUE(res.config->getBool("B", "t1", v)); EXPECT_TRUE(v);
    EXPECT_TRUE(res.config->getBool("B", "t2", v)); EXPECT_TRUE(v);
    EXPECT_TRUE(res.config->getBool("B", "f1", v)); EXPECT_FALSE(v);
    EXPECT_TRUE(res.config->getBool("B", "f2", v)); EXPECT_FALSE(v);
}

// ===================== GETTERS =====================

TEST(ConfigLoader, GetDouble)
{
    auto res = loadFromString(
        "[D]\n"
        "pi = 3.14\n"
        "bad = abc\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    double v{};
    EXPECT_TRUE(res.config->getDouble("D", "pi", v));
    EXPECT_DOUBLE_EQ(v, 3.14);

    EXPECT_FALSE(res.config->getDouble("D", "bad", v));
}

TEST(ConfigLoader, GetUInt32)
{
    auto res = loadFromString(
        "[U]\n"
        "dec = 42\n"
        "hex = 0x2A\n"
        "bin = 0b101010\n"
        "neg = -1\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    uint32_t v{};
    EXPECT_TRUE(res.config->getUInt32("U", "dec", v));
    EXPECT_EQ(v, 42u);

    EXPECT_TRUE(res.config->getUInt32("U", "hex", v));
    EXPECT_EQ(v, 42u);

    EXPECT_TRUE(res.config->getUInt32("U", "bin", v));
    EXPECT_EQ(v, 42u);

    EXPECT_FALSE(res.config->getUInt32("U", "neg", v));
}

TEST(ConfigLoader, GetInt64)
{
    auto res = loadFromString(
        "[I64]\n"
        "dec = -42\n"
        "hex = -0x2A\n"
        "bin = -0b101010\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_TRUE(res.config->getInt64("I64", "dec", v));
    EXPECT_EQ(v, -42);

    EXPECT_TRUE(res.config->getInt64("I64", "hex", v));
    EXPECT_EQ(v, -42);

    EXPECT_TRUE(res.config->getInt64("I64", "bin", v));
    EXPECT_EQ(v, -42);
}

TEST(ConfigLoader, DataAccess)
{
    auto res = loadFromString(
        "[A]\n"
        "k = v\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    const auto& data = res.config->data();

    ASSERT_EQ(data.size(), 1u);

    auto secIt = data.find("A");
    ASSERT_NE(secIt, data.end());

    const auto& section = secIt->second;

    auto keyIt = section.find("k");
    ASSERT_NE(keyIt, section.end());

    EXPECT_EQ(keyIt->second, "v");
    // Specific AMap apis
    EXPECT_FALSE(data.contains("B"));
    EXPECT_TRUE(section.contains("k"));
}

TEST(ConfigLoader, GetSection)
{
    auto res = loadFromString(
        "[MESSAGES]\n"
        "weight_center_start = 0x1A5,P,S\n"
        "weight_center_stop  = 0x1A5,P,E\n"
        "intertial_get_azimuth = 0x01A5,C,A\n"
        "inertial_get_angle = 0x01A5,C,I\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    const auto* section = res.config->getSection("MESSAGES");
    ASSERT_NE(section, nullptr);

    EXPECT_EQ(section->size(), 4u);

    EXPECT_EQ(section->at("weight_center_start"), "0x1A5,P,S");
    EXPECT_EQ(section->at("weight_center_stop"),  "0x1A5,P,E");
    EXPECT_EQ(section->at("intertial_get_azimuth"), "0x01A5,C,A");
    EXPECT_EQ(section->at("inertial_get_angle"), "0x01A5,C,I");
}

// ===================== LOAD FILE =====================

TEST(ConfigLoader, LoadFromFileOk)
{
    auto tmp = std::filesystem::temp_directory_path() / "cfg_test.ini";

    {
        std::ofstream f(tmp);
        f << "[A]\nkey=123\n";
    }

    auto res = ConfigLoader::load(tmp);

    EXPECT_EQ(res.error, ConfigError::Ok);

    int v{};
    EXPECT_TRUE(res.config->getInt("A", "key", v));
    EXPECT_EQ(v, 123);

    std::filesystem::remove(tmp);
}

TEST(ConfigLoader, ErrorStringConversion)
{
    ConfigLoader loader;

    EXPECT_EQ(ConfigLoader::errorToStr(ConfigError::Ok), "Ok");
    EXPECT_EQ(ConfigLoader::errorToStr(ConfigError::FileError), "FileError");
    EXPECT_EQ(ConfigLoader::errorToStr(ConfigError::MalformedLine), "MalformedLine");
    EXPECT_EQ(ConfigLoader::errorToStr(ConfigError::DuplicateKey), "DuplicateKey");
    EXPECT_EQ(ConfigLoader::errorToStr(ConfigError::CapacityError), "CapacityError");
    
    EXPECT_EQ(ConfigLoader::errorToStr(static_cast<ConfigError>(999)), "Unknown");
}

// ===================== INVALID OPERATIONS =====================

TEST(ConfigLoader, FileError)
{
    auto res = ConfigLoader::load("non_existing_config.ini");

    EXPECT_EQ(res.error, ConfigError::FileError);
}

TEST(ConfigLoader, MissingEquals)
{
    auto res = loadFromString(
        "[A]\n"
        "key value\n"
    );

    EXPECT_EQ(res.error, ConfigError::MalformedLine);
    EXPECT_EQ(res.errorLine, 2);
}

TEST(ConfigLoader, EmptyKey)
{
    auto res = loadFromString(
        "[A]\n"
        "= value\n"
    );

    EXPECT_EQ(res.error, ConfigError::MalformedLine);
}

TEST(ConfigLoader, NoSection)
{
    auto res = loadFromString(
        "key = value\n"
    );

    EXPECT_EQ(res.error, ConfigError::MalformedLine);
}

TEST(ConfigLoader, EmptySection)
{
    auto res = loadFromString(
        "[]\n"
    );

    EXPECT_EQ(res.error, ConfigError::MalformedLine);
}

TEST(ConfigLoader, DuplicateKey)
{
    auto res = loadFromString(
        "[A]\n"
        "k = 1\n"
        "k = 2\n"
    );

    EXPECT_EQ(res.error, ConfigError::DuplicateKey);
    EXPECT_EQ(res.errorLine, 3);
}

TEST(ConfigLoader, UnterminatedQuote)
{
    auto res = loadFromString(
        "[A]\n"
        "k = \"abc\n"
    );

    EXPECT_EQ(res.error, ConfigError::MalformedLine);
}

TEST(ConfigLoader, InvalidEscape)
{
    auto res = loadFromString(
        "[A]\n"
        "k = \"a\\q\"\n"
    );

    EXPECT_EQ(res.error, ConfigError::MalformedLine);
}


TEST(ConfigLoader, ParseValueInvalidEscape_Isolated)
{
    ConfigLoader::ConfigResult res;

    ConfigLoader::parse(
        "[A]\n"
        "k = \"a\\q\"\n",
        res
    );

    EXPECT_EQ(res.error, ConfigError::MalformedLine);
    EXPECT_EQ(res.errorLine, 2);
}

TEST(ConfigLoader, GetDoubleTrailingGarbage)
{
    auto res = loadFromString(
        "[D]\n"
        "v = 3.14abc\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    double v{};
    EXPECT_FALSE(res.config->getDouble("D", "v", v));
}

TEST(ConfigLoader, GetInt64Overflow)
{
    auto res = loadFromString(
        "[I64]\n"
        "big = 9223372036854775808\n"   // INT64_MAX + 1
        "small = -9223372036854775809\n" // INT64_MIN - 1
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_FALSE(res.config->getInt64("I64", "big", v));
    EXPECT_FALSE(res.config->getInt64("I64", "small", v));
}

TEST(ConfigLoader, GetInt64HexOverflow)
{
    auto res = loadFromString(
        "[I64]\n"
        "hex = 0x8000000000000000\n" // > INT64_MAX
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_FALSE(res.config->getInt64("I64", "hex", v));
}

TEST(ConfigLoader, GetInt64HexNegativeOverflow)
{
    auto res = loadFromString(
        "[I64]\n"
        "hex = -0x8000000000000001\n" // < INT64_MIN
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_FALSE(res.config->getInt64("I64", "hex", v));
}

TEST(ConfigLoader, GetInt64HexMaxBoundary)
{
    auto res = loadFromString(
        "[I64]\n"
        "hex = 0x7FFFFFFFFFFFFFFF\n" // INT64_MAX
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_TRUE(res.config->getInt64("I64", "hex", v));
    EXPECT_EQ(v, 9223372036854775807LL);
}

TEST(ConfigLoader, GetInt64HexMinBoundary)
{
    auto res = loadFromString(
        "[I64]\n"
        "hex = -0x8000000000000000\n" // INT64_MIN
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_FALSE(res.config->getInt64("I64", "hex", v));
}

TEST(ConfigLoader, GetInt64DecMinBoundary)
{
    auto res = loadFromString(
        "[I64]\n"
        "dec = -9223372036854775808\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_FALSE(res.config->getInt64("I64", "dec", v));
}

TEST(ConfigLoader, GetInt64DecMaxBoundary)
{
    auto res = loadFromString(
        "[I64]\n"
        "dec = 9223372036854775807\n" // INT64_MAX
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_TRUE(res.config->getInt64("I64", "dec", v));
    EXPECT_EQ(v, std::numeric_limits<int64_t>::max());
}

TEST(ConfigLoader, GetInt64LeadingZeros)
{
    auto res = loadFromString(
        "[I64]\n"
        "dec = 0009223372036854775807\n" // INT64_MAX with leading zeros
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    int64_t v{};
    EXPECT_TRUE(res.config->getInt64("I64", "dec", v));
    EXPECT_EQ(v, std::numeric_limits<int64_t>::max());
}

TEST(ConfigLoader, GetSection_NotFound)
{
    auto res = loadFromString(
        "[A]\n"
        "k = v\n"
    );

    ASSERT_EQ(res.error, ConfigError::Ok);

    const auto* section = res.config->getSection("B");
    EXPECT_EQ(section, nullptr);
}

}// namespace TestConfigLoader