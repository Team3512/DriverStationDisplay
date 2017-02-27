// Copyright (c) FRC Team 3512, Spartatroniks 2012-2017. All Rights Reserved.

#pragma once

#include <QLabel>
#include <QWidget>

#include "NetUpdate.hpp"

/**
 * Provides a wrapper for QLabel
 */
class Text : public QWidget, public NetUpdate {
    Q_OBJECT

public:
    explicit Text(bool netUpdate, QWidget* parent = nullptr);

    void setString(const std::wstring& text);
    std::wstring getString() const;

    void updateValue();

private:
    QLabel* m_text;
};
