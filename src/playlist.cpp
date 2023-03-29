#include "playlist.hpp"

int Playlist::importFromFolder(std::filesystem::path path)
{
    int count = 0;
    for (const auto & entry : fs::directory_iterator(path))
    {
        std::shared_ptr<Track> track = std::make_shared<Track>();
        if (track->initFromFile(entry.path()))
        {
            addTrack(track);
            count++;
        }
    }
    return count;
}

int Playlist::size() const
{
    return m_tracks.size();
}

std::shared_ptr<Track> Playlist::currentTrack()
{
    if (m_tracks.empty())
    {
        return nullptr;
    }
    else
    {
        return *m_currentTrackIter;
    }
}

std::shared_ptr<Track> Playlist::nextTrack()
{
    if (m_tracks.empty())
    {
        return nullptr;
    }

    ++m_currentTrackIter;
    if (m_isShuffled && m_currentTrackIter == m_shuffledPlaylist.end())
    {
        if (m_repeatMode == RepeatMode::RepeatWholePlaylist)
        {
            m_currentTrackIter = m_shuffledPlaylist.begin();
        }
        else
        {
            return nullptr;
        }
        
    }
    else if (!m_isShuffled && m_currentTrackIter == m_tracks.end())
    {
        if (m_repeatMode == RepeatMode::RepeatWholePlaylist)
        {
            m_currentTrackIter = m_tracks.begin();
        }
        else
        {
            return nullptr;
        }
    }

    return *m_currentTrackIter;
}

std::shared_ptr<Track> Playlist::previousTrack()
{
    if (m_tracks.empty())
    {
        return nullptr;
    }

    if (m_isShuffled)
    {
        if (m_currentTrackIter != m_shuffledPlaylist.begin())
        {
            --m_currentTrackIter;
        }
        else
        {
            if (m_repeatMode == RepeatMode::RepeatWholePlaylist)
            {
                m_currentTrackIter = std::prev(m_shuffledPlaylist.end(), 1);
            }
            else
            {
                return nullptr;
            }
        }
    }
    else 
    {
        if (m_currentTrackIter != m_tracks.begin())
        {
            --m_currentTrackIter;
        }
        else
        {
            if (m_repeatMode == RepeatMode::RepeatWholePlaylist)
            {
                m_currentTrackIter = std::prev(m_tracks.end(), 1);
            }
            else
            {
                return nullptr;
            }
        }
    }

    return *m_currentTrackIter;
}

void Playlist::addTrack(std::shared_ptr<Track> track)
{
    // Add the track at the end of the normal list
    m_tracks.push_back(track);
    // Add the track at a random point from the list
    if (m_shuffledPlaylist.empty())
    {
        m_shuffledPlaylist.push_back(track);
    }
    else
    {
        auto randomPos = helper::randomInt(0, m_shuffledPlaylist.size());
        auto iter = std::next(m_shuffledPlaylist.begin(), randomPos);
        m_shuffledPlaylist.insert(iter, track);
    }
}

bool Playlist::removeTrack(int trackIdx)
{
    if (trackIdx > m_tracks.size())
    {
        return false;
    }
    auto iter = std::next(m_tracks.begin(), trackIdx);
    auto trackToRemove = *iter;
    iter = m_tracks.erase(iter);
    if (!m_tracks.empty() && !m_isShuffled && iter == m_tracks.end())
    {
        m_currentTrackIter = m_tracks.begin();
    }

    for (auto iter2 = m_shuffledPlaylist.begin(); iter2 != m_shuffledPlaylist.end(); ++iter2)
    {
        if (*iter2 == trackToRemove)
        {
            m_shuffledPlaylist.erase(iter2);

            if (!m_shuffledPlaylist.empty() && m_isShuffled && iter2 == m_shuffledPlaylist.end())
            {
                m_currentTrackIter = m_shuffledPlaylist.begin();
            }
            break;
        }
    }
    return true;    
}

void Playlist::clear()
{
    m_tracks.clear();
    m_shuffledPlaylist.clear();
}