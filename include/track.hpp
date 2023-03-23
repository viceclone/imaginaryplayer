#pragma once

#include <string>
#include <vector>

class Track
{
public:

private:
    std::string m_title;
    std::string m_codec;
    int durationMs{0}; // track duration in millisecond
    std::vector<int8_t> m_content;
};