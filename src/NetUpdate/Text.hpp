// =============================================================================
// Description: Provides a wrapper for QLabel
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef TEXT_HPP
#define TEXT_HPP

#include <QLabel>
#include <QWidget>

#include "NetUpdate.hpp"

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

#endif // TEXT_HPP
