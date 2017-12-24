// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "NetEntry.hpp"

NetEntry::NetEntry(uint8_t type) {
    m_type = type;
}

void NetEntry::setType(uint8_t type) {
    m_type = type;
}

uint8_t NetEntry::getType() const { return m_type; }
