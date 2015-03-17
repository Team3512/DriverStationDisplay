// =============================================================================
// File Name: ProgressBar.hpp
// Description: Provides an interface to a progress bar with WinGDI
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#include "RectangleShape.hpp"
#include "NetUpdate.hpp"
#include "Text.hpp"
#include <string>

class ProgressBar : public Drawable, public NetUpdate {
public:
    ProgressBar(std::wstring text,
                QColor fullFillColor,
                QColor emptyFillColor,
                QColor outlineColor,
                bool netUpdate,
                const QPoint& position = QPoint());

    void setPercent(float percentFull);
    float getPercent();

    void setPosition(const QPoint& position);
    void setPosition(short x, short y);

    void setSize(const QPoint& size);
    void setSize(short width, short height);

    void setString(const std::wstring& message);
    const std::wstring& getString();

    void setBarFillColor(QColor fill);
    QColor getBarFillColor();

    void paintEvent(QPaintEvent* event);

    void updateValue();

private:
    RectangleShape m_barOutline;
    RectangleShape m_barFill;

    float percent;

    Text m_text;
};

#endif // PROGRESSBAR_HPP

