// Copyright (c) 2012-2018 FRC Team 3512. All Rights Reserved.

#pragma once

#include <QWidget>

#include "NetWidget.hpp"

/**
 * Used to draw circle in other widgets
 */
class CircleWidget : public QWidget, public NetWidget {
    Q_OBJECT

public:
    enum Status { active, standby, inactive };

    explicit CircleWidget(bool netUpdate, QWidget* parent = nullptr);

    void setActive(Status newStatus);
    Status getActive();

    QSize sizeHint() const;

    void updateEntry() override;

protected:
    void paintEvent(QPaintEvent* event);

private:
    Status m_status;
    QColor m_color;
};
