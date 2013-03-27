//=============================================================================
//File Name: NetUpdate.hpp
//Description: Allows WinGDI drawable objects to update over the network
//Author: Tyler Veness
//=============================================================================

#ifndef NET_UPDATE_HPP
#define NET_UPDATE_HPP

#include <string>
#include <list>
#include <sstream>
#include <map>

#include <cstdlib> // For std::memcpy(3)

#include "../SFML/Network/Packet.hpp"

/* Usage:
 *
 * When a packet full of display data comes in, each element of the packet is
 * applied to a NetUpdate object in their respective storage orders (If the
 * packet is constructed wrong, an object may receive data intended for another
 * object.)
 */

template <typename T>
std::wstring numberToString( T number ) {
    std::wstringstream ss;
    std::wstring str;

    ss << number;
    str = ss.str();

    return str;
}

template <typename T>
T stringToNumber( std::wstring str ) {
    std::wstringstream ss;
    T num;

    ss << str;
    ss >> num;

    return num;
}

std::wstring replaceUnicodeChars( std::wstring text );

typedef struct netValue_t {
    unsigned char type;
    void* value;
    size_t size;
} netValue_t;

class NetUpdate {
public:
    /* Passing 'true' adds the object to m_updateObjs, which makes it update
     * during a call to updateElements(). This may be desirable if the object is a
     * member of another class also deriving from the NetUpdate class.
     */
    NetUpdate( bool trackUpdate );
    virtual ~NetUpdate();

    // Sets string which determines how displayed text is updated
    void setUpdateText( const std::wstring& text );

    // Returns template for value updates
    const std::wstring& getUpdateText();

    // Updates values currently in table and adds new ones if they don't exist
    static void updateValues( sf::Packet& packet );

    // Frees value storage for 'value' member of netValue_t
    static void freeValue( netValue_t* netVal );

    // Clears all values from table
    static void clearNetValues();

    // Returns the corresponding network value of a keyword
    static netValue_t* getValue( const std::wstring& key );

    // All elements' values are updated from the table of network values
    static void updateElements();

    // Updates keys which are used to retrieve the network variables
    void updateKeys( std::vector<std::wstring>& keys );

    /* Insert value into update text and output it to a buffer
     * 'size' is how many bytes wide 'buffer' is
     */
    void fillValue( wchar_t* buffer , unsigned int size , netValue_t* value );

    // Updates custom values of object to display
    virtual void updateValue() = 0;

protected:
    std::vector<std::wstring> m_varIds;

private:
    static std::list<NetUpdate*> m_netObjs;
    static std::map<std::wstring , netValue_t*> m_netValues;

    std::wstring m_updateText;

    const bool m_trackUpdate;
};

#endif // NET_UPDATE_HPP
