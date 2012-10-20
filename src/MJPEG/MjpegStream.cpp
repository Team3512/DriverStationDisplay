//=============================================================================
//File Name: KinectStream.cpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#include <SFML/Graphics/Texture.hpp>

#include "../UIFont.hpp"
#include "MjpegStream.hpp"
#include <iostream> // TODO Remove me
#include <fstream>
#include <sstream>

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
		m_connectMsg( "Connecting..." , UIFont::getInstance()->segoeUI() , 18 ) ,
		m_disconnectMsg( "Disconnected" , UIFont::getInstance()->segoeUI() , 18 ) ,

		m_stopReceive( true ) {

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

	/* ===== Fill RenderTextures with special messages ===== */
	m_connectMsg.setPosition( sf::Vector2f( ( m_streamDisplay.getSize().x - m_connectMsg.findCharacterPos( 13 ).x ) / 2.f , ( m_streamDisplay.getSize().y - m_connectMsg.getCharacterSize() - 6.f ) / 2.f ) );

	m_connectTxtr.create( m_streamDisplay.getSize().x , m_streamDisplay.getSize().y );
	m_connectTxtr.clear( sf::Color( 40 , 40 , 40 ) );
	m_connectTxtr.draw( m_connectMsg );
	m_connectTxtr.display();

	m_disconnectMsg.setPosition( sf::Vector2f( ( m_streamDisplay.getSize().x - m_disconnectMsg.findCharacterPos( 12 ).x ) / 2.f , ( m_streamDisplay.getSize().y - m_disconnectMsg.getCharacterSize() - 6.f ) / 2.f ) );

	m_disconnectTxtr.create( m_streamDisplay.getSize().x , m_streamDisplay.getSize().y );
	m_disconnectTxtr.clear( sf::Color( 40 , 40 , 40 ) );
	m_disconnectTxtr.draw( m_disconnectMsg );
	m_disconnectTxtr.display();
	/* ===================================================== */

	/* Makes stream display "Connecting..." until at least one image has been received from the host
	 * No mutexes used here because receive thread hasn't started yet
	 */
	m_imageSprite.setTexture( m_connectTxtr.getTexture() );

	// Set up the callback description structure
	ZeroMemory( &m_callbacks , sizeof(struct mjpeg_callbacks_t) );
	m_callbacks.readcallback = readCallback;
	m_callbacks.donecallback = doneCallback;
	m_callbacks.optarg = this;

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

sf::Vector2i MjpegStream::getPosition() {
	sf::Vector2i tempPos;

	m_displayMutex.lock();
	tempPos = m_streamDisplay.getPosition();
	m_displayMutex.unlock();

	return tempPos;
}

void MjpegStream::setPosition( const sf::Vector2i& position ) {
	m_displayMutex.lock();
	m_streamDisplay.setPosition( position );
	m_displayMutex.unlock();
}

sf::Vector2u MjpegStream::getSize() {
	sf::Vector2u tempPos;

	m_displayMutex.lock();
	tempPos = m_streamDisplay.getSize();
	m_displayMutex.unlock();

	return tempPos;
}

void MjpegStream::setSize( const sf::Vector2u size ) {
	m_displayMutex.lock();

	m_streamDisplay.setSize( size );

	m_connectTxtr.create( m_streamDisplay.getSize().x , m_streamDisplay.getSize().y );
	m_connectMsg.setPosition( sf::Vector2f( ( m_streamDisplay.getSize().x - m_connectMsg.findCharacterPos( 13 ).x ) / 2.f , ( m_streamDisplay.getSize().y - m_connectMsg.getCharacterSize() - 6.f ) / 2.f ) );

	m_disconnectTxtr.create( m_streamDisplay.getSize().x , m_streamDisplay.getSize().y );
	m_disconnectMsg.setPosition( sf::Vector2f( ( m_streamDisplay.getSize().x - m_disconnectMsg.findCharacterPos( 12 ).x ) / 2.f , ( m_streamDisplay.getSize().y - m_disconnectMsg.getCharacterSize() - 6.f ) / 2.f ) );

	m_displayMutex.unlock();

	/* ===== Fill RenderTextures with messages ===== */
	m_connectTxtr.clear( sf::Color( 40 , 40 , 40 ) );
	m_connectTxtr.draw( m_connectMsg );
	m_connectTxtr.display();


	m_disconnectTxtr.clear( sf::Color( 40 , 40 , 40 ) );
	m_disconnectTxtr.draw( m_disconnectMsg );
	m_disconnectTxtr.display();
	/* ============================================= */
}

void MjpegStream::startStream() {
	if ( m_stopReceive == true ) { // if stream is closed, reopen it
		m_stopReceive = false;

		// Launch the mjpeg recieving/processing thread
		m_streamInst = mjpeg_launchthread( const_cast<char*>( m_hostName.c_str() ) , m_port , &m_callbacks );
	}
}

void MjpegStream::stopStream() {
	if ( m_stopReceive == false ) { // if stream is open, close it
		m_stopReceive = true;

		// Close the receive thread
		mjpeg_stopthread( m_streamInst );
	}
}

bool MjpegStream::isStreaming() {
	return !m_stopReceive;
}

void MjpegStream::display() {
	m_displayMutex.lock();

	if ( isStreaming() ) {
		if ( m_imageAge.getElapsedTime().asMilliseconds() > 1000 ) {
			m_streamDisplay.draw( sf::Sprite( m_connectTxtr.getTexture() ) );
		}
		else {
			m_imageMutex.lock();
			m_streamDisplay.draw( m_imageSprite );
			m_imageMutex.unlock();
		}
	}
	else {
		m_streamDisplay.draw( sf::Sprite( m_disconnectTxtr.getTexture() ) );
	}

	m_streamDisplay.display();

	m_displayMutex.unlock();
}

void MjpegStream::doneCallback( void* optarg ) {
	static_cast<MjpegStream*>(optarg)->stopStream();
}

void MjpegStream::readCallback( char* buf , int bufsize , void* optarg ) {
	// Store JPEG image to buffer
	sf::Texture streamTexture;

	// If the image loaded successfully, display it
	if ( streamTexture.loadFromMemory( static_cast<void*>(buf) , bufsize ) ) {
		// Create pointer to stream to make it easier to access the instance later
		MjpegStream* streamPtr = static_cast<MjpegStream*>( optarg );
		std::cout << "streamPtr=" << streamPtr << "\n";

		// Set up image for drawing
		streamPtr->m_imageMutex.lock();
		streamPtr->m_imageSprite.setTexture( streamTexture );
		streamPtr->m_imageMutex.unlock();

		// Reset the image age timer
		streamPtr->m_imageAge.restart();

		std::cout << "Got image\n";
	}
}
