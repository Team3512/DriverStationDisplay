//=============================================================================
//File Name: Text.hpp
//Description: Provides a wrapper for WinGDI text
//Author: Tyler Veness
//=============================================================================

/* In this class's case, outlineColor is actually the background color of the
 * text
 */

#ifndef TEXT_HPP
#define TEXT_HPP

#include "Drawable.hpp"
#include <string>

class Text : public Drawable {
public:
    Text( const Vector& position , COLORREF fillColor , COLORREF outlineColor );

    void setString( std::wstring text );

    const std::wstring& getString();

    void draw( HDC hdc );

private:
    // The following functions don't do anything
    /* void setSize( const Vector& size );
     * void setSize( short width , short height );

     * const Vector& getSize();

     * void setOutlineThickness( int thickness );
     */

    std::wstring m_string;
};

#endif // TEXT_HPP
