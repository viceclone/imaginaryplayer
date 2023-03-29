#pragma once

#include <string>
#include <vector>
#include <filesystem>

class Track
{
public:
    Track() = default;
    ~Track() = default;

    bool initFromFile(std::filesystem::path path);

    // Getters
    const std::string& title() const;

    const std::string& artist() const;

    const std::string& codec() const;

    int duration() const;

    const std::string& content() const;

private:
    bool parseKeyValue(const std::string& key, const std::string& val);

    std::string m_title;
    std::string m_artist{"unknown"};
    std::string m_codec;
    int m_durationMs{0}; // track duration in milliseconds
    std::string m_content;
};