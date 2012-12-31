//=============================================================================
//File Name: RectangleShape.hpp
//Description: Provides a wrapper for WinGDI rectangles
//Author: Tyler Veness
//=============================================================================

#ifndef RECTANGLE_SHAPE_HPP
#define RECTANGLE_SHAPE_HPP

#include "Drawable.hpp"

class RectangleShape : public Drawable {
public:
    RectangleShape( const Vector& position , const Vector& size , COLORREF fillColor , COLORREF outlineColor , int outlineThickness );

    void draw( HDC hdc );
};

#endif // RECTANGLE_SHAPE_HPP
