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
std::map<std::string, NetValue*> NetUpdate::m_netValues;

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
        clearNetValues();
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
    std::string str;
    bool valAllocated = false;

    bool haveValidData = true;

    while (!packet.endOfPacket() && haveValidData) {
        if (packet >> type && packet >> key) {
            // If 'key' already has an entry
            if (m_netValues.find(key) != m_netValues.end()) {
                /* If types aren't the same, free the value member for
                 * later reallocation
                 */
                if (m_netValues[key]->type != type) {
                    freeValue(m_netValues[key]);
                    valAllocated = false;
                }
                else {
                    // Key exists and value is right size
                    valAllocated = true;
                }
            }
            // Else make a new one
            else {
                NetValue* val = new NetValue;
                val->type = '\0'; // Initialize type
                val->value = nullptr;
                m_netValues[key] = val;

                tempVal = val;
            }

            /* Now that there is guaranteed to be a valid NetValue struct,
             * get a pointer to it
             */
            tempVal = m_netValues[key];

            /* Unlike the other types, the string must be retrieved
             * early so we know how much memory to allocate for it
             */
            if (type == 's') {
                packet >> str;
            }

            // Allocate the right amount of space if needed
            if (!valAllocated) {
                switch (type) {
                case 'c': {
                    tempVal->type = 'c';
                    tempVal->value = new unsigned char;

                    break;
                }
                case 'i': {
                    tempVal->type = 'i';
                    tempVal->value = new int;

                    break;
                }
                case 'u': {
                    tempVal->type = 'u';
                    tempVal->value = new unsigned int;

                    break;
                }
                case 's': {
                    tempVal->type = 's';

                    // 'str.length() + 1' provides room for the null terminator
                    tempVal->value = new std::wstring;

                    break;
                }
                default: {
                    abort();
                }
                }
            }

            // Assign value to prepared space
            switch (type) {
            case 'c': {
                unsigned char value = 0;
                packet >> value;

                std::memcpy(tempVal->value, &value, sizeof(value));

                break;
            }
            case 'i': {
                int value = 0;
                packet >> value;

                std::memcpy(tempVal->value, &value, sizeof(value));

                break;
            }
            case 'u': {
                unsigned int value = 0;
                packet >> value;

                std::memcpy(tempVal->value, &value, sizeof(value));

                break;
            }
            case 's': {
                // Convert std::string to std::wstring
                static_cast<std::wstring*>(tempVal->value)->assign(
                    str.begin(), str.end());

                break;
            }
            }
        }
        else {
            haveValidData = false;
        }
    }
}

void NetUpdate::freeValue(NetValue* netVal) {
    // Free value in void*
    if (netVal->type == 'c') {
        delete static_cast<unsigned char*>(netVal->value);
    }
    else if (netVal->type == 'i') {
        delete static_cast<int*>(netVal->value);
    }
    else if (netVal->type == 'u') {
        delete static_cast<unsigned int*>(netVal->value);
    }
    else if (netVal->type == 's') {
        delete[] static_cast<wchar_t*>(netVal->value);
    }
}

void NetUpdate::clearNetValues() {
    for (auto i : m_netValues) {
        freeValue(i.second);

        delete i.second; // Free NetValue object
    }

    m_netValues.clear(); // Remove freed objects from std::map m_netValues
}

NetValue* NetUpdate::getValue(const std::string& key) {
    // If there is a value for the given key, return it
    if (m_netValues.find(key) != m_netValues.end()) {
        return m_netValues[key];
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
    if (value->type == 'c') {
        unsigned char tempVal = 0;
        std::memcpy(&tempVal, value->value, sizeof(tempVal));

        _snwprintf(buffer, size, m_updateText.c_str(), std::to_string(
                       tempVal).c_str());
    }
    else if (value->type == 'u') {
        unsigned int tempVal = 0;
        std::memcpy(&tempVal, value->value, sizeof(tempVal));

        _snwprintf(buffer, size, m_updateText.c_str(), std::to_string(
                       tempVal).c_str());
    }
    else if (value->type == 'i') {
        int tempVal = 0;
        std::memcpy(&tempVal, value->value, sizeof(tempVal));

        _snwprintf(buffer, size, m_updateText.c_str(), std::to_string(
                       tempVal).c_str());
    }
    // Else data is already in a string
    else {
        _snwprintf(buffer, size, m_updateText.c_str(), value->value);
    }
}

