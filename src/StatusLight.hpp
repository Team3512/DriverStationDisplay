//=============================================================================
//File Name: StatusLight.hpp
//Description: Shows green, yellow, or red circle depending on its internal
//             state
//Author: Tyler Veness
//=============================================================================

// TODO Add color-blind mode

#ifndef STATUS_LIGHT_HPP
#define STATUS_LIGHT_HPP

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>

class StatusLight : public sf::CircleShape {
public:
    enum Status {
        active,
        standby,
        inactive
    };

    StatusLight( const sf::Vector2f& position , std::string message , Status currentStatus = StatusLight::inactive );

    void setActive( Status newStatus );
    Status getActive();

    void setPosition( const sf::Vector2f& position );
    void setPosition( float x , float y );

    void setString( const std::string& message );
    const sf::String& getString();

private:
    Status m_status;

    sf::Text m_sfText;

    void draw( sf::RenderTarget& target , sf::RenderStates states = sf::RenderStates::Default ) const;
};

#endif // STATUS_LIGHT_HPP
