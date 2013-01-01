//=============================================================================
//File Name: ProgressBar.hpp
//Description: Holds declarations for ProgressBar class
//Author: Tyler Veness
//=============================================================================

#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#include "RectangleShape.hpp"
#include "Text.hpp"
#include <string>

class ProgressBar : public RectangleShape {
public:
    ProgressBar( const Vector& position , std::wstring message , COLORREF fullFillColor , COLORREF emptyFillColor , COLORREF outlineColor , float percentFull = 0.f );

    void setPercent( float percentFull );
    float getPercent();

    void setPosition( const Vector& position );
    void setPosition( short x , short y );

    void setSize( const Vector& size );
    void setSize( short width , short height );

    void setString( const std::wstring& message );
    const std::wstring& getString();

    void setBarFillColor( COLORREF fill );
    COLORREF getBarFillColor();

    void draw( HDC hdc );

private:
    RectangleShape m_barFill;

    float percent;

    Text m_text;
};

#endif // PROGRESSBAR_HPP
