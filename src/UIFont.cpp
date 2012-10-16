//=============================================================================
//File Name: UIFont.cpp
//Description: Provides a collection of fonts for use by other classes
//Author: Tyler Veness
//=============================================================================

#include "UIFont.hpp"

UIFont* UIFont::m_instance = NULL;
sf::Font UIFont::m_segoeUI;

UIFont* UIFont::getInstance() {
	if ( m_instance == NULL ) {
		m_instance = new UIFont;
	}

	return m_instance;
}

UIFont::UIFont() {
	m_segoeUI.loadFromFile( "segoeui.ttf" );
}

sf::Font& UIFont::segoeUI() {
	return m_segoeUI;
}
