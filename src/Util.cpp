// Copyright (c) 2012-2018 FRC Team 3512. All Rights Reserved.

#include "Util.hpp"

bool packetToVar(const std::vector<char>& data, size_t& pos,
                 std::string& dest) {
    if (pos + sizeof(uint32_t) <= data.size()) {
        uint32_t size = qFromBigEndian<uint32_t>(
            reinterpret_cast<const uint8_t*>(&data[pos]));
        pos += sizeof(size);

        if (pos + size <= data.size()) {
            dest.assign(&data[pos], size);
            pos += size;

            return true;
        }
    }

    /* Return false if either size of string or string data couldn't be
     * extracted
     */
    return false;
}
