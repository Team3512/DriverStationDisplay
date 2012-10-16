//=============================================================================
//File Name: KinectStream.cpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Window/Context.hpp>

#include "KinectStream.hpp"
#include <fstream>
#include <sstream>
#include <iostream> // FIXME
#include <SFML/System/Sleep.hpp>

int stringToNumber( std::string str ) {
	int num;
	std::istringstream( str ) >> num;
	return num;
}

KinectStream::KinectStream( const std::string& hostName ,
		unsigned short port ,
		HWND parentWin ,
		int xPosition ,
		int yPosition ,
		int width ,
		int height ,
		HINSTANCE appInstance
		) :
		m_hostName( hostName ) ,
		m_port( port ) ,
		m_httpStream( m_hostName , m_port ) ,
		m_imageRequest( "/mjpg/video.mjpg" , sf::ContHttp::Request::Get ) ,
		m_recvStatus( sf::ContHttp::Response::Ok ) ,
		m_receiveThread( &KinectStream::receive , this ) ,
		m_closeThread( true ) {

	m_parentWin = parentWin;

	m_streamWin = CreateWindowEx( 0 ,
		"STATIC" ,
		"" ,
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE ,
		xPosition ,
		yPosition ,
		width ,
		height ,
		parentWin ,
		NULL ,
		appInstance ,
		NULL );
	m_streamDisplay.create( m_streamWin );

	startStream();
}

KinectStream::~KinectStream() {
	stopStream();

	DestroyWindow( m_streamWin );
}

void KinectStream::bringToTop() {
	// if top child window isn't stream display
	if ( GetTopWindow( m_parentWin ) != m_streamWin ) {
		BringWindowToTop( m_streamWin );
	}
}

sf::Vector2i KinectStream::getPosition() const {
	return m_streamDisplay.getPosition();
}

void KinectStream::setPosition( const sf::Vector2i& position ) {
	m_streamDisplay.setPosition( position );
}

sf::Vector2u KinectStream::getSize() const {
	return m_streamDisplay.getSize();
}

void KinectStream::setSize( const sf::Vector2u size ) {
	m_streamDisplay.setSize( size );
}

void KinectStream::startStream() {
	if ( m_closeThread == true ) { // if stream is closed, reopen it
		m_closeThread = false;
		m_receiveThread.launch();
	}
}

void KinectStream::stopStream() {
	if ( m_closeThread == false ) { // if stream is open, close it
		m_closeThread = true;

		m_receiveThread.wait();
	}
}

void KinectStream::receive() {
	sf::Context context;

	sf::Sprite imageSprite;
	sf::Texture kinectTexture;
	sf::Image kinectImage;

	while ( !m_closeThread ) {
		// Retrieve the Kinect's image from the Single Board computer and display it
		m_httpStream.startStream( m_imageRequest );

		if ( m_serverResponse.getStatus() == sf::ContHttp::Response::Ok ) {
			std::fstream saveStream( "img.jpg" , std::fstream::in );
			if ( saveStream.is_open() ) {
				// Sends "Content-Length" number of bytes into stream for saving jpeg
				saveStream << m_serverResponse.getBody().substr( 0 ,
						stringToNumber( m_serverResponse.getField( "Content-Length" ) ) );
				saveStream.close();
			}

			kinectTexture.loadFromFile( "img.jpg" );
			imageSprite.setTexture( kinectTexture );
		}

		m_streamDisplay.clear( sf::Color( 0 , 0 , 0 ) );

		m_streamDisplay.draw( imageSprite );

		m_streamDisplay.display();
	}
}
