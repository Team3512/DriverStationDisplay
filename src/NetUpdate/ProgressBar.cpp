// =============================================================================
// File Name: ProgressBar.cpp
// Description: Provides an interface to a progress bar with WinGDI
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "ProgressBar.hpp"
#include "UIFont.hpp"

ProgressBar::ProgressBar(std::wstring text,
                         QColor fullFillColor,
                         QColor emptyFillColor,
                         QColor outlineColor,
                         bool netUpdate,
                         const QPoint& position) :
    Drawable(position, QPoint(100, 18), emptyFillColor, outlineColor, 1),
    NetUpdate(netUpdate),
    m_barOutline(QPoint(100, 18), emptyFillColor, outlineColor, 1,
                 position),
    m_barFill(QPoint(98, 16),
              fullFillColor,
              QColor(0, 70, 0), 0,
              QPoint(position.x() + 1, position.y() + 1)),
    m_text(UIFont::getInstance().segoeUI14(), text, QColor(0, 0, 0),
           false,
           QPoint(position.x(), position.y() + 18 + 2)) {
    setPercent(0.f);
}

void ProgressBar::paintEvent(QPaintEvent* event) {
    (void) event;

    m_barOutline.update();
    m_barFill.update();
    m_text.update();
}

void ProgressBar::setPercent(float percentFull) {
    if (percentFull > 100) {
        percentFull = 100;
    }

    percent = percentFull;
    m_barFill.setSize(QPoint((Drawable::getSize().x() - 2.f) * percentFull /
                             100.f, m_barFill.getSize().y()));
}

float ProgressBar::getPercent() {
    return percent;
}

void ProgressBar::setPosition(const QPoint& position) {
    m_barOutline.setPosition(position);
    m_barFill.setPosition(position.x() + 1, position.y() + 1);

    m_text.setPosition(
        m_barOutline.getPosition().x(),
        m_barOutline.getPosition().y() + m_barOutline.getSize().y() + 2);
}

void ProgressBar::setPosition(short x, short y) {
    Drawable::setPosition(x, y);
    m_barFill.setPosition(x + 1, y + 1);

    m_text.setPosition(
        m_barOutline.getPosition().x(),
        m_barOutline.getPosition().y() + m_barOutline.getSize().y() + 2);
}

void ProgressBar::setSize(const QPoint& size) {
    m_barOutline.setSize(size);
    m_barFill.setSize(QPoint((size.y() - 2) * percent, size.y() - 2));

    m_text.setPosition(
        m_barOutline.getPosition().x(),
        m_barOutline.getPosition().y() + m_barOutline.getSize().y() + 2);
}

void ProgressBar::setSize(short width, short height) {
    m_barOutline.setSize(QPoint(width, height));
    m_barFill.setSize(QPoint((width - 2) * percent, height - 2));

    m_text.setPosition(
        m_barOutline.getPosition().x(),
        m_barOutline.getPosition().y() + m_barOutline.getSize().y() + 2);
}

void ProgressBar::setString(const std::wstring& message) {
    m_text.setString(message);
}

const std::wstring& ProgressBar::getString() {
    return m_text.getString();
}

void ProgressBar::setBarFillColor(QColor fill) {
    m_barFill.setFillColor(fill);
}

QColor ProgressBar::getBarFillColor() {
    return m_barFill.getFillColor();
}

void ProgressBar::updateValue() {
    NetValue* printValue = getValue(m_varIds[0]);
    NetValue* percentValue = getValue(m_varIds[1]);

    if (printValue != nullptr) {
        setString(NetUpdate::fillValue(printValue));
    }

    if (percentValue != nullptr) {
        setPercent(*static_cast<unsigned char*>(percentValue->getValue()));
    }
}

