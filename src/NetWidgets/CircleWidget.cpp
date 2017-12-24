// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "CircleWidget.hpp"

#include <cstring>

#include <QBrush>
#include <QPainter>
#include <QPen>

CircleWidget::CircleWidget(bool netUpdate, QWidget* parent)
    : QWidget(parent), NetWidget(netUpdate) {
    setActive(Status::inactive);
}

void CircleWidget::setActive(Status newStatus) {
    if (newStatus == Status::active) {
        m_color = QColor(0, 255, 0);
    } else if (newStatus == Status::standby) {
        m_color = QColor(255, 255, 0);
    } else {
        m_color = QColor(255, 0, 0);
    }

    m_status = newStatus;
}

CircleWidget::Status CircleWidget::getActive() { return m_status; }

QSize CircleWidget::sizeHint() const { return QSize(25, 25); }

void CircleWidget::updateEntry() {
    NetEntry& lightEntry = getNetEntry(m_varIds[0]);

    if (lightEntry.getType() == 'c') {
        setActive(static_cast<Status>(lightEntry.getValue<int32_t>()));
        update();
    }
}

void CircleWidget::paintEvent(QPaintEvent* event) {
    (void)event;

    QPainter painter(this);
    painter.translate(width() / 2, height() / 2);

    QPen pen(QColor(50, 50, 50));
    QBrush brush(QColor(50, 50, 50));

    painter.setPen(pen);
    painter.setBrush(brush);
    painter.drawEllipse(-12, -12, 24, 24);

    pen.setColor(m_color);
    brush.setColor(m_color);

    painter.setPen(pen);
    painter.setBrush(brush);
    painter.drawEllipse(-10, -10, 20, 20);
}
