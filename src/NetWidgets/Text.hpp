// Copyright (c) 2012-2018 FRC Team 3512, Spartatroniks. All Rights Reserved.

#pragma once

#include <QLabel>

#include "NetWidget.hpp"

/**
 * Provides a wrapper for QLabel
 */
class Text : public QLabel, public NetWidget {
    Q_OBJECT

public:
    explicit Text(bool netUpdate, QWidget* parent = nullptr);

    void setString(const std::wstring& text);
    std::wstring getString() const;

    void updateEntry() override;
};
