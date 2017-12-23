// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "NetEntry.hpp"

#include <cstring>
#include <string>

NetEntry::NetEntry(uint8_t type) {
    m_type = type;

    allocValue();
}

NetEntry::~NetEntry() { freeValue(); }

NetEntry& NetEntry::operator=(const NetEntry& rhs) {
    if (m_type != rhs.m_type) {
        reallocValue(rhs.m_type);
    }
    setValue(rhs.m_value);

    return *this;
}

uint8_t NetEntry::getType() const { return m_type; }

void NetEntry::reallocValue(uint8_t type) {
    freeValue();
    m_type = type;
    allocValue();
}

void NetEntry::setValue(void* value) {
    if (m_type == 'c') {
        std::memcpy(m_value, value, sizeof(uint8_t));
    } else if (m_type == 'i') {
        std::memcpy(m_value, value, sizeof(int32_t));
    } else if (m_type == 's') {
        std::string* str = static_cast<std::string*>(value);

        // Convert std::string to std::wstring
        static_cast<std::wstring*>(m_value)->assign(str->begin(), str->end());
    }
}

void* NetEntry::getValue() const { return m_value; }

void NetEntry::allocValue() {
    if (m_type == 'c') {
        m_value = new uint8_t;
    } else if (m_type == 'i') {
        m_value = new int32_t;
    } else if (m_type == 's') {
        m_value = new std::wstring;
    } else {
        m_type = 0;
        m_value = nullptr;
    }
}

void NetEntry::freeValue() {
    // Free value in void*
    if (m_type == 'c') {
        delete static_cast<uint8_t*>(m_value);
    } else if (m_type == 'i') {
        delete static_cast<int32_t*>(m_value);
    } else if (m_type == 's') {
        delete static_cast<std::wstring*>(m_value);
    }

    m_type = 0;
}
