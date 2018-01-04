// Copyright (c) 2012-2018 FRC Team 3512. All Rights Reserved.

#include "ProgressBar.hpp"

#include <type_traits>

ProgressBar::ProgressBar(bool netUpdate, QWidget* parent)
    : QWidget(parent), NetWidget(netUpdate) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);

    m_bar = new QProgressBar(this);
    m_bar->setMinimum(0);
    m_bar->setMaximum(100);
    layout->addWidget(m_bar);

    m_text = new Text(false, this);
    layout->addWidget(m_text);

    setPercent(0.f);
}

void ProgressBar::setPercent(int percent) {
    if (percent > 100) {
        percent = 100;
    }

    m_bar->setValue(percent);
    m_bar->update();
}

int ProgressBar::getPercent() { return m_bar->value(); }

void ProgressBar::setString(const std::wstring& text) {
    m_text->setString(text);
}

std::wstring ProgressBar::getString() { return m_text->getString(); }

void ProgressBar::updateEntry() {
    NetEntry& printEntry = getEntry(m_varIds[0]);
    setString(NetWidget::fillEntry(printEntry));

    NetEntry& percentEntry = getEntry(m_varIds[1]);

    std::visit(
        [&](auto&& arg) {
            int32_t value;
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int32_t>) {
                value = std::get<int32_t>(percentEntry);
            } else {
                value = std::stoi(std::get<std::wstring>(percentEntry));
            }
            setPercent(value);
        },
        percentEntry);
}
