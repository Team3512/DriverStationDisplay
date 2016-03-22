// =============================================================================
// Description: Provides an interface to a progress bar
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef PROGRESS_BAR_HPP
#define PROGRESS_BAR_HPP

#include <string>

#include <QProgressBar>
#include <QVBoxLayout>

#include "NetUpdate.hpp"
#include "Text.hpp"

class ProgressBar : public QWidget, public NetUpdate {
    Q_OBJECT

public:
    explicit ProgressBar(bool netUpdate, QWidget* parent = nullptr);

    void setPercent(int percent);
    int getPercent();

    void setString(const std::wstring& text);
    std::wstring getString();

    void updateValue();

private:
    QProgressBar* m_bar;
    Text* m_text;
};

#endif // PROGRESS_BAR_HPP
