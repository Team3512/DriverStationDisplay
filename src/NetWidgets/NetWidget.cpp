// Copyright (c) 2012-2018 FRC Team 3512. All Rights Reserved.

#include "NetWidget.hpp"

#include <algorithm>
#include <codecvt>
#include <cstring>
#include <locale>
#include <sstream>
#include <type_traits>
#include <utility>

#include "../Util.hpp"

std::vector<NetWidget*> NetWidget::m_netObjs;
std::map<std::string, NetWidget::NetEntry> NetWidget::m_netValues;

NetWidget::NetWidget(bool trackUpdate) : m_trackUpdate(trackUpdate) {
    if (m_trackUpdate) {
        m_netObjs.push_back(this);
    }
}

NetWidget::~NetWidget() {
    if (m_trackUpdate) {
        auto index = std::find(m_netObjs.begin(), m_netObjs.end(), this);

        if (index != m_netObjs.end()) {
            m_netObjs.erase(index);
        }
    }

    if (m_netObjs.size() == 0) {
        m_netValues.clear();
    }
}

void NetWidget::setUpdateText(const std::wstring& text) { m_updateText = text; }

const std::wstring& NetWidget::getUpdateText() { return m_updateText; }

void NetWidget::updateValues(std::vector<char>& data, size_t& pos) {
    uint8_t type;
    std::string key;

    while (pos < data.size() && packetToVar(data, pos, type) &&
           packetToVar(data, pos, key)) {
        auto& entry = m_netValues[key];

        // Assign value to prepared space
        if (type == 'c') {
            uint8_t value = 0;
            packetToVar(data, pos, value);

            entry = value;
        } else if (type == 'i') {
            int32_t value = 0;
            packetToVar(data, pos, value);

            entry = value;
        } else if (type == 's') {
            std::string value;
            packetToVar(data, pos, value);

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            entry = converter.from_bytes(value);
        }
    }
}

NetWidget::NetEntry& NetWidget::getEntry(const std::string& key) {
    // If there is a value for the given key, return it
    return m_netValues[key];
}

void NetWidget::updateElements() {
    for (auto i : m_netObjs) {
        i->updateEntry();
    }
}

void NetWidget::updateKeys(std::vector<std::string>& keys) { m_varIds = keys; }

std::wstring NetWidget::fillEntry(NetEntry& entry) {
    std::wstring replacement;

    std::visit(
        [&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int32_t>) {
                replacement = std::to_wstring(std::get<int32_t>(entry));
            } else {
                // Else data is already in a string
                replacement = std::get<std::wstring>(entry);
            }
        },
        entry);

    std::wstring buffer = m_updateText;
    size_t pos = 0;

    while ((pos = buffer.find(L"%s", pos)) != std::wstring::npos) {
        buffer.replace(pos, 2, replacement);
    }

    return buffer;
}
