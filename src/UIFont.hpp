//=============================================================================
//File Name: UIFont.hpp
//Description: Provides a collection of fonts for use by other classes
//Author: Tyler Veness
//=============================================================================

#ifndef UIFONT_HPP
#define UIFONT_HPP

#include <SFML/Graphics/Font.hpp>

class UIFont {
public:
    // @return a global instance of the resources available
    static UIFont* getInstance();

    sf::Font& segoeUI();

protected:
    UIFont();

private:
    static UIFont* m_instance;

    static sf::Font m_segoeUI;

    // disallow copy and assignment
    UIFont ( const UIFont& );
    void operator=( const UIFont& );
};

#endif // UIFONT_HPP
