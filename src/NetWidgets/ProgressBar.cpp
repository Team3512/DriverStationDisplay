// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "ProgressBar.hpp"

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
    NetEntry& printEntry = getNetEntry(m_varIds[0]);
    NetEntry& percentEntry = getNetEntry(m_varIds[1]);

    if (printEntry.getType() != 0) {
        setString(NetWidget::fill(printEntry));
    }

    if (percentEntry.getType() == 'c' || percentEntry.getType() == 'i') {
        setPercent(percentEntry.getValue<int32_t>());
    } else if (percentEntry.getType() == 's') {
        setPercent(std::stoi(percentEntry.getValue<std::wstring>()));
    }
}
