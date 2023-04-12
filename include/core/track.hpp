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
    std::string path() const;

    const std::string& title() const;

    const std::string& artist() const;

    const std::string& codec() const;

    int duration() const;

    const std::string& content() const;

    char streamCurrentContent();

    void resetCurrentContentIndex();

    bool endOfTrack() const;

private:
    bool parseKeyValue(const std::string& key, const std::string& val);

    std::filesystem::path m_path;
    std::string m_title;
    std::string m_artist{"unknown"};
    std::string m_codec;
    int m_durationMs{0}; // track duration in milliseconds
    std::string m_content;
    int m_currentContentIndex{0}; // a cursor to the current position in the track content
    bool m_endOfTrack{false};
};