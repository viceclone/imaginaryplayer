#include "core/helper.hpp"

namespace helper
{
int randomInt(int x, int y)
{
    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_int_distribution<> distrib(x, y);
    return distrib(rng);
}
}