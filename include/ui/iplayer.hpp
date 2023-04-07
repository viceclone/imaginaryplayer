#pragma once

#include "core/playlist.hpp"

// This class is intended to be pure virtual. It opens the future implementation on the UI design
class Player
{
public:
    Player() = default;
    ~Player() = default;

    virtual int importPlaylist() = 0;

    virtual void play() = 0;
    virtual void pause() = 0;

    virtual bool next() = 0;
    virtual bool previous() = 0;

    virtual void shuffle() = 0;
    virtual void unshuffle() = 0;
    
    virtual void repeatAll() = 0;
    virtual void repeatOne() = 0;

    // Run the player
    virtual void init() = 0;
    virtual void run() = 0;
    virtual void terminate() = 0;
};