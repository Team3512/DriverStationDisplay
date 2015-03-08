// =============================================================================
// File Name: ProgressBar.hpp
// Description: Provides an interface to a progress bar with WinGDI
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#include "RectangleShape.hpp"
#include "NetUpdate.hpp"
#include "../WinGDI/Text.hpp"
#include <string>

class ProgressBar : public RectangleShape, public NetUpdate {
public:
    ProgressBar(const Vector2i& position,
                std::wstring text,
                Colorf fullFillColor,
                Colorf emptyFillColor,
                Colorf outlineColor,
                bool netUpdate);

    void setPercent(float percentFull);
    float getPercent();

    void setPosition(const Vector2i& position);
    void setPosition(short x, short y);

    void setSize(const Vector2i& size);
    void setSize(short width, short height);

    void setString(const std::wstring& message);
    const std::wstring& getString();

    void setBarFillColor(Colorf fill);
    Colorf getBarFillColor();

    void draw(HDC hdc);

    void updateValue();

private:
    RectangleShape m_barFill;

    float percent;

    Text m_text;
};

#endif // PROGRESSBAR_HPP

