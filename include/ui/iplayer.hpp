#pragma once

#include "core/playlist.hpp"

class Player
{
public:
    Player() = default;
    ~Player() = default;

    virtual void streamCurrentSong() = 0;

    virtual void play() = 0;
    virtual void pause() = 0;

    virtual void shuffle() = 0;
    virtual void unshuffle() = 0;
    
    virtual void repeatAll() = 0;
    virtual void repeatOne() = 0;
private:
    Playlist m_playlist;
};