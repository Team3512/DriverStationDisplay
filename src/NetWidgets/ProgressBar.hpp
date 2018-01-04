// Copyright (c) 2012-2018 FRC Team 3512. All Rights Reserved.

#pragma once

#include <string>

#include <QProgressBar>
#include <QVBoxLayout>

#include "NetWidget.hpp"
#include "Text.hpp"

/**
 * Provides an interface to a progress bar
 */
class ProgressBar : public QWidget, public NetWidget {
    Q_OBJECT

public:
    explicit ProgressBar(bool netUpdate, QWidget* parent = nullptr);

    void setPercent(int percent);
    int getPercent();

    void setString(const std::wstring& text);
    std::wstring getString();

    void updateEntry() override;

private:
    QProgressBar* m_bar;
    Text* m_text;
};
