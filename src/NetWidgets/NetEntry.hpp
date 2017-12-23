// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#pragma once

#include <stdint.h>

#include <string>

/**
 * Provides a universal container for values being received over the network
 */
class NetEntry {
public:
    explicit NetEntry(uint8_t type = 0);
    virtual ~NetEntry();

    NetEntry(const NetEntry&) = delete;
    NetEntry& operator=(const NetEntry& rhs);

    // Returns type ID of internal value storage
    uint8_t getType() const;

    /* Delete the underlying storage for the value and reallocate it based on
     * the type ID passed in. this->type will be updated to the new type. If no
     * argument is given, The current value of this->type is used.
     */
    void reallocValue(uint8_t type = 0);

    // Copies the memory pointed to by 'value' to internal storage
    void setValue(void* value);

    // Returns pointer to internal value storage
    void* getValue() const;

private:
    void allocValue();
    void freeValue();

    uint8_t m_type = 0;
    void* m_value;
};
