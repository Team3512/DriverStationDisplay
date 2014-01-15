//=============================================================================
//File Name: RectangleShape.hpp
//Description: Provides a wrapper for WinGDI rectangles
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#ifndef RECTANGLE_SHAPE_HPP
#define RECTANGLE_SHAPE_HPP

#include "Drawable.hpp"
#include "Color.hpp"

#include <GL/gl.h>

class RectangleShape : public Drawable {
public:
    RectangleShape( const Vector2i& position , const Vector2i& size , Colorf fillColor , Colorf outlineColor , int outlineThickness );

    void draw( HDC hdc );
};

#endif // RECTANGLE_SHAPE_HPP
