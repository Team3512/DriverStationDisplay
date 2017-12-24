// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "NetWidget.hpp"

#include <algorithm>
#include <codecvt>
#include <cstring>
#include <locale>
#include <sstream>
#include <utility>

#include "../Util.hpp"

std::wstring replaceUnicodeChars(std::wstring text) {
    size_t uPos = 0;

    /* Replace all "\uXXXX" strings with the unicode character corresponding
     * to the 32 bit code XXXX
     */
    while (uPos < text.length()) {
        if (uPos == 0) {
            uPos = text.find(L"\\u", uPos);
        } else {
            uPos = text.find(L"\\u", uPos + 1);
        }

        if (uPos < text.length() - 5) {
            std::wstringstream ss;
            ss << std::hex << text.substr(uPos + 2, 4);
            wchar_t num;
            ss >> num;

            text = text.replace(uPos, 6, &num);
        }
    }

    return text;
}

std::vector<NetWidget*> NetWidget::m_netObjs;
std::map<std::string, NetEntry> NetWidget::m_netValues;

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
        entry.setType(type);

        // Assign value to prepared space
        if (type == 'c') {
            uint8_t value = 0;
            packetToVar(data, pos, value);

            entry.setValue(value);
        } else if (type == 'i') {
            int32_t value = 0;
            packetToVar(data, pos, value);

            entry.setValue(value);
        } else if (type == 's') {
            std::string value;
            packetToVar(data, pos, value);

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            entry.setValue(converter.from_bytes(value));
        }
    }
}

NetEntry& NetWidget::getNetEntry(const std::string& key) {
    // If there is a value for the given key, return it
    return m_netValues[key];
}

void NetWidget::updateElements() {
    for (auto i : m_netObjs) {
        i->updateEntry();
    }
}

void NetWidget::updateKeys(std::vector<std::string>& keys) { m_varIds = keys; }

std::wstring NetWidget::fill(NetEntry& entry) {
    uint8_t type = entry.getType();
    std::wstring replacement;

    if (type == 'c' || type == 'i') {
        replacement = std::to_wstring(entry.getValue<int32_t>());
    } else {
        // Else data is already in a string
        replacement = entry.getValue<std::wstring>();
    }

    std::wstring buffer = m_updateText;
    size_t pos = 0;

    while ((pos = buffer.find(L"%s", pos)) != std::wstring::npos) {
        buffer.replace(pos, 2, replacement);
    }

    return buffer;
}
