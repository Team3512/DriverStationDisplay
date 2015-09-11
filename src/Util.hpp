// =============================================================================
// File Name: Util.hpp
// Description: Contains miscellaneous utility functions
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
#include <string>

template <class T>
bool packetToVar(const std::vector<char>& data, size_t& pos, T& dest);

bool packetToVar(const std::vector<char>& data, size_t& pos, std::string& dest);

#include "Util.inl"

#endif // UTIL_HPP

