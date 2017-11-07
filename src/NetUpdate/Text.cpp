// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "Text.hpp"

Text::Text(bool netUpdate, QWidget* parent)
    : QLabel(parent), NetUpdate(netUpdate) {}

void Text::setString(const std::wstring& text) {
    setText(QString::fromStdWString(text));
    update();
}

std::wstring Text::getString() const { return text().toStdWString(); }

void Text::updateValue() {
    NetValue* printValue = getValue(m_varIds[0]);

    if (printValue != nullptr) {
        std::wcout << "text=" << NetUpdate::fillValue(printValue) << std::endl;
        setString(NetUpdate::fillValue(printValue));
    }
}
