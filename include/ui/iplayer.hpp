#pragma once

#include "core/playlist.hpp"

// This class is intended to be pure virtual. It opens the future implementation on the UI design
class Player
{
public:
    Player() = default;
    ~Player() = default;

    virtual int importPlaylist() = 0;
    virtual void currentPlaylistInfo() = 0;
    virtual void currentTrackInfo() = 0;

    virtual void play() = 0;
    virtual void pause(bool autopause = false) = 0;

    // 
    /**
     * @param autoplay true if the player continues automatically to the next song, false otherwise
     * 
    */
    virtual bool next(bool autoplay = false) = 0;
    virtual bool previous() = 0;

    virtual void shuffle() = 0;
    
    // Switching repeat mode. Order: NoRepeat -> RepeatAll -> RepeatOne
    virtual void repeat() = 0;

    // Run the player
    virtual void init() = 0;
    virtual void run() = 0;
    virtual void terminate() = 0;
};