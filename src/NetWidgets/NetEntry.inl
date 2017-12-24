// Copyright (c) 2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#pragma once

template <typename T>
void NetEntry::setValue(const T& value) {
    m_value = value;
}

template <typename T>
const T& NetEntry::getValue() const {
    return std::get<T>(m_value);
}
