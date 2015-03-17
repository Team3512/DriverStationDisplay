// =============================================================================
// File Name: Drawable.cpp
// Description: Provides interface for WinGDI drawable objects
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "Drawable.hpp"

Drawable::Drawable(const QPoint& position,
                   const QPoint& size,
                   QColor fillColor,
                   QColor outlineColor,
                   int outlineThickness) {
    m_boundingRect.setLeft(position.x());
    m_boundingRect.setTop(position.y());
    m_boundingRect.setRight(position.x() + size.x());
    m_boundingRect.setBottom(position.y() + size.y());

    m_fillColor = fillColor;
    m_outlineColor = outlineColor;
    m_outlineThickness = outlineThickness;
}

Drawable::~Drawable() {
}

void Drawable::setPosition(const QPoint& position) {
    // Keep width and height the same at the new position
    m_boundingRect.setRight(position.x() +
                            (m_boundingRect.right() - m_boundingRect.left()));
    m_boundingRect.setBottom(position.y() +
                             (m_boundingRect.bottom() - m_boundingRect.top()));

    m_boundingRect.setLeft(position.x());
    m_boundingRect.setTop(position.y());
}

void Drawable::setPosition(short x, short y) {
    // Keep width and height the same at the new position
    m_boundingRect.setRight(x +
                            (m_boundingRect.right() - m_boundingRect.left()));
    m_boundingRect.setBottom(
        y + (m_boundingRect.bottom() - m_boundingRect.top()));

    m_boundingRect.setLeft(x);
    m_boundingRect.setTop(y);
}

const QPoint Drawable::getPosition() {
    return QPoint(static_cast<short>(m_boundingRect.left()),
                  static_cast<short>(m_boundingRect.top()));
}

void Drawable::setSize(const QPoint& size) {
    m_boundingRect.setRight(m_boundingRect.left() + size.x());
    m_boundingRect.setBottom(m_boundingRect.top() + size.y());
}

void Drawable::setSize(short width, short height) {
    m_boundingRect.setRight(m_boundingRect.left() + width);
    m_boundingRect.setBottom(m_boundingRect.top() + height);
}

const QPoint Drawable::getSize() {
    return QPoint(static_cast<short>(m_boundingRect.right() -
                                     m_boundingRect.left()),
                  static_cast<short>(m_boundingRect.bottom() -
                                     m_boundingRect.top()));
}

void Drawable::setFillColor(QColor color) {
    m_fillColor = color;
}

QColor Drawable::getFillColor() {
    return m_fillColor;
}

void Drawable::setOutlineColor(QColor color) {
    m_outlineColor = color;
}

QColor Drawable::getOutlineColor() {
    return m_outlineColor;
}

void Drawable::setOutlineThickness(int thickness) {
    m_outlineThickness = thickness;
}

int Drawable::getOutlineThickness() {
    return m_outlineThickness;
}

const QRect& Drawable::getBoundingRect() {
    return m_boundingRect;
}

