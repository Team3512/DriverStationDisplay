//=============================================================================
//File Name: RectangleShape.cpp
//Description: Provides a wrapper for WinGDI rectangles
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "RectangleShape.hpp"

#include <wingdi.h>

RectangleShape::RectangleShape( const Vector2i& position , const Vector2i& size , Colorf fillColor , Colorf outlineColor , int outlineThickness ) :
        Drawable( position , size , fillColor , outlineColor , outlineThickness ) {
}

void RectangleShape::draw( HDC hdc ) {
    if ( hdc == NULL ) {
        // Draw rectangle
        glBegin( GL_TRIANGLES );
        glColor4f( Drawable::getFillColor().glR() , Drawable::getFillColor().glG() , Drawable::getFillColor().glB() , Drawable::getFillColor().glA() );
        glVertex3f( getBoundingRect().left , getBoundingRect().bottom , 0 );
        glVertex3f( getBoundingRect().right , getBoundingRect().top , 0 );
        glVertex3f( getBoundingRect().left , getBoundingRect().top , 0 );

        glVertex3f( getBoundingRect().left , getBoundingRect().bottom , 0 );
        glVertex3f( getBoundingRect().right , getBoundingRect().bottom , 0 );
        glVertex3f( getBoundingRect().right , getBoundingRect().top , 0 );
        glEnd();

        // Draw border
        glBegin( GL_LINES );
        glColor4f( Drawable::getOutlineColor().glR() , Drawable::getOutlineColor().glG() , Drawable::getOutlineColor().glB() , Drawable::getOutlineColor().glA() );
        glVertex3f( getBoundingRect().left - getOutlineThickness() , getBoundingRect().bottom + getOutlineThickness() , 0 );
        glVertex3f( getBoundingRect().right + getOutlineThickness() , getBoundingRect().bottom + getOutlineThickness() , 0 );

        glVertex3f( getBoundingRect().right + getOutlineThickness() , getBoundingRect().bottom + getOutlineThickness() , 0 );
        glVertex3f( getBoundingRect().right + getOutlineThickness() , getBoundingRect().top - getOutlineThickness() , 0 );

        glVertex3f( getBoundingRect().right + getOutlineThickness() , getBoundingRect().top - getOutlineThickness() , 0 );
        glVertex3f( getBoundingRect().left - getOutlineThickness() , getBoundingRect().top - getOutlineThickness() , 0 );

        glVertex3f( getBoundingRect().left - getOutlineThickness() , getBoundingRect().top - getOutlineThickness() , 0 );
        glVertex3f( getBoundingRect().left - getOutlineThickness() , getBoundingRect().bottom + getOutlineThickness() , 0 );
        glEnd();
    }
}
