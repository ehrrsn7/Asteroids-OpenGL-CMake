#pragma once

#include "window.hpp"

class Rock
{
public:
   Rock() = default;
   ~Rock() = default;
};

class SmallRock : public Rock
{
public:
   SmallRock() = default;
   ~SmallRock() = default;
};

class DefaultRock : public Rock
{
public:
   DefaultRock() = default;
   ~DefaultRock() = default;
};

class LargeRock : public Rock
{
public:
   LargeRock() = default;
   ~LargeRock() = default;
};
