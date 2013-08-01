//=============================================================================
//File Name: DisplaySettings.cpp
//Description: Parses a stream of bytes into a list of UI elements to create on
//             the DriverStationDisplay
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "DisplaySettings.hpp"

#include "WinGDI/ProgressBar.hpp"
#include "WinGDI/StatusLight.hpp"
#include "WinGDI/Text.hpp"
#include "WinGDI/UIFont.hpp"

#include <fstream>
#include <cstdio>

DisplaySettings::DisplaySettings( std::string fileName , int leftX , int leftY , int rightX , int rightY ) :
    m_lStartX( leftX ) ,
    m_lStartY( leftY ) ,
    m_rStartX( rightX ) ,
    m_rStartY( rightY ) {
        if ( fileName != std::string( "" ) ) {
            reloadGUI( fileName );
        }
}

DisplaySettings::~DisplaySettings() {
    clearGUI();
}

void DisplaySettings::reloadGUI( sf::Packet& packet ) {
    std::wstring *elementLine;
    unsigned int filesize;
    wchar_t *tmpbuf;
    unsigned char tmpbyte;
    unsigned int i;
    wchar_t *curLine;

    // Remove old elements before creating new ones
    clearGUI();

    resetAllTemps();

    // Read the file out of the packet
    packet >> filesize;

    // Read the file into a wchar_t buffer
    tmpbuf = new wchar_t[filesize + 1];
    for(i = 0; i < filesize; i++) {
        packet >> tmpbyte;
        tmpbuf[i] = std::btowc(tmpbyte);
    }
    tmpbuf[i] = L'\0';

    // Send individual lines for processing
    curLine = std::wcstok( tmpbuf, L"\r\n" );
    while( curLine != NULL ) {
        elementLine = new std::wstring(curLine);
        if( elementLine->length() > 0 ) {
            parseLine( *elementLine );
        }
        delete elementLine;

        curLine = std::wcstok(NULL, L"\r\n");
    }

    // Free the file buffer
    delete[] tmpbuf;
}

void DisplaySettings::reloadGUI( const std::string& fileName ) {
    std::string tempName = fileName;
    std::wifstream guiSettings( tempName );

    // Remove old elements before creating new ones
    clearGUI();

    if ( guiSettings.is_open() ) {
        resetAllTemps();

        while ( !guiSettings.eof() ) {
            std::getline( guiSettings , m_line );

            parseLine( m_line );
        }

        guiSettings.close();
    }
}

void DisplaySettings::clearGUI() {
    for ( std::list<Drawable*>::iterator i = m_drawables.begin() ; i != m_drawables.end() ; i++ ) {
        delete *i;
    }

    m_drawables.clear();
}

void DisplaySettings::updateGuiTable( sf::Packet& packet ) {
    NetUpdate::updateValues( packet );
}

void DisplaySettings::drawDisplay( HDC hdc ) {
    for ( std::list<Drawable*>::iterator i = m_drawables.begin() ; i != m_drawables.end() ; i++ ) {
        (*i)->draw( hdc );
    }
}

void DisplaySettings::parseLine( std::wstring line ) {
    m_line = line;
    m_tempVarIds.clear();

    /* If the line doesn't have any characters in it, don't bother
     * parsing it
     */
    if ( m_line.length() == 0 ) {
        return;
    }

    m_start = 0;
    m_length = 0;

    m_delimiter = L" ";

    // Get first three arguments
    for ( size_t i = 0 ; i < 3 ; i++ ) {
        // Find next argument
        while ( m_line.substr( m_start , 1 ) == m_delimiter ) {
            if ( m_start < m_line.length() - 1 ) {
                m_start++;
            }
        }

        // Find end of next argument
        while ( m_line.substr( m_start + m_length , 1 ) != m_delimiter ) {
            if ( m_start < m_line.length() - 1 ) {
                m_length++;
            }
        }

        // Get current argument
        if ( i == 0 ) {
            // lastType vars are updated when the column is found
            m_currentType = m_line.substr( m_start , m_length );
        }
        else if ( i == 1 ) {
            m_substring = m_line.substr( m_start , m_length );

            unsigned int start = 0;
            unsigned int length = 0;
            std::wstring delimiter = L",";

            // While there are still characters to parse
            while ( start < m_substring.size() - 1 ) {
                // Find next argument
                while ( m_substring.substr( start , 1 ) == delimiter && start < m_substring.length() - 1 ) {
                    start++;
                }

                // Find end of next argument
                while ( m_substring.substr( start + length , 1 ) != delimiter && start + length < m_substring.length() - 1 ) {
                    length++;
                }

                // Make sure the last character of the end of the string is included in a key
                if ( start + length == m_substring.length() - 1 ) {
                    length++;
                }

                /* Add the next variable ID to a storage vector for adding to
                 * the element later
                 */
                m_tempVarIds.push_back( m_substring.substr( start , length ) );

                start += length;
                length = 0;
            }
        }
        else if ( i == 2 ) {
            m_column = m_line.substr( m_start , m_length );

            // Store the element IDs in the correct columns
            if ( m_column == std::wstring( L"left" ) ) {
                // Store previous "current type" before updating it
                m_lLastType = m_lCurrentType;
                m_lCurrentType = m_currentType;
            }
            else if ( m_column == std::wstring( L"right" ) ) {
                // Store previous "current type" before updating it
                m_rLastType = m_rCurrentType;
                m_rCurrentType = m_currentType;
            }
        }

        // Move start past current argument
        m_start += m_length;
        m_length = 0;
    }

    m_delimiter = L"\"";

    // Get last two arguments in quotes
    for ( size_t i = 2 ; i < 4 ; i++ ) {
        // Find delimiter at start of next argument
        while ( m_line.substr( m_start , 1 ) != m_delimiter ) {
            if ( m_start < m_line.length() - 1 ) {
                m_start++;
            }
        }

        // Advance past delimiter
        if ( m_start < m_line.length() - 1 ) {
            m_start++;
        }

        // Find end of next argument
        while ( m_line.substr( m_start + m_length , 1 ) != m_delimiter ) {
            m_length++;
        }

        // Get current argument
        if ( i == 2 ) {
            m_startText = m_line.substr( m_start , m_length );
        }
        else if ( i == 3 ) {
            m_updateText = m_line.substr( m_start , m_length );
        }

        // Move start past current argument
        m_start += m_length;
        m_length = 0;

        // Move past delimiter
        if ( m_start < m_line.length() - 1 ) {
            m_start++;
        }
    }

    /* Set appropriate X and Y coordinates for new element and use
     * correct column's previous element ID
     */
    if ( m_column == std::wstring( L"left" ) ) {
        m_tempX = m_lStartX;
        m_tempY = m_lTempY;
        m_lastType = m_lLastType;
    }
    else if ( m_column == std::wstring( L"right" ) ) {
        m_tempX = m_rStartX;
        m_tempY = m_rTempY;
        m_lastType = m_rLastType;
    }

    // Increase Y for new element
    if ( m_lastType == std::wstring( L"TEXT" ) && m_currentType == std::wstring( L"TEXT" ) ) {
        m_tempY += 33;
    }
    else if ( m_lastType == std::wstring( L"TEXT" ) && m_currentType == std::wstring( L"STATUSLIGHT" ) ) {
        m_tempY += 44;
    }
    else if ( m_lastType == std::wstring( L"STATUSLIGHT" ) && m_currentType == std::wstring( L"STATUSLIGHT" ) ) {
        m_tempY += 40;
    }
    else if ( m_lastType == std::wstring( L"PBAR" ) && m_currentType == std::wstring( L"PBAR" ) ) {
        m_tempY += 56;
    }
    else if ( m_lastType == std::wstring( L"PBAR" ) && m_currentType == std::wstring( L"STATUSLIGHT" ) ) {
        m_tempY += 61;
    }
    else if ( m_lastType != L"" ) {
        /* If the element doesn't match a name, like BLANK, the Y will
         * increment without creating an element
         */
        m_tempY += 49;
    }

    // Replace all unicode escape characters in the string with their unicode equivalents
    m_startText = replaceUnicodeChars( m_startText );

    Drawable* elemPtr = NULL;
    NetUpdate* netPtr = NULL;

    // Create element
    if ( m_currentType == std::wstring( L"TEXT" ) ) {
        Text* temp =  new Text( Vector2i( m_tempX , m_tempY ) , UIFont::getInstance().segoeUI14() , m_startText , RGB( 0 , 0 , 0 ) , RGB( 236 , 233 , 216 ) , true );
        elemPtr = temp;
        netPtr = temp;
    }
    else if ( m_currentType == std::wstring( L"STATUSLIGHT" ) ) {
        StatusLight* temp = new StatusLight( Vector2i( m_tempX  , m_tempY ) , m_startText , true );
        elemPtr = temp;
        netPtr = temp;
    }
    else if ( m_currentType == std::wstring( L"PBAR" ) ) {
        ProgressBar* temp = new ProgressBar( Vector2i( m_tempX , m_tempY ) , m_startText , RGB( 0 , 120 , 0 ) , RGB( 255 , 255 , 255 ) , RGB( 50 , 50 , 50 ) , true );
        elemPtr = temp;
        netPtr = temp;
    }

    /* Set update text and add the update variables to track if an element was
     * created
     */
    if ( netPtr != NULL ) {
        netPtr->setUpdateText( m_updateText );
        netPtr->updateKeys( m_tempVarIds );
    }

    // Add element to gDrawables if it was created
    if ( elemPtr != NULL ) {
        m_drawables.push_back( elemPtr );
    }

    // Update Y position for future elements
    if ( m_column == std::wstring( L"left" ) ) {
        m_lTempY = m_tempY;
    }
    else if ( m_column == std::wstring( L"right" ) ) {
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
