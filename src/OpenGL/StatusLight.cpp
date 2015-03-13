// =============================================================================
// File Name: StatusLight.cpp
// Description: Shows green, yellow, or red circle depending on its internal
//             state
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "StatusLight.hpp"
#include "../WinGDI/UIFont.hpp"

#include <cstring>
#include <cmath>
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#include <wingdi.h>

bool StatusLight::m_isColorBlind = false;

StatusLight::StatusLight(const Vector2i& position,
                         std::wstring message,
                         bool netUpdate) :
    Drawable(position, Vector2i(0, 0), Colorf(), Colorf(), 0),
    NetUpdate(netUpdate),
    m_status(StatusLight::inactive),
    m_text(position, UIFont::getInstance().segoeUI14(), message, Colorf(0,
                                                                        0,
                                                                        0),
           Colorf(GetRValue(GetSysColor(COLOR_3DFACE)),
                  GetGValue(GetSysColor(COLOR_3DFACE)), GetBValue(GetSysColor(
                                                                      COLOR_3DFACE))),
           false) {
    setOutlineThickness(2);
    setOutlineColor(Colorf(50, 50, 50));
    setPosition(position);
    setSize(24, 24);

    setActive(m_status);
}

void StatusLight::draw(HDC hdc) {
    if (hdc == NULL) {
        // Draw outline
        glBegin(GL_POLYGON);
        glColor4f(Drawable::getOutlineColor().glR(),
                  Drawable::getOutlineColor().glG(),
                  Drawable::getOutlineColor().glB(),
                  Drawable::getOutlineColor().glA());
        double radius = 12;
        for (double i = 0; i < 2 * M_PI; i += M_PI / 12) {
            glVertex3f(getBoundingRect().left + radius + std::cos(
                           i) * radius,
                       getBoundingRect().top + radius + std::sin(
                           i) * radius, 0);
        }
        glEnd();

        // Draw fill
        glBegin(GL_POLYGON);
        glColor4f(Drawable::getFillColor().glR(),
                  Drawable::getFillColor().glG(),
                  Drawable::getFillColor().glB(),
                  Drawable::getFillColor().glA());
        radius = 10;
        for (double i = 0; i < 2 * M_PI; i += M_PI / 12) {
            glVertex3f(getBoundingRect().left + (radius + 2) + std::cos(
                           i) * radius,
                       getBoundingRect().top + (radius + 2) + std::sin(
                           i) * radius, 0);
        }
        glEnd();
    }
    else {
        m_text.draw(hdc);
    }
}

void StatusLight::setActive(Status newStatus) {
    if (m_isColorBlind) {
        if (newStatus == StatusLight::active) {
            setFillColor(Colorf(0, 0, 120));
        }
        else if (newStatus == StatusLight::standby) {
            setFillColor(Colorf(128, 128, 0));
        }
        else {
            setFillColor(Colorf(128, 0, 0));
        }
    }
    else {
        if (newStatus == StatusLight::active) {
            setFillColor(Colorf(0, 120, 0));
        }
        else if (newStatus == StatusLight::standby) {
            setFillColor(Colorf(128, 128, 0));
        }
        else {
            setFillColor(Colorf(128, 0, 0));
        }
    }

    m_status = newStatus;
}

StatusLight::Status StatusLight::getActive() {
    return m_status;
}

void StatusLight::setPosition(const Vector2i& position) {
    // Set position of circle
    Drawable::setPosition(Vector2i(position.X - 6, position.Y - 6));

    // Set position of text in relation to the circle
    m_text.setPosition(Vector2i(position.X + 12 /* diameter */ + 10,
                                position.Y - 3));
}

void StatusLight::setPosition(short x, short y) {
    // Set position of circle
    Drawable::setPosition(x - 6, y - 6);

    // Set position of text in relation to the circle
    m_text.setPosition(Vector2i(x + 12 /* diameter */ + 10,
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

void StatusLight::updateValue() {
    NetValue* lightValue = getValue(m_varIds[0]);

    if (lightValue != NULL) {
        unsigned char tempVal = 0;
        std::memcpy(&tempVal, lightValue->getValue(), sizeof(unsigned char));

        setActive(Status(tempVal));
    }

    setString(getUpdateText());
}

