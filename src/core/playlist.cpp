#include "core/playlist.hpp"
#include "core/logger.hpp"
#include <fstream>

const std::string& Playlist::name() const
{
    return m_name;
}

const std::string& Playlist::description() const
{
    return m_description;
}

const TrackList & Playlist::tracks() const
{
    return m_tracks;
}

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

    resetToFirstTrack();
    return count;
}

int Playlist::importFromFile(std::filesystem::path path)
{
    std::error_code ec;
    if (!fs::exists(path, ec) || !fs::is_regular_file(path, ec))
    {
        ERROR_EC_MSG(ec);
        return 0;
    }

    m_path = path;
    std::ifstream in(path);
    std::string line;
    // Get playlist name
    if (getline(in, line) && line != "")
    {
        m_name = line;
    }
    else
    {
        ERROR_LOG("Playlist name is missing (corrupted file)");
        return 0;
    }

    if (getline(in, line))
    {
        m_description = line;
    }
    else
    {
        ERROR_LOG("Playlist description is missing (corrupted file)");
        return 0;
    }

    int count = 0;
    m_tracks.clear();
    m_shuffledPlaylist.clear();
    auto parentPath = path.parent_path();
    for (std::string line; getline(in, line);)
    {
        TrackPtr track = std::make_shared<Track>();
        if (track->initFromFile(parentPath / fs::path(line)))
        {
            addTrack(track);
            count++;
        }
    }
    m_isValid = true;
    return count;
}

bool Playlist::isValid() const
{
    return m_isValid;
}

int Playlist::size() const
{
    return m_tracks.size();
}

std::shared_ptr<Track> Playlist::resetToFirstTrack()
{
    if (m_tracks.empty())
    {
        return nullptr;
    }

    if (m_isShuffled)
    {
        m_currentTrackIter = m_shuffledPlaylist.begin();
    }
    else
    {
        m_currentTrackIter = m_tracks.begin();
    }

    return *m_currentTrackIter;
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

std::shared_ptr<Track> Playlist::nextTrack(bool autoplay)
{
    if (m_tracks.empty())
    {
        return nullptr;
    }

    if (autoplay && m_repeatMode == RepeatMode::RepeatCurrentSong)
    {
        // Return the current song and do nothing else
        return *m_currentTrackIter;
    }

    if (m_isShuffled)
    {
        if (m_currentTrackIter == m_shuffledPlaylist.end())
        {
            return nullptr;
        }
        ++m_currentTrackIter;
        if (m_currentTrackIter == m_shuffledPlaylist.end())
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
    }
    else
    {
        if (m_currentTrackIter == m_tracks.end())
        {
            return nullptr;
        }

        ++m_currentTrackIter;
        if (m_currentTrackIter == m_tracks.end())
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
    }

    return *m_currentTrackIter;
}

std::shared_ptr<Track> Playlist::previousTrack()
{
    if (m_tracks.empty())
    {
        return nullptr;
    }

    if (m_repeatMode == RepeatMode::RepeatCurrentSong)
    {
        // Return the current song and do nothing else
        return *m_currentTrackIter; 
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

bool Playlist::isShuffled() const
{
    return m_isShuffled;
}

void Playlist::shuffle()
{
    std::vector<std::reference_wrapper<const TrackPtr>> v(m_tracks.begin(), m_tracks.end());
    std::shuffle(v.begin(), v.end(), std::mt19937{ std::random_device{}()});
    TrackList shuffled;
    for (auto &ref : v) 
    {
        if (ref.get() == *m_currentTrackIter)
        {
            shuffled.push_front(std::move(ref.get()));
        }
        else
        {
            shuffled.push_back(std::move(ref.get()));
        }
    }
    
    m_shuffledPlaylist.swap(shuffled);
    m_isShuffled = true;
    m_currentTrackIter = m_shuffledPlaylist.begin();
}

void Playlist::unshuffle()
{
    m_isShuffled = false;
    if (m_currentTrackIter == m_shuffledPlaylist.end())
    {
        m_currentTrackIter = m_tracks.begin();
        m_shuffledPlaylist.clear();
        return;
    }

    for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it)
    {
        if (*m_currentTrackIter == *it)
        {
            m_currentTrackIter = it;
            break;
        }
    }
    m_shuffledPlaylist.clear();
}

RepeatMode Playlist::getRepeatMode() const
{
    return m_repeatMode;
}

void Playlist::repeat()
{
    switch (m_repeatMode)
    {
    case RepeatMode::NoRepeat:
        m_repeatMode = RepeatMode::RepeatWholePlaylist;
        break;
    case RepeatMode::RepeatWholePlaylist:
        m_repeatMode = RepeatMode::RepeatCurrentSong;
        break;
    case RepeatMode::RepeatCurrentSong:
        m_repeatMode = RepeatMode::NoRepeat;
        break;
    default:
        break;
    }
    
}

void Playlist::clear()
{
    m_tracks.clear();
    m_shuffledPlaylist.clear();
}