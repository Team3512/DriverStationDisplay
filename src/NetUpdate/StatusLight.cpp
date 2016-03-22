// =============================================================================
// File Name: StatusLight.cpp
// Description: Shows green, yellow, or red circle depending on its internal
//             state
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "StatusLight.hpp"

#include <QHBoxLayout>

StatusLight::StatusLight(bool netUpdate, QWidget* parent) :
    QWidget(parent), NetUpdate(false) {
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    setLayout(layout);

    m_circle = new CircleWidget(netUpdate);
    layout->addWidget(m_circle);

    m_text = new Text(false);
    layout->addWidget(m_text);
}

void StatusLight::setString(const std::wstring& text) {
    m_text->setString(text);
}

std::wstring StatusLight::getString() const {
    return m_text->getString();
}

void StatusLight::updateKeys(std::vector<std::string>& keys) {
    m_circle->updateKeys(keys);
}

void StatusLight::updateValue() {
    setString(getUpdateText());
}
