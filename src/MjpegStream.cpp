//=============================================================================
//File Name: KinectStream.cpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Window/Context.hpp>

#include "MjpegStream.hpp"
#include <fstream>
#include <sstream>
#include <iostream> // FIXME
#include <fstream>
#include <SFML/System/Sleep.hpp>

int stringToNumber( std::string str ) {
	int num;
	std::istringstream( str ) >> num;
	return num;
}

MjpegStream::MjpegStream( const std::string& hostName ,
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
		m_receiveThread( &MjpegStream::receive , this ) ,
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

MjpegStream::~MjpegStream() {
	stopStream();

	DestroyWindow( m_streamWin );
}

void MjpegStream::bringToTop() {
	// if top child window isn't stream display
	if ( GetTopWindow( m_parentWin ) != m_streamWin ) {
		BringWindowToTop( m_streamWin );
	}
}

sf::Vector2i MjpegStream::getPosition() const {
	return m_streamDisplay.getPosition();
}

void MjpegStream::setPosition( const sf::Vector2i& position ) {
	m_streamDisplay.setPosition( position );
}

sf::Vector2u MjpegStream::getSize() const {
	return m_streamDisplay.getSize();
}

void MjpegStream::setSize( const sf::Vector2u size ) {
	m_streamDisplay.setSize( size );
}

void MjpegStream::startStream() {
	if ( m_closeThread == true ) { // if stream is closed, reopen it
		m_closeThread = false;
		m_receiveThread.launch();
	}
}

void MjpegStream::stopStream() {
	if ( m_closeThread == false ) { // if stream is open, close it
		m_closeThread = true;

		m_receiveThread.wait();
	}
}

void MjpegStream::receive() {
	sf::Context context;

	sf::Sprite imageSprite;
	sf::Texture kinectTexture;
	sf::Image kinectImage;

	while ( !m_closeThread ) {
		// Retrieve the Kinect's image from the Single Board computer and display it
		m_httpStream.startStream( m_imageRequest );

		if ( m_serverResponse.getStatus() == sf::ContHttp::Response::Ok ) {
			const void* imageBuffer = static_cast<const void*>( m_serverResponse.getBody().substr( 0 ,
				stringToNumber( m_serverResponse.getField( "Content-Length" ) ) ).c_str() );

			kinectTexture.loadFromMemory( imageBuffer , stringToNumber( m_serverResponse.getField( "Content-Length" ) ) );
			imageSprite.setTexture( kinectTexture );
		}

		m_streamDisplay.clear( sf::Color( 0 , 0 , 0 ) );

		m_streamDisplay.draw( imageSprite );

		m_streamDisplay.display();
	}
}
