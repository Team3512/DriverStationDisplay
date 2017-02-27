// Copyright (c) FRC Team 3512, Spartatroniks 2012-2017. All Rights Reserved.

#include "NetUpdate.hpp"

#include <algorithm>
#include <cstring>
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

std::vector<NetUpdate*> NetUpdate::m_netObjs;
std::map<std::string, NetValue> NetUpdate::m_netValues;

NetUpdate::NetUpdate(bool trackUpdate) : m_trackUpdate(trackUpdate) {
    if (m_trackUpdate) {
        m_netObjs.push_back(this);
    }
}

NetUpdate::~NetUpdate() {
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

void NetUpdate::setUpdateText(const std::wstring& text) { m_updateText = text; }

const std::wstring& NetUpdate::getUpdateText() { return m_updateText; }

void NetUpdate::updateValues(std::vector<char>& data, size_t& pos) {
    unsigned char type;
    std::string key;

    NetValue* tempVal;

    while (pos < data.size() && packetToVar(data, pos, type) &&
           packetToVar(data, pos, key)) {
        // If 'key' already has an entry
        if (m_netValues.find(key) != m_netValues.end()) {
            tempVal = &m_netValues[key];

            /* If types aren't the same, free the value member for later
             * reallocation
             */
            if (tempVal->getType() != type) {
                tempVal->reallocValue(type);
            }
        } else {
            // Else make a new one
            m_netValues[key] = std::move(NetValue(type));

            tempVal = &m_netValues[key];
        }

        // Assign value to prepared space
        if (type == 'c') {
            unsigned char value = 0;
            packetToVar(data, pos, value);

            tempVal->setValue(&value);
        } else if (type == 'i') {
            int value = 0;
            packetToVar(data, pos, value);

            tempVal->setValue(&value);
        } else if (type == 'u') {
            unsigned int value = 0;
            packetToVar(data, pos, value);

            tempVal->setValue(&value);
        } else if (type == 's') {
            std::string value;
            packetToVar(data, pos, value);

            tempVal->setValue(&value);
        }
    }
}

NetValue* NetUpdate::getValue(const std::string& key) {
    // If there is a value for the given key, return it
    if (m_netValues.find(key) != m_netValues.end()) {
        return &m_netValues[key];
    } else {
        return nullptr;
    }
}

void NetUpdate::updateElements() {
    for (auto i : m_netObjs) {
        i->updateValue();
    }
}

void NetUpdate::updateKeys(std::vector<std::string>& keys) { m_varIds = keys; }

std::wstring NetUpdate::fillValue(NetValue* value) {
    unsigned char type = value->getType();
    std::wstring replacement;

    if (type == 'c') {
        unsigned char tempVal = 0;
        std::memcpy(&tempVal, value->getValue(), sizeof(tempVal));
        replacement = std::to_wstring(tempVal);
    } else if (type == 'u') {
        unsigned int tempVal = 0;
        std::memcpy(&tempVal, value->getValue(), sizeof(tempVal));
        replacement = std::to_wstring(tempVal);
    } else if (type == 'i') {
        int tempVal = 0;
        std::memcpy(&tempVal, value->getValue(), sizeof(tempVal));
        replacement = std::to_wstring(tempVal);
    } else {
        // Else data is already in a string
        replacement = *static_cast<std::wstring*>(value->getValue());
    }

    std::wstring buffer = m_updateText;
    size_t pos = 0;

    while ((pos = buffer.find(L"%s", pos)) != std::wstring::npos) {
        buffer.replace(pos, 2, replacement);
    }

    return buffer;
}
