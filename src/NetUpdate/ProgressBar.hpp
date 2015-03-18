// =============================================================================
// File Name: ProgressBar.hpp
// Description: Provides an interface to a progress bar with WinGDI
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#include <QVBoxLayout>
#include <QProgressBar>

#include "NetUpdate.hpp"
#include "Text.hpp"
#include <string>

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

#endif // PROGRESSBAR_HPP

