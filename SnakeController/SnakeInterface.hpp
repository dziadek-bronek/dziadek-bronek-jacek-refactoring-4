#pragma once
#include "Directions.hpp"

#include <cstdint>

namespace Snake
{

struct DirectionInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x10;

    Direction direction;
};


struct TimeoutInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x20;
};

enum Cell
{
    Cell_FREE,
    Cell_FOOD,
    Cell_SNAKE
};

struct DisplayInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x30;

    int x;
    int y;
    Cell value;
};

struct FoodInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x40;

    int x;
    int y;
};

struct FoodReq
{
    static constexpr std::uint32_t MESSAGE_ID = 0x41;
};

struct FoodResp
{
    static constexpr std::uint32_t MESSAGE_ID = 0x42;

    int x;
    int y;
};

struct ScoreInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x70;
};

struct LooseInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x71;
};

struct PauseInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x91;
};

} // namespace Snake
