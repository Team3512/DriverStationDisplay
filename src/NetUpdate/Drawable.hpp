// =============================================================================
// File Name: Drawable.hpp
// Description: Provides interface for WinGDI drawable objects
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <QWidget>
#include <QPoint>
#include <QColor>

class Drawable : public QWidget {
    Q_OBJECT
public:
    Drawable(const QPoint& position,
             const QPoint& size,
             QColor fillColor,
             QColor outlineColor,
             int outlineThickness);
    virtual ~Drawable();

    // Draws the drawable to the currently stored device context
    virtual void paintEvent(QPaintEvent* event) = 0;

    virtual void setPosition(const QPoint& position);
    virtual void setPosition(short x, short y);

    const QPoint getPosition();

    virtual void setSize(const QPoint& size);
    virtual void setSize(short width, short height);

    const QPoint getSize();

    virtual void setFillColor(QColor color);
    QColor getFillColor();

    virtual void setOutlineColor(QColor color);
    QColor getOutlineColor();

    virtual void setOutlineThickness(int thickness);
    int getOutlineThickness();

protected:
    const QRect& getBoundingRect();

private:
    QRect m_boundingRect;

    QColor m_fillColor;
    QColor m_outlineColor;
    int m_outlineThickness;
};

#endif // DRAWABLE_HPP

