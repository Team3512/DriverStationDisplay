// =============================================================================
// File Name: Text.cpp
// Description: Provides a wrapper for WinGDI text
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "Text.hpp"
#include "UIFont.hpp"

#include <QPainter>
#include <QPen>

Text::Text(QFont& font,
           std::wstring text,
           QColor fillColor,
           bool netUpdate,
           const QPoint& position) :
    Drawable(position, QPoint(0, 0), fillColor, QColor(0, 0, 0), 0),
    NetUpdate(netUpdate),
    m_font(font) {
    m_string = text;
}

void Text::setFont(QFont& font) {
    m_font = font;
}

const QFont& Text::getFont() const {
    return m_font;
}

void Text::setString(std::wstring text) {
    m_string = text;
}

const std::wstring& Text::getString() const {
    return m_string;
}

void Text::paintEvent(QPaintEvent* event) {
    (void) event;

    QPainter painter(this);
    QPen pen;
    pen.setColor(getFillColor());
    painter.setPen(pen);

    painter.drawText(getPosition().x(),
                     (painter.device()->height()) / 2.0,
                     QString::fromWCharArray(m_string.c_str()));
}

void Text::updateValue() {
    NetValue* printValue = getValue(m_varIds[0]);

    if (printValue != nullptr) {
        setString(NetUpdate::fillValue(printValue));
    }
}

