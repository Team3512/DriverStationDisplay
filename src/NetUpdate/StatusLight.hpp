// Copyright (c) FRC Team 3512, Spartatroniks 2012-2017. All Rights Reserved.

#pragma once

#include <string>
#include <vector>

#include <QWidget>

#include "CircleWidget.hpp"
#include "NetUpdate.hpp"
#include "Text.hpp"

/**
 * Shows green, yellow, or red circle depending on its internal state
 */
class StatusLight : public QWidget, public NetUpdate {
    Q_OBJECT

public:
    explicit StatusLight(bool netUpdate, QWidget* parent = nullptr);

    void setString(const std::wstring& text);
    std::wstring getString() const;

    static void setColorBlind(bool on);
    static bool isColorBlind();

    void updateKeys(std::vector<std::string>& keys);

    void updateValue();

private:
    CircleWidget* m_circle;
    Text* m_text;
};
