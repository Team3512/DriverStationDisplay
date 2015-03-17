// =============================================================================
// File Name: StatusLight.cpp
// Description: Shows green, yellow, or red circle depending on its internal
//             state
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "StatusLight.hpp"
#include "UIFont.hpp"

#include <QPainter>
#include <QPen>

#include <cstring>

bool StatusLight::m_isColorBlind = false;

StatusLight::StatusLight(std::wstring message,
                         bool netUpdate,
                         const QPoint& position) :
    Drawable(position, QPoint(0, 0), QColor(), QColor(), 0),
    NetUpdate(netUpdate),
    m_status(StatusLight::inactive),
    m_text(UIFont::getInstance().segoeUI14(), message, QColor(0, 0, 0), false,
           position) {
    setOutlineThickness(2);
    setOutlineColor(QColor(50, 50, 50));
    setPosition(position);
    setSize(24, 24);

    setActive(m_status);
}

void StatusLight::setActive(Status newStatus) {
    if (m_isColorBlind) {
        if (newStatus == StatusLight::active) {
            setFillColor(QColor(0, 0, 120));
        }
        else if (newStatus == StatusLight::standby) {
            setFillColor(QColor(128, 128, 0));
        }
        else {
            setFillColor(QColor(128, 0, 0));
        }
    }
    else {
        if (newStatus == StatusLight::active) {
            setFillColor(QColor(0, 120, 0));
        }
        else if (newStatus == StatusLight::standby) {
            setFillColor(QColor(128, 128, 0));
        }
        else {
            setFillColor(QColor(128, 0, 0));
        }
    }

    m_status = newStatus;
}

StatusLight::Status StatusLight::getActive() {
    return m_status;
}

void StatusLight::setPosition(const QPoint& position) {
    // Set position of circle
    Drawable::setPosition(QPoint(position.x() - 6, position.y() - 6));

    // Set position of text in relation to the circle
    m_text.setPosition(QPoint(position.x() + 12 /* diameter */ + 10,
                              position.y() - 3));
}

void StatusLight::setPosition(short x, short y) {
    // Set position of circle
    Drawable::setPosition(x - 6, y - 6);

    // Set position of text in relation to the circle
    m_text.setPosition(QPoint(x + 12 /* diameter */ + 10,
                              y - 3));
}

void StatusLight::setString(const std::wstring& message) {
    m_text.setString(message);
}

const std::wstring& StatusLight::getString() {
    return m_text.getString();
}

void StatusLight::setColorBlind(bool on) {
    m_isColorBlind = on;
}

bool StatusLight::isColorBlind() {
    return m_isColorBlind;
}

void StatusLight::paintEvent(QPaintEvent* event) {
    (void) event;

    QPainter painter(this);
    QPen pen;
    QBrush brush;
    painter.setPen(pen);
    painter.setBrush(brush);

    QFontMetrics* fm = new QFontMetrics(UIFont::getInstance().segoeUI14());

    int yShift = painter.device()->height() / 2;

    pen.setColor(getOutlineColor());
    brush.setColor(getOutlineColor());
    painter.drawEllipse(0, yShift - 12, 24, 24);

    pen.setColor(getFillColor());
    brush.setColor(getFillColor());
    painter.drawEllipse(2, yShift - 10, 20, 20);

    painter.drawText(30,
                     yShift + fm->lineSpacing() / 2.0,
                     QString::fromWCharArray(getUpdateText().c_str()));
}

void StatusLight::updateValue() {
    NetValue* lightValue = getValue(m_varIds[0]);

    if (lightValue != nullptr) {
        unsigned char tempVal = 0;
        std::memcpy(&tempVal, lightValue->getValue(), sizeof(tempVal));

        setActive(static_cast<Status>(tempVal));
    }

    setString(getUpdateText());
}

