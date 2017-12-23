// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "Text.hpp"

Text::Text(bool netUpdate, QWidget* parent)
    : QLabel(parent), NetWidget(netUpdate) {}

void Text::setString(const std::wstring& text) {
    setText(QString::fromStdWString(text));
    update();
}

std::wstring Text::getString() const { return text().toStdWString(); }

void Text::updateEntry() {
    NetEntry& printValue = getNetEntry(m_varIds[0]);

    if (printValue.getType() == 's') {
        setString(NetWidget::fill(printValue));
    }
}
