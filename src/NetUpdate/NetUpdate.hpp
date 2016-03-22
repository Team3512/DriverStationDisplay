// =============================================================================
// File Name: NetUpdate.hpp
// Description: Allows WinGDI drawable objects to update over the network
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef NET_UPDATE_HPP
#define NET_UPDATE_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>

#include <cstdlib> // For std::memcpy(3)

#include "NetValue.hpp"

/* Usage:
 *
 * When a packet full of display data comes in, each element of the packet is
 * applied to a NetUpdate object in their respective storage orders (If the
 * packet is constructed wrong, an object may receive data intended for another
 * object.)
 */

std::wstring replaceUnicodeChars(std::wstring text);

class NetUpdate {
public:
    /* Passing 'true' adds the object to m_updateObjs, which makes it update
     * during a call to updateElements(). This may be desirable if the object is a
     * member of another class also deriving from the NetUpdate class.
     */
    NetUpdate(bool trackUpdate);
    virtual ~NetUpdate();

    // Sets string which determines how displayed text is updated
    void setUpdateText(const std::wstring& text);

    // Returns template for value updates
    const std::wstring& getUpdateText();

    // Updates values currently in table and adds new ones if they don't exist
    static void updateValues(std::vector<char>& data, size_t& pos);

    // Returns the corresponding network value of a keyword
    static NetValue* getValue(const std::string& key);

    // All elements' values are updated from the table of network values
    static void updateElements();

    // Updates keys which are used to retrieve the network variables
    virtual void updateKeys(std::vector<std::string>& keys);

    // Insert value into update text and return the result
    std::wstring fillValue(NetValue* value);

    // Updates custom values of object to display
    virtual void updateValue() = 0;

protected:
    std::vector<std::string> m_varIds;

private:
    static std::vector<NetUpdate*> m_netObjs;
    static std::map<std::string, NetValue> m_netValues;

    std::wstring m_updateText;

    const bool m_trackUpdate;
};

#endif // NET_UPDATE_HPP
