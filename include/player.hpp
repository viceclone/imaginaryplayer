#pragma once

#include "playlist.hpp"

class Player
{
public:
    Player() = default;
    ~Player() = default;

    void play();
    void pause();
private:
    Playlist m_playlist;
};