//=============================================================================
//File Name: ProgressBar.cpp
//Description: Provides an interface to a progress bar with WinGDI
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "ProgressBar.hpp"
#include "UIFont.hpp"

#include <wingdi.h>

ProgressBar::ProgressBar( const Vector2i& position , std::wstring text , COLORREF fullFillColor , COLORREF emptyFillColor , COLORREF outlineColor , bool netUpdate ) :
        RectangleShape( position , Vector2i( 100 , 18 ) , emptyFillColor , outlineColor , 1 ) ,
        NetUpdate( netUpdate ) ,
        m_barFill( Vector2i( position.X + 1 , position.Y + 1 ) , Vector2i( 98 , 16 ) , fullFillColor , RGB( 0 , 70 , 0 ) , 0 ) ,
        m_text( Vector2i( position.X , position.Y + 18 + 2 ) , UIFont::getInstance().segoeUI14() , text.c_str() , RGB( 0 , 0 , 0 ) , RGB( GetRValue(GetSysColor(COLOR_3DFACE)) , GetGValue(GetSysColor(COLOR_3DFACE)) , GetBValue(GetSysColor(COLOR_3DFACE)) ) , false ) {

    setPercent( 0.f );
}

void ProgressBar::draw( HDC hdc ) {
    RectangleShape::draw( hdc );
    m_barFill.draw( hdc );
    m_text.draw( hdc );
}

void ProgressBar::setPercent( float percentFull ) {
    if ( percentFull > 100 ) {
        percentFull = 100;
    }

    percent = percentFull;
    m_barFill.setSize( Vector2i( ( Drawable::getSize().X - 2.f ) * percentFull / 100.f , m_barFill.getSize().Y ) );
}

float ProgressBar::getPercent() {
    return percent;
}

void ProgressBar::setPosition( const Vector2i& position ) {
    RectangleShape::setPosition( position );
    m_barFill.setPosition( position.X + 1 , position.Y + 1 );

    m_text.setPosition( RectangleShape::getPosition().X , RectangleShape::getPosition().Y + RectangleShape::getSize().Y + 2 );
}

void ProgressBar::setPosition( short x , short y ) {
    Drawable::setPosition( x , y );
    m_barFill.setPosition( x + 1 , y + 1 );

    m_text.setPosition( RectangleShape::getPosition().X , RectangleShape::getPosition().Y + RectangleShape::getSize().Y + 2 );
}

void ProgressBar::setSize( const Vector2i& size ) {
    RectangleShape::setSize( size );
    m_barFill.setSize( Vector2i( ( size.X - 2 ) * percent , size.Y - 2 ) );

    m_text.setPosition( RectangleShape::getPosition().X , RectangleShape::getPosition().Y + RectangleShape::getSize().Y + 2 );
}

void ProgressBar::setSize( short width , short height ) {
    RectangleShape::setSize( Vector2i( width , height ) );
    m_barFill.setSize( Vector2i( ( width - 2 ) * percent , height - 2 ) );

    m_text.setPosition( RectangleShape::getPosition().X , RectangleShape::getPosition().Y + RectangleShape::getSize().Y + 2 );
}

void ProgressBar::setString( const std::wstring& message ) {
    m_text.setString( message );
}

const std::wstring& ProgressBar::getString() {
    return m_text.getString();
}

void ProgressBar::setBarFillColor( COLORREF fill ) {
    m_barFill.setFillColor( fill );
}

COLORREF ProgressBar::getBarFillColor() {
    return m_barFill.getFillColor();
}

void ProgressBar::updateValue() {
    netValue_t* printValue = getValue( m_varIds[0] );
    netValue_t* percentValue = getValue( m_varIds[1] );

    if ( printValue != NULL ) {
        // TODO Not secure
        wchar_t temp[128];

        NetUpdate::fillValue( temp , 128 , printValue );

        setString( temp );
    }

    if ( percentValue != NULL ) {
        setPercent( *static_cast<unsigned char*>(percentValue->value) );
    }
}
