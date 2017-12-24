// Copyright (c) 2012-2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include "CircleWidget.hpp"

#include <cstring>
#include <type_traits>

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
    NetEntry& lightEntry = getEntry(m_varIds[0]);

    std::visit(
        [&](auto&& arg) {
            int32_t value;
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int32_t>) {
                value = std::get<int32_t>(lightEntry);
            } else {
                value = std::stoi(std::get<std::wstring>(lightEntry));
            }
            setActive(static_cast<Status>(value));
            update();
        },
        lightEntry);
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
