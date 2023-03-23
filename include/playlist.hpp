#pragma once

#include <vector>
#include <string>
#include <optional>

#include "track.hpp"

class Playlist
{
public:

private:
    std::string m_name;
    std::optional<std::string> m_description;
    std::vector<Track> m_tracks;
    std::vector<int> m_playingOrder;
};