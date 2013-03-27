//=============================================================================
//File Name: StatusLight.cpp
//Description: Shows green, yellow, or red circle depending on its internal
//             state
//Author: Tyler Veness
//=============================================================================

#include "StatusLight.hpp"
#include "UIFont.hpp"

#include <cstring>
#include <wingdi.h>

StatusLight::StatusLight( const Vector2i& position , std::wstring message , bool netUpdate ) :
        Drawable( position , Vector2i( 0 , 0 ) , 0 , 0 , 0 ) ,
        NetUpdate( netUpdate ) ,
        m_status( StatusLight::inactive ) ,
        m_text( position , UIFont::getInstance()->segoeUI14() , message , RGB( 255 , 255 , 255 ) , RGB( 87 , 87 , 87 ) , false ) {
    setOutlineThickness( 2 );
    setOutlineColor( RGB( 50 , 50 , 50 ) );
    setPosition( position );
    setSize( 24 , 24 );

    setActive( m_status );
}

void StatusLight::draw( HDC hdc ) {
    // Set up the device context for drawing this shape
    HPEN oldPen = (HPEN)SelectObject( hdc , CreatePen( PS_SOLID , Drawable::getOutlineThickness() , Drawable::getOutlineColor() ) );
    HBRUSH oldBrush = (HBRUSH)SelectObject( hdc , CreateSolidBrush( Drawable::getFillColor() ) );

    // Draw circle with the points centered at its origin
    Ellipse( hdc , getBoundingRect().left , getBoundingRect().top , getBoundingRect().right , getBoundingRect().bottom );

    // Restore old brush and free the created one
    oldBrush = (HBRUSH)SelectObject( hdc , oldBrush );
    DeleteObject( oldBrush );

    // Restore old pen and free the created one
    oldPen = (HPEN)SelectObject( hdc , oldPen );
    DeleteObject( oldPen );

    m_text.draw( hdc );
}

void StatusLight::setActive( Status newStatus ) {
    if ( newStatus == StatusLight::active ) {
        setFillColor( RGB( 0 , 0 , 120 ) );
    }
    else if ( newStatus == StatusLight::standby ) {
        setFillColor( RGB( 128 , 128 , 0 ) );
    }
    else {
        setFillColor( RGB( 128 , 0 , 0 ) );
    }

    m_status = newStatus;
}

StatusLight::Status StatusLight::getActive() {
    return m_status;
}

void StatusLight::setPosition( const Vector2i& position ) {
    // Set position of circle
    Drawable::setPosition( Vector2i( position.X - 6 , position.Y - 6 ) );

    // Set position of text in relation to the circle
    m_text.setPosition( Vector2i( position.X + 12 /* diameter */ + 10 ,
            position.Y - 3 ) );
}

void StatusLight::setPosition( short x , short y ) {
    // Set position of circle
    Drawable::setPosition( x - 6 , y - 6 );

    // Set position of text in relation to the circle
    m_text.setPosition( Vector2i( x + 12 /* diameter */ + 10 ,
            y - 3 ) );
}

void StatusLight::setString( const std::wstring& message ) {
    m_text.setString( message );
}

const std::wstring& StatusLight::getString() {
    return m_text.getString();
}

void StatusLight::updateValue() {
    netValue_t* lightValue = getValue( m_varIds[0] );

    unsigned char tempVal = 0;
    std::memcpy( &tempVal , lightValue->value , sizeof(unsigned char) );

    setActive( Status( tempVal ) );

    setString( getUpdateText() );
}
