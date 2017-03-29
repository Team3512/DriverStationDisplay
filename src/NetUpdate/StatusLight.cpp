// Copyright (c) FRC Team 3512, Spartatroniks 2012-2017. All Rights Reserved.

#include "StatusLight.hpp"

#include <vector>

#include <QHBoxLayout>

StatusLight::StatusLight(bool netUpdate, QWidget* parent)
    : QWidget(parent), NetUpdate(false) {
    auto layout = new QHBoxLayout(this);
    setLayout(layout);

    m_circle = new CircleWidget(netUpdate, this);
    layout->addWidget(m_circle);

    m_text = new Text(false, this);
    layout->addWidget(m_text);

    layout->addStretch(1);
}

void StatusLight::setString(const std::wstring& text) {
    m_text->setString(text);
}

std::wstring StatusLight::getString() const { return m_text->getString(); }

void StatusLight::updateKeys(std::vector<std::string>& keys) {
    m_circle->updateKeys(keys);
}

void StatusLight::updateValue() { setString(getUpdateText()); }
