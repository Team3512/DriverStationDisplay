//=============================================================================
//File Name: Drawable.hpp
//Description: Provides interface for WinGDI drawable objects
//Author: Tyler Veness
//=============================================================================

#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Vector {
    template<class T , class U>
    Vector( T x , U y ) {
        X = x;
        Y = y;
    }

    short X;
    short Y;
};

class Drawable {
public:
    Drawable( const Vector& position , const Vector& size , COLORREF fillColor , COLORREF outlineColor , int outlineThickness );
    virtual ~Drawable();

    // Draws the drawable to the currently stored device context
    virtual void draw( HDC hdc ) = 0;

    virtual void setPosition( const Vector& position );
    virtual void setPosition( short x , short y );

    const Vector getPosition();

    virtual void setSize( const Vector& size );
    virtual void setSize( short width , short height );

    const Vector getSize();

    void setFillColor( COLORREF color );
    COLORREF getFillColor();

    void setOutlineColor( COLORREF color );
    COLORREF getOutlineColor();

    void setOutlineThickness( int thickness );
    int getOutlineThickness();

protected:
    const RECT& getBoundingRect();

private:
    RECT m_boundingRect;

    COLORREF m_fillColor;
    COLORREF m_outlineColor;
    int m_outlineThickness;
};

#endif // DRAWABLE_HPP
