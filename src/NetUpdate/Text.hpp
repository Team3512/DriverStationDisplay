// =============================================================================
// File Name: Text.hpp
// Description: Provides a wrapper for WinGDI text
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

/* In this class's case, outlineColor is actually the background color of the
 * text
 */

#ifndef TEXT_HPP
#define TEXT_HPP

#include "Drawable.hpp"
#include "NetUpdate.hpp"
#include <string>

class Text : public Drawable, public NetUpdate {
public:
    Text(QFont& font,
         std::wstring text,
         QColor fillColor,
         bool netUpdate,
         const QPoint& position = QPoint());

    void setFont(QFont& font);

    const QFont& getFont() const;

    void setString(std::wstring text);

    const std::wstring& getString() const;

    void paintEvent(QPaintEvent* event);

    void updateValue();

private:
    // The following functions don't do anything
    /* void setSize(const Vector& size);
     * void setSize(short width, short height);
     *
     * const Vector& getSize();
     *
     * void setOutlineThickness(int thickness);
     */

    QFont& m_font;
    std::wstring m_string;
};

#endif // TEXT_HPP

