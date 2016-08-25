// Copyright (c) FRC Team 3512, Spartatroniks 2012-2016. All Rights Reserved.

#ifndef TEXT_HPP
#define TEXT_HPP

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

#endif  // TEXT_HPP
