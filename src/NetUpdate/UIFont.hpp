// =============================================================================
// File Name: UIFont.hpp
// Description: Provides a collection of fonts for use by other classes
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef UIFONT_HPP
#define UIFONT_HPP

#include <QFont>

class UIFont {
public:
    // @return a global instance of the fonts available
    static UIFont& getInstance();

    QFont& segoeUI14();
    QFont& segoeUI18();

private:
    QFont m_segoeUI14{"Segoe UI", 14, QFont::Normal};
    QFont m_segoeUI18{"Segoe UI", 18, QFont::Normal};

    UIFont() = default;
    UIFont(const UIFont&) = delete;
    void operator=(const UIFont&) = delete;
};

#endif // UIFONT_HPP

