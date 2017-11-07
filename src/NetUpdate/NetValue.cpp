// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "NetValue.hpp"

#include <cstring>
#include <string>

NetValue::NetValue(unsigned char type) {
    m_type = type;

    allocValue();
}

NetValue::~NetValue() { freeValue(); }

NetValue& NetValue::operator=(const NetValue& rhs) {
    reallocValue(rhs.m_type);
    setValue(rhs.m_value);

    return *this;
}

unsigned char NetValue::getType() const { return m_type; }

void NetValue::reallocValue(unsigned char type) {
    freeValue();

    if (m_type != type) {
        m_type = type;
    }

    allocValue();
}

void NetValue::setValue(void* value) {
    if (m_type == 'c') {
        std::memcpy(m_value, value, sizeof(unsigned char));
    } else if (m_type == 'i') {
        std::memcpy(m_value, value, sizeof(int));
    } else if (m_type == 'u') {
        std::memcpy(m_value, value, sizeof(unsigned int));
    } else if (m_type == 's') {
        std::string* str = static_cast<std::string*>(value);

        // Convert std::string to std::wstring
        static_cast<std::wstring*>(m_value)->assign(str->begin(), str->end());
    }
}

void* NetValue::getValue() const { return m_value; }

void NetValue::allocValue() {
    if (m_type == 'c') {
        m_value = new unsigned char;
    } else if (m_type == 'i') {
        m_value = new int;
    } else if (m_type == 'u') {
        m_value = new unsigned int;
    } else if (m_type == 's') {
        m_value = new std::wstring;
    } else {
        m_type = 0;
        m_value = nullptr;
    }
}

void NetValue::freeValue() {
    // Free value in void*
    if (m_type == 'c') {
        delete static_cast<unsigned char*>(m_value);
    } else if (m_type == 'i') {
        delete static_cast<int*>(m_value);
    } else if (m_type == 'u') {
        delete static_cast<unsigned int*>(m_value);
    } else if (m_type == 's') {
        delete static_cast<std::wstring*>(m_value);
    }

    m_type = 0;
}
