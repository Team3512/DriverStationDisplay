// Copyright (c) 2012-2018 FRC Team 3512. All Rights Reserved.

#pragma once

#include <stdint.h>

#include <cstdlib>  // For std::memcpy(3)
#include <map>
#include <string>
#include <variant>
#include <vector>

/**
 * Allows drawable objects to update over the network
 *
 * Usage:
 *
 * When a packet full of display data comes in, each element of the packet is
 * applied to a NetWidget object in their respective storage orders (If the
 * packet is constructed wrong, an object may receive data intended for another
 * object.)
 */
class NetWidget {
public:
    using NetEntry = std::variant<int32_t, std::wstring>;

    /**
     * Passing 'true' adds the object to m_updateObjs, which makes it update
     * during a call to updateElements(). This may be desirable if the object is
     * a member of another class also deriving from the NetWidget class.
     */
    explicit NetWidget(bool trackUpdate);
    virtual ~NetWidget();

    /**
     * Sets string which determines how displayed text is updated
     */
    void setUpdateText(const std::wstring& text);

    /**
     * Returns template for value updates
     */
    const std::wstring& getUpdateText();

    /**
     * Updates values currently in table and adds new ones if they don't exist
     */
    static void updateValues(std::vector<char>& data, size_t& pos);

    /**
     * Returns the corresponding network value of a keyword
     */
    static NetEntry& getEntry(const std::string& key);

    /**
     * All elements' values are updated from the table of network values
     */
    static void updateElements();

    /**
     * Updates keys which are used to retrieve the network variables
     */
    virtual void updateKeys(std::vector<std::string>& keys);

    /**
     * Insert value into update text and return the result
     */
    std::wstring fillEntry(NetEntry& entry);

    /**
     * Updates custom values of object to display
     */
    virtual void updateEntry() = 0;

protected:
    std::vector<std::string> m_varIds;

private:
    static std::vector<NetWidget*> m_netObjs;
    static std::map<std::string, NetEntry> m_netValues;

    std::wstring m_updateText;

    const bool m_trackUpdate;
};
