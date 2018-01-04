// Copyright (c) 2012-2018 FRC Team 3512, Spartatroniks. All Rights Reserved.

#pragma once

#include <string>
#include <vector>

#include <QWidget>

#include "CircleWidget.hpp"
#include "NetWidget.hpp"
#include "Text.hpp"

/**
 * Shows green, yellow, or red circle depending on its internal state
 */
class StatusLight : public QWidget, public NetWidget {
    Q_OBJECT

public:
    explicit StatusLight(bool netUpdate, QWidget* parent = nullptr);

    void setString(const std::wstring& text);
    std::wstring getString() const;

    static void setColorBlind(bool on);
    static bool isColorBlind();

    void updateKeys(std::vector<std::string>& keys);

    void updateEntry() override;

private:
    CircleWidget* m_circle;
    Text* m_text;
};
