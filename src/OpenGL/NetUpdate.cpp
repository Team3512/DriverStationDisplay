// =============================================================================
// File Name: NetUpdate.cpp
// Description: Allows WinGDI drawable objects to update over the network
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "NetUpdate.hpp"
#include <sstream>
#include <algorithm>

#include <cstring>
#include <cwchar> // For _snwprintf(3, ...)

std::wstring replaceUnicodeChars(std::wstring text) {
    size_t uPos = 0;

    /* Replace all "\uXXXX" strings with the unicode character corresponding
     * to the 32 bit code XXXX
     */
    while (uPos < text.length()) {
        if (uPos == 0) {
            uPos = text.find(L"\\u", uPos);
        }
        else {
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

NetUpdate::NetUpdate(bool trackUpdate) :
    m_trackUpdate(trackUpdate) {
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

void NetUpdate::setUpdateText(const std::wstring& text) {
    m_updateText = replaceUnicodeChars(text);
}

const std::wstring& NetUpdate::getUpdateText() {
    return m_updateText;
}

void NetUpdate::updateValues(sf::Packet& packet) {
    unsigned char type;
    std::string key;

    NetValue* tempVal;

    while (!packet.endOfPacket() && packet >> type && packet >> key) {
        // If 'key' already has an entry
        if (m_netValues.find(key) != m_netValues.end()) {
            tempVal = &m_netValues[key];

            /* If types aren't the same, free the value member for later
             * reallocation
             */
            if (tempVal->getType() != type) {
                tempVal->reallocValue(type);
            }
        }
        // Else make a new one
        else {
            m_netValues[key] = std::move(NetValue(type));

            tempVal = &m_netValues[key];
        }

        // Assign value to prepared space
        if (type == 'c') {
            unsigned char value = 0;
            packet >> value;

            tempVal->setValue(&value);
        }
        else if (type == 'i') {
            int value = 0;
            packet >> value;

            tempVal->setValue(&value);
        }
        else if (type == 'u') {
            unsigned int value = 0;
            packet >> value;

            tempVal->setValue(&value);
        }
        else if (type == 's') {
            std::string value;
            packet >> value;

            tempVal->setValue(&value);
        }
    }
}

NetValue* NetUpdate::getValue(const std::string& key) {
    // If there is a value for the given key, return it
    if (m_netValues.find(key) != m_netValues.end()) {
        return &m_netValues[key];
    }
    else {
        return nullptr;
    }
}

void NetUpdate::updateElements() {
    for (auto i : m_netObjs) {
        i->updateValue();
    }
}

void NetUpdate::updateKeys(std::vector<std::string>& keys) {
    m_varIds = keys;
}

void NetUpdate::fillValue(wchar_t* buffer, unsigned int size, NetValue* value) {
    unsigned char type = value->getType();

    if (type == 'c') {
        unsigned char tempVal = 0;
        std::memcpy(&tempVal, value->getValue(), sizeof(tempVal));

        swprintf(buffer, size, m_updateText.c_str(), std::to_wstring(
                     tempVal).c_str());
    }
    else if (type == 'u') {
        unsigned int tempVal = 0;
        std::memcpy(&tempVal, value->getValue(), sizeof(tempVal));

        swprintf(buffer, size, m_updateText.c_str(), std::to_wstring(
                     tempVal).c_str());
    }
    else if (type == 'i') {
        int tempVal = 0;
        std::memcpy(&tempVal, value->getValue(), sizeof(tempVal));

        swprintf(buffer, size, m_updateText.c_str(), std::to_wstring(
                     tempVal).c_str());
    }
    // Else data is already in a string
    else {
        swprintf(buffer, size, m_updateText.c_str(), value->getValue());
    }
}

