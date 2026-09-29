/**
 * @file ConfigLoader.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once

#include <string>
#include <string_view>
#include <memory>
// For tests only
// #include <unordered_map>

#include "core/AMap.hpp"
#include "Config.hpp"
#include "core/FileLoader.hpp"

/**
 * @page ConfigLoaderPage Configiguration Loader
 *
 * @brief Strict INI configuration parser.
 *
 * @details
 * This module parses `.ini` files into a structured configuration:
 *
 * - Sections are mandatory
 * - Key/value pairs are stored as strings
 * - Type conversion is performed at access time
 * - Parsing stops on first error, partial data is preserved
 *
 * Implementation reference: :contentReference[oaicite:0]{index=0}
 *
 * ---
 *
 * @section format Supported Format
 *
 * General form:
 * @code
 * [SECTION]
 * key = value
 * @endcode
 *
 * ---
 *
 * @section valid_examples Valid Examples
 *
 * Basic:
 * @code
 * [NETWORK]
 * port = 8080
 * host = localhost
 * enabled = true
 * @endcode
 *
 * Quoted values:
 * @code
 * [MESSAGES]
 * inertial_start = "0x1A2, P, S"
 * @endcode
 *
 * Spaces and special characters:
 * @code
 * [TEST]
 * a = "value with spaces"
 * b = "value;still data"
 * c = "value # still data"
 * @endcode
 *
 * Escapes:
 * @code
 * [ESCAPE]
 * path = "C:\\dir\\file.txt"
 * newline = "line1\nline2"
 * quote = "value \"inside\""
 * literal = "value \x"
 * @endcode
 *
 * Empty values:
 * @code
 * [EMPTY]
 * key1 =
 * key2 = ""
 * @endcode
 *
 * Numeric formats (parsed at access time):
 * @code
 * [NUM]
 * dec = 42
 * hex = 0x1A2
 * bin = 0b1010
 * @endcode
 *
 * ---
 *
 * @section ignored_lines Ignored Lines
 *
 * Full-line comments:
 * @code
 * ; comment
 * # comment
 * @endcode
 *
 * Inline comments:
 * @code
 * key = value ; comment
 * key = value # comment
 * @endcode
 *
 * Leading whitespace:
 * @code
 *    ; comment
 * @endcode
 *
 * Mixed:
 * @code
 * [SECTION] ; comment
 * key = 123 # comment
 * @endcode
 *
 * Comments inside quotes are preserved:
 * @code
 * key = "value ; not a comment"
 * @endcode
 *
 * ---
 *
 * @section invalid_examples Invalid Examples
 *
 * Missing '=':
 * @code
 * key value
 * @endcode
 *
 * Empty key:
 * @code
 * = value
 * @endcode
 *
 * No section:
 * @code
 * key = value
 * @endcode
 *
 * Empty section:
 * @code
 * [ ]
 * @endcode
 *
 * Duplicate key:
 * @code
 * [A]
 * key = 1
 * key = 2
 * @endcode
 *
 * Unterminated string:
 * @code
 * key = "value
 * @endcode
 *
 * Invalid escape:
 * @code
 * key = "value \q"
 * @endcode
 *
 * ---
 *
 * @section parsing_rules Parsing Rules
 *
 * - Sections must be defined before keys
 * - Keys must be unique per section
 * - Comments are stripped unless inside quotes
 * - Quotes must be properly closed
 * - Escape sequences are strictly validated
 *
 * Supported escapes:
 * - \n
 * - \t
 * - \\
 * - \"
 * - \x (literal 'x')
 *
 * ---
 * 
 * @section error_handling Error Handling
 *
 * - Parsing stops at first error
 * - Error type and line are reported
 * - Partial configuration is preserved
 * 
 * ---
 *
 * @section type_conversion Type Conversion
 *
 * Conversion is performed via accessors:
 *
 * - getInt()
 * - getUInt32()
 * - getInt64()
 * - getDouble()
 * - getBool()
 *
 * Supported numeric formats:
 * - Decimal: 42
 * - Hex: 0x1A2
 * - Binary: 0b1010
 *
 * Notes:
 * - No signed hex/bin support
 * - Empty values return failure in getters
 * 
 * ---
 * 
 * @section type_conversion_examples Type Conversion Examples
 *
 * @subsection getInt_example getInt()
 * @code
 * [NUM]
 * value = 42
 * hex   = 0x1A
 * bin   = 0b1010
 *
 * int v{};
 * config.getInt("NUM", "value", v); // v = 42
 * config.getInt("NUM", "hex", v);   // v = 26
 * config.getInt("NUM", "bin", v);   // v = 10
 * @endcode
 *
 * @subsection getUInt32_example getUInt32()
 * @code
 * [NUM]
 * value = 123
 * hex   = 0xFF
 *
 * uint32_t v{};
 * config.getUInt32("NUM", "value", v); // v = 123
 * config.getUInt32("NUM", "hex", v);   // v = 255
 *
 * // invalid:
 * // value = -1  -> rejected
 * @endcode
 *
 * @subsection getInt64_example getInt64()
 * @code
 * [NUM]
 * value = -9223372036854775807
 * hex   = 0x1FFFFFFFF
 *
 * int64_t v{};
 * config.getInt64("NUM", "value", v); // negative decimal OK
 * config.getInt64("NUM", "hex", v);   // large hex OK
 *
 * // note:
 * // signed hex like "-0x1A" is not supported
 * @endcode
 *
 * @subsection getDouble_example getDouble()
 * @code
 * [NUM]
 * pi = 3.14159
 *
 * double v{};
 * config.getDouble("NUM", "pi", v); // v ≈ 3.14159
 * @endcode
 *
 * @subsection getBool_example getBool()
 * @code
 * [BOOL]
 * a = true
 * b = false
 * c = 1
 * d = 0
 *
 * bool v{};
 * config.getBool("BOOL", "a", v); // true
 * config.getBool("BOOL", "b", v); // false
 * config.getBool("BOOL", "c", v); // true
 * config.getBool("BOOL", "d", v); // false
 * 
 * // invalid:
 * // value = yes -> rejected
 *
 * @endcode
 * 
 * ---
 *
 */

namespace Core
{

/**
 * @enum ConfigError
 * @brief Error codes produced during parsing.
 */
enum class ConfigError
{
    Ok,
    FileError, // File load error
    MalformedLine, // Syntax error
    DuplicateKey, // Duplicate key in same section
    CapacityError
};    


/**
 * @class Config
 * @brief Stores parsed configuration data, organized by sections and keys.
 *
 * @details
 * Internal structure:
 * - Section → key/value map
 * - Values stored as strings
 * - Type conversion performed on access
 */
class Config
{
public:
    // For tests only !
    // using Section = std::unordered_map<std::string, std::string>;
    // using Data = std::unordered_map<std::string, Section>;

    using Section = AMap<std::string, std::string, Core::CONFIG_SECTION_MAX_SIZE>;
    using Data = AMap<std::string, Section, Core::CONFIG_MAX_SECTIONS>;

    Config()=default;
    ~Config()=default;

    /**
     * @brief Get the Section object
     * 
     * @param section 
     * @return const Section* = AMap<std::string, std::string, Core::CONFIG_SECTION_MAX_SIZE>*
     */
    const Section* getSection(std::string_view section) const noexcept;

    /**
     * @brief Retrieve raw string value.
     *
     * @param section Section name
     * @param key Key name
     * @return Reference to stored value, or empty string if not found
     *
     * @note No error is reported; empty string indicates missing entry.
     */
    const std::string& get(std::string_view section, std::string_view key) const noexcept;

    /**
     * @brief Retrieve integer value.
     *
     * @param section Section name
     * @param key Key name
     * @param out Output integer
     * @return true if conversion succeeded
     *
     * @details
     * Supported formats:
     * - Decimal: 42
     * - Hex: 0x1A
     * - Binary: 0b1010
     *
     * @note Signed hex/binary not supported.
     */
    bool getInt(std::string_view section, std::string_view key, int& out) const noexcept;

    /**
     * @brief Retrieve unsigned 32-bit integer.
     *
     * @param section Section name
     * @param key Key name
     * @param out Output value
     * @return true if conversion succeeded
     *
     * @details
     * Same formats as getInt(), but:
     * - Negative values are rejected
     */
    bool getUInt32(std::string_view section, std::string_view key, uint32_t& out) const noexcept;

    /**
     * @brief Retrieve signed 64-bit integer.
     *
     * @param section Section name
     * @param key Key name
     * @param out Output value
     * @return true if conversion succeeded
     *
     * @details
     * Supports:
     * - Signed decimal
     * - Unsigned hex/binary
     *
     * @note Signed hex/binary not supported.
     */
    bool getInt64(std::string_view section, std::string_view key, int64_t& out) const noexcept;

    /**
     * @brief Retrieve floating-point value.
     *
     * @param section Section name
     * @param key Key name
     * @param out Output value
     * @return true if conversion succeeded
     *
     * @details
     * Uses std::strtod parsing.
     */
    bool getDouble(std::string_view section, std::string_view key, double& out) const noexcept;

    /**
     * @brief Retrieve boolean value.
     *
     * @param section Section name
     * @param key Key name
     * @param out Output value
     * @return true if conversion succeeded
     *
     * @details
     * Accepted values:
     * - "true", "1" → true
     * - "false", "0" → false
     */
    bool getBool(std::string_view section, std::string_view key, bool& out) const noexcept;

    /**
     * @brief Access full configuration data.
     *
     * @return Internal data structure
     */
    const Data& data() const noexcept;

private:
    Data m_data;
    friend class ConfigLoader;
};


/**
 * @class ConfigLoader
 * @brief Loads and parses INI configuration files.
 *
 * @details
 * Characteristics:
 * - Strict syntax
 * - Sections required
 * - Stops on first error
 * - Partial config preserved on failure
 */
class ConfigLoader
{
public:
    /**
     * @struct ConfigLoader::ConfigResult
     * @brief Result of configuration loading.
     */
    struct ConfigResult
    {
        std::unique_ptr<Config> config; // Parsed configuration (may be partial) in case of error
        ConfigError error{ConfigError::Ok}; //  Error status
        std::size_t errorLine{0}; // Line where error occurred
    };

    ConfigLoader()=default;
    ~ConfigLoader()=default;

    /**
     * @brief Load and parse configuration file.
     *
     * @param path File path
     * @return Parsing result
     *
     * @details
     * Steps:
     * - Load file via FileLoader
     * - Parse content line-by-line
     * - Stop at first error
     */
    static ConfigResult load(const std::filesystem::path& path);

    /**
     * @brief Parse raw file content.
     *
     * @param content File content
     * @param cfg Output result
     */
    static void parse(std::string_view content, ConfigResult& cfg);

    static std::string_view errorToStr(ConfigError error) noexcept;

private:
    /**
     * @brief Parse value with optional quoting and escaping.
     *
     * @param inputValue Raw value string
     * @param ok Output status
     * @return Parsed string
     *
     * @details
     * Supports:
     * - Quoted values
     * - Escapes: \n, \t, \\, \", \x (literal)
     *
     * Rejects:
     * - Invalid escape sequences
     * - Unterminated quotes
     */
    static std::string parseValue(std::string_view v, bool& ok);

    /**
     * @brief Remove comments from line.
     *
     * @param line Input line
     * @return Line without comments
     *
     * @details
     * - Removes content after ';' or '#'
     * - Ignores comment markers inside quotes
     */
    static std::string_view stripComment(std::string_view line);

    /**
     * @brief Trim whitespace from both ends.
     *
     * @param s Input string
     * @return Trimmed view
     */
    static std::string_view trim(std::string_view s);
};

} // namespace Core