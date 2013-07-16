//=============================================================================
//File Name: RectangleShape.cpp
//Description: Provides a wrapper for WinGDI rectangles
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "RectangleShape.hpp"

#include <wingdi.h>

RectangleShape::RectangleShape( const Vector2i& position , const Vector2i& size , COLORREF fillColor , COLORREF outlineColor , int outlineThickness ) :
        Drawable( position , size , fillColor , outlineColor , outlineThickness ) {
}

void RectangleShape::draw( HDC hdc ) {
    // Set up the device context for drawing this shape
    HPEN oldPen = (HPEN)SelectObject( hdc , CreatePen( PS_SOLID , Drawable::getOutlineThickness() , Drawable::getOutlineColor() ) );
    HBRUSH oldBrush = (HBRUSH)SelectObject( hdc , CreateSolidBrush( Drawable::getFillColor() ) );

    // Draw rectangle
    Rectangle( hdc ,
            getBoundingRect().left - getOutlineThickness() ,
            getBoundingRect().top - getOutlineThickness() ,
            getBoundingRect().right + getOutlineThickness() ,
            getBoundingRect().bottom + getOutlineThickness()
            );

    // Replace old pen
    oldPen = (HPEN)SelectObject( hdc , oldPen );

    // Replace old brush
    oldBrush = (HBRUSH)SelectObject( hdc , oldBrush );

    // Free newly created objects
    DeleteObject( oldPen );
    DeleteObject( oldBrush );
}
