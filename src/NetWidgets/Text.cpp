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
    NetEntry& printEntry = getEntry(m_varIds[0]);

    std::visit(
        [&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::wstring>) {
                setString(NetWidget::fillEntry(printEntry));
            }
        },
        printEntry);
}
