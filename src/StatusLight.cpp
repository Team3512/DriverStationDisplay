//=============================================================================
//File Name: StatusLight.cpp
//Description: Shows green, yellow, or red circle depending on its internal
//             state
//Author: Tyler Veness
//=============================================================================

#include "StatusLight.hpp"
#include "UIFont.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

StatusLight::StatusLight( const sf::Vector2f& position , std::string message , Status currentStatus  ) :
        sf::CircleShape( 10 ) ,
        m_status( currentStatus ) ,
        m_sfText( message , UIFont::getInstance()->segoeUI() , 14 ) {
    setOrigin( 5.f , 5.f );
    setOutlineThickness( 2.f );
    setOutlineColor( sf::Color( 50 , 50 , 50 ) );

    m_sfText.setColor( sf::Color( 255 , 255 , 255 ) );

    setPosition( position );

    setActive( currentStatus );
}

void StatusLight::draw( sf::RenderTarget& target , sf::RenderStates states ) const {
    target.draw( static_cast<sf::CircleShape>(*this) );
    target.draw( m_sfText );
}

void StatusLight::setActive( Status newStatus ) {
    if ( newStatus == StatusLight::active ) {
        setFillColor( sf::Color( 0 , 120 , 0 ) );
    }
    else if ( newStatus == StatusLight::standby ) {
        setFillColor( sf::Color( 128 , 128 , 0 ) );
    }
    else {
        setFillColor( sf::Color( 128 , 0 , 0 ) );
    }

    m_status = newStatus;
}

StatusLight::Status StatusLight::getActive() {
    return m_status;
}

void StatusLight::setPosition( const sf::Vector2f& position ) {
    CircleShape::setPosition( position );

    m_sfText.setPosition( CircleShape::getPosition().x + CircleShape::getRadius() + 11.f , CircleShape::getPosition().y - 3.f );
}

void StatusLight::setPosition( float x , float y ) {
    CircleShape::setPosition( x , y );

    m_sfText.setPosition( CircleShape::getPosition().x + CircleShape::getRadius() + 11.f , CircleShape::getPosition().y - 3.f );
}

void StatusLight::setString( const std::string& message ) {
    m_sfText.setString( message );
}

const sf::String& StatusLight::getString() {
    return m_sfText.getString();
}
