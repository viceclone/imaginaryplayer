#pragma once

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <optional>
#include <filesystem>

#include "track.hpp"
#include "enums.hpp"
#include "helper.hpp"

namespace fs = std::filesystem;

using TrackList = std::list<std::shared_ptr<Track>>;
using TrackListIterator = TrackList::iterator;

class Playlist
{
public:
    Playlist() = default;
    ~Playlist() = default;

    // Return the number of tracks added to the playlist
    int importFromFolder(std::filesystem::path path);

    int size() const;

    // Return the pointer to the current track
    std::shared_ptr<Track> currentTrack();
    // Switch to the next/previous track
    std::shared_ptr<Track> nextTrack();
    std::shared_ptr<Track> previousTrack();

    // Add track to the playlist
    void addTrack(std::shared_ptr<Track> track);
    // Return true if removal successful. False otherwise
    bool removeTrack(int trackIdx);

    void shuffle();
    void unshuffle();

    void clear();
private:
    std::string m_name;
    std::optional<std::string> m_description;
    TrackList m_tracks; // A track can be in different playlist, therefore they are included as shared pointers.
    TrackList m_shuffledPlaylist;
    TrackListIterator m_currentTrackIter;
    RepeatMode m_repeatMode{RepeatMode::NoRepeat};
    bool m_isShuffled{false};
};