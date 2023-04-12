#pragma once

#include <filesystem>
#include <optional>
#include <random>

#include "track.hpp"

namespace helper
{
    std::optional<Track> getTrackFromFile(std::filesystem::path&& path);

    // Generate a random integer in the range [x, y]
    int randomInt(int x, int y);
}