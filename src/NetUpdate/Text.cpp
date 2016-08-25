// Copyright (c) FRC Team 3512, Spartatroniks 2012-2016. All Rights Reserved.

#include "Text.hpp"

Text::Text(bool netUpdate, QWidget* parent)
    : QWidget(parent), NetUpdate(netUpdate) {
    m_text = new QLabel(this);
}

void Text::setString(const std::wstring& text) {
    m_text->setText(QString::fromStdWString(text));
    m_text->update();
}

std::wstring Text::getString() const { return m_text->text().toStdWString(); }

void Text::updateValue() {
    NetValue* printValue = getValue(m_varIds[0]);

    if (printValue != nullptr) {
        std::wcout << "text=" << NetUpdate::fillValue(printValue) << "\n";
        setString(NetUpdate::fillValue(printValue));
    }
}
