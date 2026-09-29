/**
 * @file ConfigLoader.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

 #include "core/ConfigLoader.hpp"

 #include <charconv>
 #include <cstdlib>
 #include <cstdint>


namespace Core
{

const Config::Section* Config::getSection(std::string_view section) const noexcept
{
    auto sectionIt = m_data.find(std::string(section));

    if (sectionIt == m_data.end())
    {
        return nullptr;
    }
        
    return &sectionIt->second;
}

const std::string& Config::get(std::string_view section, std::string_view key) const noexcept
{
    static const std::string empty_string;

    auto sectionIt = m_data.find(std::string(section));

    if (sectionIt == m_data.end())
    {
        return empty_string;
    }
    
    auto keyIt = sectionIt->second.find(key);

    if (keyIt == sectionIt->second.end())
    {
        return empty_string;
    } 

    return keyIt->second;
}

bool Config::getInt(std::string_view section, std::string_view key, int& out) const noexcept
{
    const auto& value = get(section, key);

    if (value.empty())
    {
        return false;
    } 

    const char* begin = value.data();
    const char* end = begin + value.size();

    int base = 10;

    if (value.size() > 2 && value.rfind("0x", 0) == 0)
    {
        begin += 2;
        base = 16;
    }
    else if (value.size() > 2 && value.rfind("0b", 0) == 0)
    {
        begin += 2;
        base = 2;
    }

    auto [ptr, errorCode] = std::from_chars(begin, end, out, base);

    return (errorCode == std::errc{}) && (ptr == end);
}

bool Config::getBool(std::string_view section, std::string_view key, bool& out) const noexcept
{
    const auto& value = get(section, key);

    if (value.empty())
    {
        return false;
    } 

    if (value == "true" || value == "1") 
    {
        out = true;
        return true;
    }

    if (value == "false" || value == "0")
    {
        out = false;
        return true;
    }

    return false;
}

bool Config::getDouble(std::string_view section, std::string_view key, double& out) const noexcept
{
    const auto& value = get(section, key);

    if (value.empty())
    {
        return false;
    } 

    char* end{};
    const char* value_end = value.c_str() + value.size();

    out = std::strtod(value.c_str(), &end);

    return end == value_end;
}

// Stricter control over parsing compared to GetInt()
bool Config::getUInt32(std::string_view section, std::string_view key, uint32_t& out) const noexcept
{
    const auto& value = get(section, key);

    if (value.empty())
    {
        return false;
    } 

    if (value[0] == '-') 
    {
        return false;
    }

    const char* begin = value.data();
    const char* end = begin + value.size();

    int base = 10;

    if (value.size() > 2 && value.rfind("0x", 0) == 0)
    {
        begin += 2;
        base = 16;
    }
    else if (value.size() > 2 && value.rfind("0b", 0) == 0)
    {
        begin += 2;
        base = 2;
    }

    uint32_t tmp{};
    auto [ptr, ec] = std::from_chars(begin, end, tmp, base);

    if (ec != std::errc{} || ptr != end)
    {
        return false;
    }
    
    out = tmp;
    return true;
}

// Stricter control over parsing compared to GetInt()
bool Config::getInt64(std::string_view section, std::string_view key, int64_t& out) const noexcept
{
    const auto& value = get(section, key);

    if (value.empty())
    {
        return false;
    } 

    const char* begin = value.data();
    const char* end = begin + value.size();

    int base = 10;

    bool negative = false;
    if (begin < end && *begin == '-')
    {
        negative = true;
        ++begin;
    }

    if ((end - begin) > 2 && std::string_view(begin, end - begin).rfind("0x",0) == 0)
    {
        begin += 2;
        base = 16;
    }
    else if ((end - begin) > 2 && std::string_view(begin, end - begin).rfind("0b",0) == 0)
    {
        begin += 2;
        base = 2;
    }

    int64_t tmp{};
    auto [ptr, ec] = std::from_chars(begin, end, tmp, base);

    if (ec != std::errc{} || ptr != end)
    {
        return false;
    }

    out = negative ? -tmp : tmp;

    return true;
}

const Config::Data& Config::data() const noexcept 
{ 
    return m_data; 
}

// =================================================== CONFIG LOADER ============================================================

ConfigLoader::ConfigResult ConfigLoader::load(const std::filesystem::path& path)
{
    ConfigResult result;
    result.config = std::make_unique<Config>();

    auto configFile = FileLoader::loadFile(path);

    if (configFile.error != FileLoader::FileResult::Ok)
    {
        result.error = ConfigError::FileError;
        result.config.reset();
        return result;
    }

    const auto& configFileBuffer = configFile.file->buffer();

    std::string_view configFileContent(reinterpret_cast<const char*>(configFileBuffer.data()), configFileBuffer.size());

    parse(configFileContent, result);

    return result;
}

void ConfigLoader::parse(std::string_view content, ConfigResult& cfg)
{
    if (!cfg.config)
    {
        cfg.config = std::make_unique<Config>();
    }
        
    std::string currentSection;
    size_t pos = 0;
    size_t lineNo = 1;

    while (pos < content.size())
    {
        size_t end = content.find('\n', pos);

        if (end == std::string_view::npos) 
        {
            end = content.size();
        }

        std::string_view raw = content.substr(pos, end - pos);

        std::string_view strippedLine = trim(stripComment(raw));

        if (strippedLine.empty())
        {
            pos = end + 1;
            ++lineNo;
            continue;
        }

        if (strippedLine.front() == '[' && strippedLine.back() == ']')
        {
            currentSection = std::string(trim(strippedLine.substr(1, strippedLine.size() - 2)));

            if (currentSection.empty())
            {
                cfg.error = ConfigError::MalformedLine;
                cfg.errorLine = lineNo;
                return;
            }
        }
        else
        {
            if (currentSection.empty())
            {
                cfg.error = ConfigError::MalformedLine;
                cfg.errorLine = lineNo;
                return;
            }

            auto valueAfterEq = strippedLine.find('=');

            // Allow empty values after =
            if (valueAfterEq == std::string_view::npos || valueAfterEq == 0)
            {
                cfg.error = ConfigError::MalformedLine;
                cfg.errorLine = lineNo;
                return;
            }

            auto key = trim(strippedLine.substr(0, valueAfterEq));
            auto value = trim(strippedLine.substr(valueAfterEq + 1));

            if (key.empty())
            {
                cfg.error = ConfigError::MalformedLine;
                cfg.errorLine = lineNo;
                return;
            }

            bool valueOk{};
            auto parsedVal = parseValue(value, valueOk);

            if (!valueOk)
            {
                cfg.error = ConfigError::MalformedLine;
                cfg.errorLine = lineNo;
                return;
            }

            auto sectionIt = cfg.config->m_data.find(currentSection);

            if (sectionIt == cfg.config->m_data.end())
            {
                auto tmp = cfg.config->m_data.emplace(currentSection, Config::Section{});

                if (!tmp.second)
                {
                    cfg.error = ConfigError::CapacityError;
                    cfg.errorLine = lineNo;
                    return;
                }

                sectionIt = tmp.first;
            }

            auto& section = sectionIt->second;

            if (section.find(std::string(key)) != section.end())
            {
                cfg.error = ConfigError::DuplicateKey;
                cfg.errorLine = lineNo;
                return;
            }

            auto res = section.emplace(std::string(key), std::move(parsedVal));

            if (!res.second)
            {
                cfg.error = ConfigError::CapacityError;
                cfg.errorLine = lineNo;
                return;
            }
        }

        pos = end + 1;
        ++lineNo;
    }

    cfg.error = ConfigError::Ok;
}

std::string ConfigLoader::parseValue(std::string_view inputValue, bool& ok)
{
    std::string_view value = trim(inputValue);

    ok = true;

    // Allow empty values like "key = "
    if (value.empty())
    {
        return {};
    }

    if (value.front() == '"')
    {
        if (value.size() < 2 || value.back() != '"')
        {
            ok = false;
            return {};
        }

        std::string out;
        out.reserve(value.size() - 2);

        for (size_t i = 1; i < value.size() - 1; ++i)
        {
            if (value[i] == '\\' && i + 1 < value.size() - 1)
            {
                char next = value[++i];
                switch (next)
                {
                    case 'n':
                        out += '\n';
                        break;
                    case 't':
                        out += '\t';
                        break;
                    case '\\':
                        out += '\\';
                        break;
                    case '"':
                        out += '"';
                        break;
                    case 'x':
                        // literal 'x' (no hex parsing)
                        out += 'x';
                        break;
                    default:
                        ok = false;
                        return {};
                }
            }
            else
            {
                out += value[i];
            }
        }

        return out;
    }

    return std::string(value);
}

std::string_view ConfigLoader::trim(std::string_view s)
{
    const char* ws = " \t\r\n";
    const auto begin = s.find_first_not_of(ws);
    if (begin == std::string_view::npos) return {};

    const auto end = s.find_last_not_of(ws);
    return s.substr(begin, end - begin + 1);
}

std::string_view ConfigLoader::stripComment(std::string_view line)
{
    bool inQuote = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (c == '"' && (i == 0 || line[i - 1] != '\\'))
        {
            inQuote = !inQuote;
        }
            
        if (!inQuote && (c == ';' || c == '#'))
        {
            return line.substr(0, i);
        }
    }

    return line;
}

std::string_view ConfigLoader::errorToStr(ConfigError error) noexcept
{
    switch (error)
    {
        case ConfigError::Ok:            return "Ok";
        case ConfigError::FileError:     return "FileError";
        case ConfigError::MalformedLine: return "MalformedLine";
        case ConfigError::DuplicateKey:  return "DuplicateKey";
        case ConfigError::CapacityError: return "CapacityError";
        default:                         return "Unknown";
    }
}

}//namespace Core