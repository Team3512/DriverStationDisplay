//=============================================================================
//File Name: Text.cpp
//Description: Provides a wrapper for WinGDI text
//Author: Tyler Veness
//=============================================================================

#include "Text.hpp"
#include "UIFont.hpp"
#include <wingdi.h>

Text::Text( const Vector& position , COLORREF fillColor , COLORREF outlineColor ) :
        Drawable( position , Vector( 0 , 0 ) , fillColor , outlineColor , 0 ) {
}

void Text::setString( std::wstring text ) {
    m_string = text;
}

const std::wstring& Text::getString() {
    return m_string;
}

void Text::draw( HDC hdc ) {
    // Select font
    HFONT oldFont = (HFONT)SelectObject( hdc , UIFont::getInstance()->segoeUI14() );

    // Set text color
    COLORREF oldColor = SetTextColor( hdc , Drawable::getFillColor() );

    // Set text background color
    COLORREF oldBackColor = SetBkColor( hdc , Drawable::getOutlineColor() );

    TextOutW( hdc , getPosition().X , getPosition().Y , m_string.c_str() , static_cast<int>(m_string.length()) );

    // Restore old text color
    SetTextColor( hdc , oldColor );

    // Restore old text background color
    SetBkColor( hdc , oldBackColor );

    // Restore old font
    SelectObject( hdc , oldFont );
}
