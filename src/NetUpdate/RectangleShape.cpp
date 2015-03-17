// =============================================================================
// File Name: RectangleShape.cpp
// Description: Provides a wrapper for WinGDI rectangles
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "RectangleShape.hpp"

#include <QPainter>
#include <QPen>

RectangleShape::RectangleShape(const QPoint& size,
                               QColor fillColor,
                               QColor outlineColor,
                               int outlineThickness,
                               const QPoint& position) :
    Drawable(position, size, fillColor, outlineColor, outlineThickness) {
}

void RectangleShape::paintEvent(QPaintEvent* event) {
    (void) event;

    QPainter painter(this);
    QPen pen;
    painter.setPen(pen);

    pen.setColor(getOutlineColor());
    painter.drawRect(-getOutlineThickness(), -getOutlineThickness(),
                     getSize().x() + getOutlineThickness(),
                     getSize().y() + getOutlineThickness());

    pen.setColor(getOutlineColor());
    painter.drawRect(0, 0, getSize().x(), getSize().y());
}

