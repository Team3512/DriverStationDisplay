// Copyright (c) FRC Team 3512, Spartatroniks 2012-2016. All Rights Reserved.

#pragma once

#include <string>

#include <QProgressBar>
#include <QVBoxLayout>

#include "NetUpdate.hpp"
#include "Text.hpp"

/**
 * Provides an interface to a progress bar
 */
class ProgressBar : public QWidget, public NetUpdate {
    Q_OBJECT

public:
    explicit ProgressBar(bool netUpdate, QWidget* parent = nullptr);

    void setPercent(int percent);
    int getPercent();

    void setString(const std::wstring& text);
    std::wstring getString();

    void updateValue();

private:
    QProgressBar* m_bar;
    Text* m_text;
};
