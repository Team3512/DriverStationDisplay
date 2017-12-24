// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#pragma once

#include <stdint.h>

#include <string>
#include <variant>

/**
 * Provides a universal container for values being received over the network
 */
class NetEntry {
public:
    explicit NetEntry(uint8_t type = 0);

    NetEntry& operator=(const NetEntry& rhs);

    /* Delete the underlying storage for the value and reallocate it based on
     * the type ID passed in. this->type will be updated to the new type. If no
     * argument is given, The current value of this->type is used.
     */
    void setType(uint8_t type = 0);

    /**
     * Returns type ID of internal value storage
     */
    uint8_t getType() const;

    /**
     * Copies the memory pointed to by 'value' to internal storage
     */
    template <typename T>
    void setValue(const T& value);

    /**
     * Returns pointer to internal value storage
     */
    template <typename T>
    const T& getValue() const;

private:
    std::variant<int32_t, std::wstring> m_value;
    uint8_t m_type = 0;
};

#include "NetEntry.inl"
