//=============================================================================
//File Name: DisplaySettings.hpp
//Description: Parses a stream of bytes into a list of UI elements to create on
//             the DriverStationDisplay
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#ifndef DISPLAY_SETTINGS_HPP
#define DISPLAY_SETTINGS_HPP

#include <string>
#include <list>
#include "SFML/Network/Packet.hpp"

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Drawable;

class DisplaySettings {
public:
    DisplaySettings( std::string fileName , int leftX , int leftY , int rightX , int rightY );
    virtual ~DisplaySettings();

    // Updates list of elements from packet and recreates them
    void reloadGUI( sf::Packet& packet );

    // Updates list of elements from file and recreates them
    void reloadGUI( const std::string& fileName );

    // Removes all elements from list
    void clearGUI();

    // Updates values of elements from packet
    void updateGuiTable( sf::Packet& packet );

    // Draw all drawables in the internal list to the given device context
    void drawDisplay( HDC hdc );

private:
    std::list<Drawable*> m_drawables;

    std::wstring m_line;
    std::wstring m_substring;
    size_t m_start;
    size_t m_length;

    std::wstring m_delimiter;

    std::wstring m_lastType;
    std::wstring m_currentType;
    std::wstring m_column;
    std::wstring m_startText;
    std::wstring m_updateText;
    std::vector<std::wstring> m_tempVarIds;

    /* ===== Column-specific vars ===== */
    std::wstring m_lLastType;
    std::wstring m_lCurrentType;

    std::wstring m_rLastType;
    std::wstring m_rCurrentType;

    int m_tempX;
    int m_tempY;

    const int m_lStartX;
    const int m_lStartY;
    int m_lTempY;

    const int m_rStartX;
    const int m_rStartY;
    int m_rTempY;
    /* ================================ */

    /* Parses a given settings line, adding an element if applicable and
     * incrementing the global position variables for the column to which
     * the element was added
     */
    void parseLine( std::wstring line );

    // Resets all temporary variables to be reused during a call to update(1)
    void resetAllTemps();
};

#endif // DISPLAY_SETTINGS_HPP
