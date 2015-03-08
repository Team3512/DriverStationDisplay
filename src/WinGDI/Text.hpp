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

#include "../OpenGL/Drawable.hpp"
#include "../OpenGL/NetUpdate.hpp"
#include <string>

class Text : public Drawable, public NetUpdate {
public:
    Text(const Vector2i& position,
         HFONT font,
         std::wstring text,
         Colorf fillColor,
         Colorf outlineColor,
         bool netUpdate);

    void setFont(HFONT font);

    const HFONT getFont() const;

    void setString(std::wstring text);

    const std::wstring& getString() const;

    void draw(HDC hdc);

    void updateValue();

private:
    // The following functions don't do anything
    /* void setSize( const Vector& size );
     * void setSize( short width , short height );
     *
     * const Vector& getSize();
     *
     * void setOutlineThickness( int thickness );
     */

    HFONT m_font;
    std::wstring m_string;
};

#endif // TEXT_HPP

