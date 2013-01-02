//=============================================================================
//File Name: ProgressBar.cpp
//Description: Holds definitions for ProgressBar class
//Author: Tyler Veness
//=============================================================================

#include "ProgressBar.hpp"
#include "UIFont.hpp"

#include <wingdi.h>

ProgressBar::ProgressBar( const Vector& position , std::wstring message , COLORREF fullFillColor , COLORREF emptyFillColor , COLORREF outlineColor , float percentFull ) :
        RectangleShape( position , Vector( 100 , 18 ) , emptyFillColor , outlineColor , 1 ) ,
        m_barFill( Vector( position.X + 1 , position.Y + 1 ) , Vector( 98 , 16 ) , fullFillColor , RGB( 0 , 70 , 0 ) , 0 ) ,
        m_text( Vector( position.X , position.Y + 18 + 2 ) , RGB( 255 , 255 , 255 ) , RGB( 87 , 87 , 87 ) ) {

    setPercent( percentFull );

    m_text.setString( message );
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
    m_barFill.setSize( Vector( ( Drawable::getSize().X - 2.f ) * percentFull / 100.f , m_barFill.getSize().Y ) );
}

float ProgressBar::getPercent() {
    return percent;
}

void ProgressBar::setPosition( const Vector& position ) {
    RectangleShape::setPosition( position );
    m_barFill.setPosition( position.X + 1 , position.Y + 1 );

    m_text.setPosition( RectangleShape::getPosition().X , RectangleShape::getPosition().Y + RectangleShape::getSize().Y + 2 );
}

void ProgressBar::setPosition( short x , short y ) {
    Drawable::setPosition( x , y );
    m_barFill.setPosition( x + 1 , y + 1 );

    m_text.setPosition( RectangleShape::getPosition().X , RectangleShape::getPosition().Y + RectangleShape::getSize().Y + 2 );
}

void ProgressBar::setSize( const Vector& size ) {
    RectangleShape::setSize( size );
    m_barFill.setSize( Vector( ( size.X - 2 ) * percent , size.Y - 2 ) );

    m_text.setPosition( RectangleShape::getPosition().X , RectangleShape::getPosition().Y + RectangleShape::getSize().Y + 2 );
}

void ProgressBar::setSize( short width , short height ) {
    RectangleShape::setSize( Vector( width , height ) );
    m_barFill.setSize( Vector( ( width - 2 ) * percent , height - 2 ) );

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
