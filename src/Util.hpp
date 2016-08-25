// Copyright (c) FRC Team 3512, Spartatroniks 2012-2016. All Rights Reserved.

#ifndef UTIL_HPP
#define UTIL_HPP

// Contains miscellaneous utility functions

#include <string>
#include <vector>

template <class T>
bool packetToVar(const std::vector<char>& data, size_t& pos, T& dest);

bool packetToVar(const std::vector<char>& data, size_t& pos, std::string& dest);

#include "Util.inl"

#endif  // UTIL_HPP
