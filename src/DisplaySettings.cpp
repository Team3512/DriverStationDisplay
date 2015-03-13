// =============================================================================
// File Name: DisplaySettings.cpp
// Description: Parses a stream of bytes into a list of UI elements to create on
//             the DriverStationDisplay
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "DisplaySettings.hpp"

#include "OpenGL/ProgressBar.hpp"
#include "OpenGL/StatusLight.hpp"
#include "WinGDI/Text.hpp"
#include "WinGDI/UIFont.hpp"

#include <fstream>

DisplaySettings::DisplaySettings(std::string fileName,
                                 int leftX,
                                 int leftY,
                                 int rightX,
                                 int rightY) :
    m_lStartX(leftX),
    m_lStartY(leftY),
    m_rStartX(rightX),
    m_rStartY(rightY) {
    if (fileName != "") {
        reloadGUI(fileName);
    }
}

DisplaySettings::~DisplaySettings() {
    clearGUI();
}

void DisplaySettings::reloadGUI(sf::Packet& packet) {
    // Remove old elements before creating new ones
    clearGUI();

    resetAllTemps();

    // Read the file into a string
    std::string tmpbuf;
    packet >> tmpbuf;

    std::vector<std::string> lines = split(tmpbuf, "\r\n");
    for (auto& str : lines) {
        parseLine(str);
    }
}

void DisplaySettings::reloadGUI(const std::string& fileName) {
    std::ifstream guiSettings(fileName);

    // Remove old elements before creating new ones
    clearGUI();

    if (guiSettings.is_open()) {
        resetAllTemps();

        while (!guiSettings.eof()) {
            std::getline(guiSettings, m_line);
            parseLine(m_line);
        }
    }
}

void DisplaySettings::clearGUI() {
    m_drawables.clear();
}

void DisplaySettings::updateGuiTable(sf::Packet& packet) {
    NetUpdate::updateValues(packet);
}

void DisplaySettings::drawDisplay(HDC hdc) {
    for (auto& i : m_drawables) {
        i->draw(hdc);
    }
}

std::vector<std::string> DisplaySettings::split(const std::string& s,
                                                const std::string& delim) {
    std::vector<std::string> elems;

    unsigned int pos = 0;
    unsigned int nextPos = 0;
    while (nextPos != std::string::npos) {
        nextPos = s.find_first_of(delim, pos);
        elems.emplace_back(s.substr(pos, nextPos - pos));
        pos = nextPos + delim.length();
    }

    return elems;
}

void DisplaySettings::parseLine(std::string line) {
    m_line = line;
    m_tempVarIds.clear();

    /* If the line doesn't have any characters in it, don't bother
     * parsing it
     */
    if (m_line.length() == 0) {
        return;
    }

    m_start = 0;
    m_length = 0;

    m_delimiter = " ";

    // Get first three arguments
    for (size_t i = 0; i < 3; i++) {
        // Find next argument
        while (m_line.substr(m_start, 1) == m_delimiter) {
            if (m_start < m_line.length() - 1) {
                m_start++;
            }
        }

        // Find end of next argument
        while (m_line.substr(m_start + m_length, m_delimiter.length()) !=
               m_delimiter) {
            if (m_start < m_line.length() - 1) {
                m_length++;
            }
        }

        // Get current argument
        if (i == 0) {
            // lastType vars are updated when the column is found
            m_currentType = m_line.substr(m_start, m_length);
        }
        else if (i == 1) {
            m_substring = m_line.substr(m_start, m_length);

            unsigned int start = 0;
            unsigned int length = 0;
            std::string delimiter = ",";

            // While there are still characters to parse
            while (start < m_substring.size() - 1) {
                // Find next argument
                start = m_substring.find_first_not_of(delimiter, start);

                // Find end of next argument
                length = m_substring.find_first_of(delimiter, length);

                // Make sure the last character of the end of the string is included in a key
                if (start + length == m_substring.length() - 1) {
                    length++;
                }

                /* Add the next variable ID to a storage vector for adding to
                 * the element later
                 */
                m_tempVarIds.push_back(m_substring.substr(start, length));

                start += length;
                length = 0;
            }
        }
        else if (i == 2) {
            m_column = m_line.substr(m_start, m_length);

            // Store the element IDs in the correct columns
            if (m_column == "left") {
                // Store previous "current type" before updating it
                m_lLastType = m_lCurrentType;
                m_lCurrentType = m_currentType;
            }
            else if (m_column == "right") {
                // Store previous "current type" before updating it
                m_rLastType = m_rCurrentType;
                m_rCurrentType = m_currentType;
            }
        }

        // Move start past current argument
        m_start += m_length;
        m_length = 0;
    }

    m_delimiter = "\"";

    // Get last two arguments in quotes
    for (size_t i = 2; i < 4; i++) {
        // Find delimiter at start of next argument
        m_start = m_line.find_first_of(m_delimiter, m_start);

        // Advance past delimiter
        if (m_start < m_line.length() - 1) {
            m_start++;
        }

        // Find end of next argument
        m_length = m_line.find_first_of(m_delimiter, m_start + m_length);

        // Get current argument
        if (i == 2) {
            std::string tempStr = m_line.substr(m_start, m_length);

            // Convert std::string to std::wstring
            m_startText.assign(tempStr.begin(), tempStr.end());
        }
        else if (i == 3) {
            std::string tempStr = m_line.substr(m_start, m_length);

            // Convert std::string to std::wstring
            m_updateText.assign(tempStr.begin(), tempStr.end());
        }

        // Move start past current argument
        m_start += m_length;
        m_length = 0;

        // Move past delimiter
        if (m_start < m_line.length() - 1) {
            m_start++;
        }
    }

    /* Set appropriate X and Y coordinates for new element and use
     * correct column's previous element ID
     */
    if (m_column == "left") {
        m_tempX = m_lStartX;
        m_tempY = m_lTempY;
        m_lastType = m_lLastType;
    }
    else if (m_column == "right") {
        m_tempX = m_rStartX;
        m_tempY = m_rTempY;
        m_lastType = m_rLastType;
    }

    // Increase Y for new element
    if (m_lastType == "TEXT" && m_currentType == "TEXT") {
        m_tempY += 33;
    }
    else if (m_lastType == "TEXT" && m_currentType == "STATUSLIGHT") {
        m_tempY += 44;
    }
    else if (m_lastType == "STATUSLIGHT" && m_currentType == "STATUSLIGHT") {
        m_tempY += 40;
    }
    else if (m_lastType == "PBAR" && m_currentType == "PBAR") {
        m_tempY += 56;
    }
    else if (m_lastType == "PBAR" && m_currentType == "STATUSLIGHT") {
        m_tempY += 61;
    }
    else if (m_lastType != "") {
        /* If the element doesn't match a name, like BLANK, the Y will
         * increment without creating an element
         */
        m_tempY += 49;
    }

    // Replace all unicode escape characters in the string with their unicode equivalents
    m_startText = replaceUnicodeChars(m_startText);

    Drawable* elemPtr = nullptr;
    NetUpdate* netPtr = nullptr;

    // Create element
    if (m_currentType == "TEXT") {
        Text* temp =  new Text(Vector2i(m_tempX, m_tempY),
                               UIFont::getInstance().segoeUI14(), m_startText,
                               Colorf(0,
                                      0,
                                      0),
                               Colorf(GetRValue(GetSysColor(COLOR_3DFACE)),
                                      GetGValue(GetSysColor(COLOR_3DFACE)),
                                      GetBValue(GetSysColor(
                                                    COLOR_3DFACE))), true);
        elemPtr = temp;
        netPtr = temp;
    }
    else if (m_currentType == "STATUSLIGHT") {
        StatusLight* temp = new StatusLight(Vector2i(m_tempX,
                                                     m_tempY), m_startText,
                                            true);
        elemPtr = temp;
        netPtr = temp;
    }
    else if (m_currentType == "PBAR") {
        ProgressBar* temp = new ProgressBar(Vector2i(m_tempX, m_tempY),
                                            m_startText,
                                            Colorf(0, 120, 0),
                                            Colorf(255, 255, 255),
                                            Colorf(50, 50, 50), true);
        elemPtr = temp;
        netPtr = temp;
    }

    /* Set update text and add the update variables to track if an element was
     * created
     */
    if (netPtr != nullptr) {
        netPtr->setUpdateText(m_updateText);
        netPtr->updateKeys(m_tempVarIds);
    }

    // Add element to gDrawables if it was created
    if (elemPtr != nullptr) {
        m_drawables.push_back(std::unique_ptr<Drawable>(elemPtr));
    }

    // Update Y position for future elements
    if (m_column == "left") {
        m_lTempY = m_tempY;
    }
    else if (m_column == "right") {
        m_rTempY = m_tempY;
    }
}

void DisplaySettings::resetAllTemps() {
    m_line.clear();
    m_substring.clear();
    m_start = 0;
    m_length = 0;

    m_delimiter.clear();

    m_lastType.clear();
    m_currentType.clear();
    m_column.clear();
    m_startText.clear();
    m_updateText.clear();
    m_tempVarIds.clear();

    /* ===== Column-specific vars ===== */
    m_lLastType.clear();
    m_lCurrentType.clear();

    m_rLastType.clear();
    m_rCurrentType.clear();

    m_tempX = 0;
    m_tempY = 0;

    m_lTempY = m_lStartY;
    m_rTempY = m_rStartY;
    /* ================================ */
}

