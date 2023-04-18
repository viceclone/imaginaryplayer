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

using TrackPtr = std::shared_ptr<Track>;
using TrackList = std::list<std::shared_ptr<Track>>;
using TrackListIterator = TrackList::iterator;

class Playlist
{
public:
    Playlist() = default;
    ~Playlist() = default;

    void setName(const std::string& name);
    void setDescription(const std::string& description);

    const std::string& name() const;
    const std::string& description() const;
    const TrackList & tracks() const;

    // Return the number of tracks added to the playlist
    int importFromFolder(std::filesystem::path path);
    int importFromFile(std::filesystem::path path);
    void exportToFile(std::filesystem::path path);

    void validate(bool valid);
    bool isValid() const;

    int size() const;

    // Reset the pointer to the first track
    std::shared_ptr<Track> resetToFirstTrack();
    // Return the pointer to the current track
    std::shared_ptr<Track> currentTrack();
    // Switch to the next/previous track
    std::shared_ptr<Track> nextTrack(bool autoplay);
    std::shared_ptr<Track> previousTrack();

    // Add track to the playlist
    void addTrack(std::shared_ptr<Track> track);
    // Return true if removal successful. False otherwise
    bool removeTrack(int trackIdx);

    void removeDuplicate();

    // Shuffle
    bool isShuffled() const;
    void shuffle();
    void unshuffle();

    // Repeat
    RepeatMode getRepeatMode() const;
    void repeat();

    void clear();
private:
    std::optional<fs::path> m_path;
    bool m_isValid{false};
    std::string m_name;
    std::string m_description;
    TrackList m_tracks; // A track can be in different playlist, therefore they are included as shared pointers.
    TrackList m_shuffledPlaylist;
    TrackListIterator m_currentTrackIter;
    TrackPtr m_currentTrack;
    RepeatMode m_repeatMode{RepeatMode::NoRepeat};
    bool m_isShuffled{false};
};