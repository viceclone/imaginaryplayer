#pragma once

#include <condition_variable>
#include <mutex>

#include "iplayer.hpp"

class TextBasedPlayer : public Player
{
public:
    inline static const std::string playlistFileName{"playlist.txt"};

    TextBasedPlayer() = default;
    ~TextBasedPlayer();

    // Return the number of valid track imported
    int importPlaylist() override;
    void currentPlaylistInfo() override;
    void play() override;
    void pause(bool autopause = false) override;

    bool next(bool autoplay = false) override;
    bool previous() override;

    void shuffle() override;
    
    void repeat() override;

    void init() override;
    void run() override;
    void terminate() override;
private:
    void printHelp();
    void startCommandHandler();
    void streamCurrentSong();
    std::shared_ptr<Playlist> m_playlist;
    bool m_isPlaying{false};
    bool m_isRunning{false};

    std::shared_ptr<Track> m_currentTrack;
    bool m_trackAvailable{true};

    std::thread m_streamingThread;
    std::condition_variable m_cv;
    std::mutex m_mutex;
};