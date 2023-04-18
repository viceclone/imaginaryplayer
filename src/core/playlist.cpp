#include "core/playlist.hpp"
#include "core/logger.hpp"
#include <fstream>
#include <set>

void Playlist::setName(const std::string& name)
{
    m_name = name;
}

void Playlist::setDescription(const std::string& description)
{
    m_description = description;
}

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

void Playlist::exportToFile(std::filesystem::path path)
{
    std::ofstream os(path);
    os << m_name << std::endl;
    os << m_description << std::endl;
    for (auto track : m_tracks)
    {
        os << track->path() << std::endl;
    }
    os.close();
}

void Playlist::validate(bool valid)
{
    m_isValid = valid;
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
    // if (m_tracks.empty())
    // {
    //     return nullptr;
    // }
    // else
    // {
    //     return *m_currentTrackIter;
    // }
    return m_currentTrack;
}

std::shared_ptr<Track> Playlist::nextTrack(bool autoplay)
{
    if (m_tracks.empty())
    {
        m_currentTrack = nullptr;
        return nullptr;
    }

    if (m_repeatMode == RepeatMode::RepeatCurrentSong)
    {
        if (autoplay)
        {
            // Return the current song and do nothing else
            return m_currentTrack;
        }
        else
        {
            m_repeatMode = RepeatMode::RepeatWholePlaylist;
        }
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
                m_currentTrack = nullptr;
                return nullptr;
            }
            
        }
    }
    else
    {
        if (m_currentTrackIter == m_tracks.end())
        {
            m_currentTrack = nullptr;
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
                m_currentTrack = nullptr;
                return nullptr;
            }
        }
    }

    m_currentTrack = *m_currentTrackIter;
    return m_currentTrack;
}

std::shared_ptr<Track> Playlist::previousTrack()
{
    if (m_tracks.empty())
    {
        m_currentTrack = nullptr;
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
                m_currentTrack = nullptr;
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
                m_currentTrack = nullptr;
                return nullptr;
            }
        }
    }
    m_currentTrack = *m_currentTrackIter;
    return m_currentTrack;
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

    if (m_tracks.size() == 1)
    {
        if (m_isShuffled)
        {
            m_currentTrackIter = m_shuffledPlaylist.begin();
        }
        else
        {
            m_currentTrackIter = m_tracks.begin();
        }
        m_currentTrack = *m_currentTrackIter;
    }
}

bool Playlist::removeTrack(int trackIdx)
{
    if (trackIdx > m_tracks.size() || trackIdx < 0)
    {
        return false;
    }
    auto iter = std::next(m_tracks.begin(), trackIdx);
    auto trackToRemove = *iter;
    if (iter == m_currentTrackIter)
    {
        m_currentTrackIter = m_tracks.erase(iter);
        iter = m_currentTrackIter;
    }
    else
    {
        iter = m_tracks.erase(iter);
    }
    
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

struct TrackPtrComp
{
    bool operator()(const TrackPtr& lhs, const TrackPtr& rhs) const { 
        return (lhs->title() < rhs->title()) && (lhs->artist() < rhs->artist()); 
    }
};

void Playlist::removeDuplicate()
{
    std::set<TrackPtr, TrackPtrComp> found;
    for (auto iter = m_tracks.begin(); iter != m_tracks.end(); )
    {
        if (!found.insert(*iter).second)
        {
            iter = m_tracks.erase(iter);
            for (auto iter2 = m_shuffledPlaylist.begin(); m_isShuffled && iter2 != m_shuffledPlaylist.end(); ++iter2)
            {
                if (*iter2 == *iter)
                {
                    m_shuffledPlaylist.erase(iter2);

                    if (!m_shuffledPlaylist.empty() && m_isShuffled && iter2 == m_shuffledPlaylist.end())
                    {
                        m_currentTrackIter = m_shuffledPlaylist.begin();
                    }
                    break;
                }
            }
        }
        else
        {
            ++iter;
        }      
    }
}

bool Playlist::isShuffled() const
{
    return m_isShuffled;
}

void Playlist::shuffle()
{
    if (m_tracks.size() == 0)
    {
        m_shuffledPlaylist = std::list<TrackPtr>{};
        return;
    }

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