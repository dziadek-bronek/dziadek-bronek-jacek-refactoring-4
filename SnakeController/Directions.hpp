#pragma once

namespace Snake {

enum Direction
{
    Direction_UP    = 0b00,
    Direction_DOWN  = 0b10,
    Direction_LEFT  = 0b01,
    Direction_RIGHT = 0b11
};

bool isHorizontal(Direction direction);
bool isVertical(Direction direction);
bool isPositive(Direction direction);
bool perpendicular(Direction dir1, Direction dir2);
} // namespace
