//=============================================================================
//File Name: NetUpdate.cpp
//Description: Allows WinGDI drawable objects to update over the network
//Author: Tyler Veness
//=============================================================================

#include "NetUpdate.hpp"
#include <sstream>

#include <cstring>
#include <cwchar> // For _snwprintf(3, ...)

std::wstring replaceUnicodeChars( std::wstring text ) {
    size_t uPos = 0;

    /* Replace all "\uXXXX" strings with the unicode character corresponding
     * to the 32 bit code XXXX
     */
    while ( uPos < text.length() ) {
        if ( uPos == 0 ) {
            uPos = text.find( L"\\u" , uPos );
        }
        else {
            uPos = text.find( L"\\u" , uPos + 1 );
        }

        if ( uPos < text.length() - 5 ) {
            std::wstringstream ss;
            ss << std::hex << text.substr( uPos + 2 , 4 );
            int num;
            ss >> num;

            text = text.replace( uPos , 6 , numberToString( static_cast<wchar_t>(num) ) );
        }
    }

    return text;
}

std::list<NetUpdate*> NetUpdate::m_netObjs;
std::map<std::wstring , netValue_t*> NetUpdate::m_netValues;

NetUpdate::NetUpdate( bool trackUpdate ) :
m_trackUpdate( trackUpdate ) {
    if ( m_trackUpdate ) {
        m_netObjs.push_back( this );
    }
}

NetUpdate::~NetUpdate() {
    if ( m_trackUpdate ) {
        std::list<NetUpdate*>::iterator index;
        for ( index = m_netObjs.begin() ; *index != this ; index++ ) {
            if ( index == m_netObjs.end() ) {
                return;
            }
        }

        m_netObjs.erase( index );
    }

    if ( m_netObjs.size() == 0 ) {
        clearNetValues();
    }
}

void NetUpdate::setUpdateText( const std::wstring& text ) {
    m_updateText = replaceUnicodeChars( text );
}

const std::wstring& NetUpdate::getUpdateText() {
    return m_updateText;
}

void NetUpdate::updateValues( sf::Packet& packet ) {
    unsigned char type;
    std::wstring key;

    netValue_t* tempVal;
    std::wstring str;
    bool valAllocated = false;

    while ( !packet.endOfPacket() ) {
        if ( packet >> type && packet >> key ) {
            // If 'key' already has an entry
            if ( m_netValues.find( key ) != m_netValues.end() ) {
                /* If types aren't the same, free the value member for
                 * later reallocation
                 */
                if ( m_netValues[key]->type != type ) {
                    freeValue( m_netValues[key] );
                }
                else {
                    // Key exists and value is right size
                    valAllocated = true;
                }
            }
            // Else make a new one
            else {
                netValue_t* val = new netValue_t;
                val->type = '\0'; // Initialize type
                val->value = NULL;
                val->size = 0;
                m_netValues[key] = val;

                tempVal = val;
            }

            /* Now that there is guaranteed to be a valid netValue_t struct,
             * get a pointer to it
             */
            tempVal = m_netValues[key];

            /* Unlike the other types, the string must be retrieved
             * early so we know how much memory to allocate for it
             */
            if ( type == 's' ) {
                packet >> str;
            }

            // Allocate the right amount of space if needed
            if ( !valAllocated ) {
                switch ( type ) {
                case 'c': {
                    tempVal->type = 'c';
                    tempVal->value = new unsigned char;
                    tempVal->size = sizeof( unsigned char );

                    break;
                }
                case 'i': {
                    tempVal->type = 'i';
                    tempVal->value = new int;
                    tempVal->size = sizeof( int );

                    break;
                }
                case 'u': {
                    tempVal->type = 'u';
                    tempVal->value = new unsigned int;
                    tempVal->size = sizeof( unsigned int );

                    break;
                }
                case 's': {
                    tempVal->type = 's';

                    // 'str.length() + 1' provides room for the null terminator
                    tempVal->value = new wchar_t[ str.length() + 1 ];

                    // Zero memory before use
                    std::memset( tempVal->value , 0 , sizeof(wchar_t) * (str.length() + 1) );

                    tempVal->size = str.length();

                    break;
                }
                }
            }

            // Reallocate string storage if wrong size
            if ( type == 's' ) {
                if ( tempVal->size != str.length() ) {
                    freeValue( tempVal );

                    // 'str.length() + 1' provides room for the null terminator
                    tempVal->value = new wchar_t[ str.length() + 1 ];

                    // Zero memory before use
                    std::memset( tempVal->value , 0 , sizeof(wchar_t) * (str.length() + 1) );

                    tempVal->size = str.length();
                }
            }

            // Assign value to prepared space
            switch ( type ) {
            case 'c': {
                unsigned char value = 0;
                packet >> value;

                std::memcpy( tempVal->value , &value , tempVal->size );

                break;
            }
            case 'i': {
                int value = 0;
                packet >> value;

                std::memcpy( tempVal->value , &value , tempVal->size );

                break;
            }
            case 'u': {
                unsigned int value = 0;
                packet >> value;

                std::memcpy( tempVal->value , &value , tempVal->size );

                break;
            }
            case 's': {
                std::memcpy( tempVal->value , str.c_str() , sizeof(wchar_t) * tempVal->size );

                break;
            }
            }
        }
    }
}

void NetUpdate::freeValue( netValue_t* netVal ) {
    // Free value in void*
    if ( netVal->type == 'c' ) {
        delete static_cast<unsigned char*>(netVal->value);
    }
    else if ( netVal->type == 'i' ) {
        delete static_cast<int*>(netVal->value);
    }
    else if ( netVal->type == 'u' ) {
        delete static_cast<unsigned int*>(netVal->value);
    }
    else if ( netVal->type == 's' ) {
        delete [] static_cast<wchar_t*>(netVal->value);
    }
}

void NetUpdate::clearNetValues() {
    for ( std::map<std::wstring , netValue_t*>::iterator i = m_netValues.begin() ; i != m_netValues.end() ; i++ ) {
        freeValue( i->second );

        delete i->second; // Free netValue_t object
    }

    m_netValues.clear(); // Remove freed objects from std::map m_netValues
}

netValue_t* NetUpdate::getValue( const std::wstring& key ) {
    return m_netValues[key];
}

void NetUpdate::updateElements() {
    for ( std::list<NetUpdate*>::iterator i = m_netObjs.begin() ; i != m_netObjs.end() ; i++ ) {
        (*i)->updateValue();
    }
}

void NetUpdate::updateKeys( std::vector<std::wstring>& keys ) {
    m_varIds = keys;
}

void NetUpdate::fillValue( wchar_t* buffer , unsigned int size , netValue_t* value ) {
    std::wstringstream ss;

    if ( value->type == 'c' ) {
        unsigned char tempVal = 0;
        std::memcpy( &tempVal , value->value , value->size );
        ss << tempVal;

        _snwprintf( buffer , size , m_updateText.c_str() , ss.str().c_str() );
    }
    else if ( value->type == 'u' ) {
        unsigned int tempVal = 0;
        std::memcpy( &tempVal , value->value , value->size );
        ss << tempVal;

        _snwprintf( buffer , size , m_updateText.c_str() , ss.str().c_str() );
    }
    else if ( value->type == 'i' ) {
        int tempVal = 0;
        std::memcpy( &tempVal , value->value ,  value->size );
        ss << tempVal;

        _snwprintf( buffer , size , m_updateText.c_str() , ss.str().c_str() );
    }
    // Else data is already in a string
    else {
        _snwprintf( buffer , size , m_updateText.c_str() , value->value );
    }
}
