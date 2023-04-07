#include <fstream>
#include "core/track.hpp"

bool Track::parseKeyValue(const std::string& key, const std::string& val)
{
    if (key == "title")
    {
        m_title = val;
    }
    else if (key == "artist")
    {
        m_artist = val;
    }
    else if (key == "codec")
    {
        m_codec = val;
    }
    else if (key == "duration")
    {
        m_durationMs = std::stoi(val);
    }
    else if (key == "content")
    {
        m_content = val;
    } 
    else
    {
        return false;
    }

    return true;
}

bool Track::initFromFile(std::filesystem::path path)
{
    std::ifstream in(path);
    std::string key, val;
    while (in >> key >> val)
    {
        // TODO: check if key is duplicated
        if (!parseKeyValue(key, val))
        {
            return false;
        }
    }
    return true;
}

// Getters
const std::string& Track::title() const
{
    return m_title;
}

const std::string& Track::artist() const
{
    return m_artist;
}

const std::string& Track::codec() const
{
    return m_codec;
}

int Track::duration() const 
{
    return m_durationMs;
}

const std::string& Track::content() const
{
    return m_content;
}

char Track::streamCurrentContent()
{
    if (m_currentContentIndex < m_content.size())
    {
        auto c = m_content.at(m_currentContentIndex);
        m_currentContentIndex++;
        if (m_currentContentIndex == m_content.size())
        {
            m_endOfTrack = true;
        }
        return c;
    }
    else
    {
        return 0;
    }
}

void Track::resetCurrentContentIndex()
{
    m_currentContentIndex = 0;
    m_endOfTrack = false;
}

bool Track::endOfTrack() const
{
    return m_endOfTrack;
}