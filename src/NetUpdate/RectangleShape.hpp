// =============================================================================
// File Name: RectangleShape.hpp
// Description: Provides a wrapper for WinGDI rectangles
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef RECTANGLE_SHAPE_HPP
#define RECTANGLE_SHAPE_HPP

#include "Drawable.hpp"

class RectangleShape : public Drawable {
public:
    RectangleShape(const QPoint& size,
                   QColor fillColor,
                   QColor outlineColor,
                   int outlineThickness,
                   const QPoint& position = QPoint());

    void paintEvent(QPaintEvent* event);
};

#endif // RECTANGLE_SHAPE_HPP

