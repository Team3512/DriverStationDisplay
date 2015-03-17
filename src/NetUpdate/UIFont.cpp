// =============================================================================
// File Name: UIFont.cpp
// Description: Provides a collection of fonts for use by other classes
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "UIFont.hpp"

UIFont& UIFont::getInstance() {
    static UIFont m_instance;
    return m_instance;
}

QFont& UIFont::segoeUI14() {
    return m_segoeUI14;
}

QFont& UIFont::segoeUI18() {
    return m_segoeUI18;
}

