//=============================================================================
//File Name: KinectStream.cpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "../UIFont.hpp"
#include "MjpegStream.hpp"
#include "../ButtonID.hpp"

#include <fstream>
#include <sstream>
#include <iostream> // TODO Remove me

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
		m_waitMsg( "Waiting..." , UIFont::getInstance()->segoeUI() , 18 ) ,

		m_firstImage( true ) ,

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

	/* ===== Initialize the stream toggle button ===== */
	HGDIOBJ hfDefault = GetStockObject( DEFAULT_GUI_FONT );

	m_toggleButton = CreateWindowEx( 0,
		"BUTTON",
		"Start Stream",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		( GetSystemMetrics(SM_CXSCREEN) - 320 ) / 2 ,
		305,
		100,
		24,
		m_parentWin,
		reinterpret_cast<HMENU>( IDC_STREAM_BUTTON ),
		GetModuleHandle( NULL ),
		NULL);

	SendMessage(m_toggleButton,
		WM_SETFONT,
		reinterpret_cast<WPARAM>( hfDefault ),
		MAKELPARAM( FALSE , 0 ) );
	/* =============================================== */

	// Create the textures that can be displayed in the stream window
	recreateTextures( m_streamDisplay.getSize() );

	// Set up the callback description structure
	ZeroMemory( &m_callbacks , sizeof(struct mjpeg_callbacks_t) );
	m_callbacks.readcallback = readCallback;
	m_callbacks.donecallback = doneCallback;
	m_callbacks.optarg = this;
}

MjpegStream::~MjpegStream() {
	stopStream();
	DestroyWindow( m_streamWin );
	DestroyWindow( m_toggleButton );
}

void MjpegStream::bringToTop() {
	// if top child window isn't stream toggle button
	if ( GetTopWindow( m_parentWin ) != m_toggleButton ) {
		// Put stream toggle button on top followed by stream window in Z-order
		BringWindowToTop( m_streamWin );
		BringWindowToTop( m_toggleButton );
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
	m_displayMutex.unlock();

	recreateTextures( size );
}

void MjpegStream::startStream() {
	if ( m_stopReceive == true ) { // if stream is closed, reopen it
		m_stopReceive = false;

		// Change text displayed on button (LParam is button HWND)
		SendMessage( m_toggleButton , WM_SETTEXT , 0 , reinterpret_cast<LPARAM>("Stop Stream") );

		m_imageAge.restart();

		// Launch the mjpeg recieving/processing thread
		m_streamInst = mjpeg_launchthread( const_cast<char*>( m_hostName.c_str() ) , m_port , &m_callbacks );
	}
}

void MjpegStream::stopStream() {
	if ( m_stopReceive == false ) { // if stream is open, close it
		m_stopReceive = true;

		// Change text displayed on button (LParam is button HWND)
		SendMessage( m_toggleButton , WM_SETTEXT , 0 , reinterpret_cast<LPARAM>("Start Stream") );

		std::cout << "Closing network thread... ";

		// Close the receive thread
		mjpeg_stopthread( m_streamInst );

		std::cout << "closeDone\n";

		m_firstImage = true;
	}
}

bool MjpegStream::isStreaming() {
	return !m_stopReceive;
}

void MjpegStream::display() {
	// If streaming is enabled
	if ( isStreaming() ) {
		// Create safe versions of variables
		m_imageMutex.lock();

		bool sFirstImage = m_firstImage;
		int sImageAge = m_imageAge.getElapsedTime().asMilliseconds();

		m_imageMutex.unlock();

		// If no image has been received yet
		if ( sFirstImage ) {
			m_imageMutex.lock();
			m_displayMutex.lock();

			m_streamDisplay.draw( sf::Sprite( m_connectTxtr.getTexture() ) );

			m_displayMutex.unlock();
			m_imageMutex.unlock();
		}

		// If it's been too long since we received our last image
		else if ( sImageAge > 1000 ) {
			// Display "Waiting..." over the last image received
			m_imageMutex.lock();
			m_displayMutex.lock();

			m_streamDisplay.clear( sf::Color( 40 , 40 , 40 ) );
			m_streamDisplay.draw( sf::Sprite( m_imageTxtr ) );
			m_streamDisplay.draw( sf::Sprite( m_waitTxtr.getTexture() ) );

			m_displayMutex.unlock();
			m_imageMutex.unlock();
		}

		// Else display the image last received
		else {
			m_imageMutex.lock();
			m_displayMutex.lock();

			m_streamDisplay.clear( sf::Color( 40 , 40 , 40 ) );
			m_streamDisplay.draw( sf::Sprite( m_imageTxtr ) );

			m_displayMutex.unlock();
			m_imageMutex.unlock();
		}
	}

	// Else we aren't connected to the host
	else {
		m_imageMutex.lock();
		m_displayMutex.lock();

		m_streamDisplay.draw( sf::Sprite( m_disconnectTxtr.getTexture() ) );

		m_displayMutex.unlock();
		m_imageMutex.unlock();
	}

	m_displayMutex.lock();
	m_streamDisplay.display();
	m_displayMutex.unlock();
}

void MjpegStream::doneCallback( void* optarg ) {
	static_cast<MjpegStream*>(optarg)->stopStream();
}

void MjpegStream::readCallback( char* buf , int bufsize , void* optarg ) {
	// Create pointer to stream to make it easier to access the instance later
	MjpegStream* streamPtr = static_cast<MjpegStream*>( optarg );

	streamPtr->m_imageMutex.lock();

	std::cout << "Loading... ";

	// Load the image received
	bool loadedCorrectly = streamPtr->m_tempImage.loadFromMemory( static_cast<void*>(buf) , bufsize );
	if ( loadedCorrectly ) {
		loadedCorrectly = streamPtr->m_imageTxtr.loadFromImage( streamPtr->m_tempImage );
	}

	// If the image loaded successfully, update the sprite which displays it
	if ( loadedCorrectly ) {
		// If that was the first image streamed
		if ( streamPtr->m_firstImage ) {
			streamPtr->m_firstImage = false;
		}

		// Reset the image age timer
		streamPtr->m_imageAge.restart();
	}

	std::cout << "done && loaded=" << loadedCorrectly << "\n";

	streamPtr->m_imageMutex.unlock();
}

void MjpegStream::recreateTextures( const sf::Vector2u windowSize ) {
	/* ===== Recenter the messages ===== */
	m_connectMsg.setPosition( sf::Vector2f( ( windowSize.x - m_connectMsg.findCharacterPos( 100 ).x ) / 2.f ,
		( windowSize.y - m_connectMsg.getCharacterSize() - 6.f ) / 2.f ) );

	m_disconnectMsg.setPosition( sf::Vector2f( ( windowSize.x - m_disconnectMsg.findCharacterPos( 100 ).x ) / 2.f ,
		( windowSize.y - m_disconnectMsg.getCharacterSize() - 6.f ) / 2.f ) );

	m_waitMsg.setPosition( sf::Vector2f( ( windowSize.x - m_waitMsg.findCharacterPos( 100 ).x ) / 2.f ,
		( windowSize.y - m_waitMsg.getCharacterSize() - 6.f ) / 2.f ) );
	/* ================================= */

	/* ===== Fill RenderTextures with messages ===== */
	m_imageMutex.lock();

	m_connectTxtr.create( windowSize.x , windowSize.y );
	m_connectTxtr.clear( sf::Color( 40 , 40 , 40 ) );
	m_connectTxtr.draw( m_connectMsg );
	m_connectTxtr.display();

	m_disconnectTxtr.create( windowSize.x , windowSize.y );
	m_disconnectTxtr.clear( sf::Color( 40 , 40 , 40 ) );
	m_disconnectTxtr.draw( m_disconnectMsg );
	m_disconnectTxtr.display();

	m_waitTxtr.create( windowSize.x , windowSize.y );
	m_waitTxtr.clear( sf::Color( 40 , 40 , 40 , 0 ) );

	sf::RectangleShape waitBackground( sf::Vector2f( m_waitMsg.findCharacterPos( 100 ).x + 20.f , m_waitMsg.getCharacterSize() + 10.f ) );
	waitBackground.setPosition( sf::Vector2f( ( windowSize.x - waitBackground.getSize().x ) / 2.f ,
			( windowSize.y - waitBackground.getSize().y ) / 2.f ) );
	waitBackground.setFillColor( sf::Color( 120 , 120 , 120 , 70 ) );

	m_waitTxtr.draw( waitBackground );
	m_waitTxtr.draw( m_waitMsg );
	m_waitTxtr.display();

	m_imageMutex.unlock();
	/* ============================================= */
}
