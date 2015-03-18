#ifndef CIRCLE_WIDGET_HPP
#define CIRCLE_WIDGET_HPP

#include <QWidget>

#include "NetUpdate.hpp"

class CircleWidget : public QWidget, public NetUpdate {
    Q_OBJECT
public:
    enum Status {
        active,
        standby,
        inactive
    };

    explicit CircleWidget(bool netUpdate, QWidget* parent = nullptr);

    void setActive(Status newStatus);
    Status getActive();

    QSize sizeHint() const;

    void updateValue();

protected:
    void paintEvent(QPaintEvent* event);

private:
    Status m_status;
    QColor m_color;
};

#endif // CIRCLE_WIDGET_HPP

