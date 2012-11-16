//=============================================================================
//File Name: MjpegStream.cpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "../UIFont.hpp"
#include "MjpegStream.hpp"
#include "../ButtonID.hpp"

#include <sstream>
#include <gdiplus.h>
#include <cstring>

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
        xPosition ,
        yPosition + height + 5,
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

    m_imageBuffer = NULL;
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

    // Set position of stream window
    m_streamDisplay.setPosition( position );

    // Set position of stream button below it
    SetWindowPos( m_toggleButton , NULL , position.x , position.y + 240 + 5 , 100 , 24 , SWP_NOZORDER );

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

        m_imageAge.restart();

        // Launch the MJPEG receiving/processing thread
        m_streamInst = mjpeg_launchthread( const_cast<char*>( m_hostName.c_str() ) , m_port , &m_callbacks );
    }
}

void MjpegStream::stopStream() {
    if ( m_stopReceive == false ) { // if stream is open, close it
        m_stopReceive = true;

        // Close the receive thread
        if ( m_streamInst != NULL ) {
            mjpeg_stopthread( m_streamInst );
        }
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

            m_streamDisplay.clear( sf::Color( 0 , 0 , 0 , 0 ) );
            m_streamDisplay.draw( sf::Sprite( m_waitTxtr.getTexture() ) );

            m_displayMutex.unlock();
            m_imageMutex.unlock();
        }

        // Else display the image last received
        else {
            m_imageMutex.lock();
            m_displayMutex.lock();

            // Create offscreen DC for image to go on
            HDC imageHdc = CreateCompatibleDC( NULL );

            // Put the image into the offscreen DC and save the old one
            HBITMAP imageBackup = static_cast<HBITMAP>( SelectObject( imageHdc , m_imageBuffer ) );

            // Get DC of window to which to draw
            HDC windowHdc = GetDC( m_streamWin );

            // Load image to real BITMAP just to retrieve its dimensions
            BITMAP tempBMP;
            GetObject( m_imageBuffer , sizeof( BITMAP ) , &tempBMP );

            // Copy image from offscreen DC to window's DC
            BitBlt( windowHdc , 0 , 0 , tempBMP.bmWidth , tempBMP.bmHeight , imageHdc , 0 , 0 , SRCCOPY );

            // Release window's HDC
            ReleaseDC( m_streamWin , windowHdc );

            // Restore old image
            SelectObject( imageHdc , imageBackup );

            // Delete offscreen DC
            DeleteDC( imageHdc );

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

    m_imageMutex.lock();
    m_displayMutex.lock();

    if ( m_firstImage || m_imageAge.getElapsedTime().asMilliseconds() > 1000 ) {
        m_streamDisplay.display();
    }

    m_displayMutex.unlock();
    m_imageMutex.unlock();

    char* buttonText = static_cast<char*>( std::malloc( 13 ) );
    GetWindowText( m_toggleButton , buttonText , 13 );

    // If running and button displays "Start"
    if ( !m_stopReceive && std::strcmp( buttonText , "Start Stream" ) == 0 ) {
        // Change text displayed on button (LParam is button HWND)
        SendMessage( m_toggleButton , WM_SETTEXT , 0 , reinterpret_cast<LPARAM>("Stop Stream") );
    }

    // If not running and button displays "Stop"
    else if ( m_stopReceive && std::strcmp( buttonText , "Stop Stream" ) == 0 ) {
        // Change text displayed on button (LParam is button HWND)
        SendMessage( m_toggleButton , WM_SETTEXT , 0 , reinterpret_cast<LPARAM>("Start Stream") );
    }

    std::free( buttonText );
}

void MjpegStream::doneCallback( void* optarg ) {
    static_cast<MjpegStream*>(optarg)->m_stopReceive = true;
    static_cast<MjpegStream*>(optarg)->m_firstImage = true;

    static_cast<MjpegStream*>(optarg)->m_streamInst = NULL;
}

void MjpegStream::readCallback( char* buf , int bufsize , void* optarg ) {
    // Create pointer to stream to make it easier to access the instance later
    MjpegStream* streamPtr = static_cast<MjpegStream*>( optarg );

    // Holds pixel data for decompressed and decoded JPEG
    streamPtr->m_imageMutex.lock();

    // Load the image received (converts from JPEG to pixel array)
    bool loadedCorrectly = streamPtr->m_tempImage.loadFromMemory( buf , bufsize );

    if ( loadedCorrectly ) {
        /* ===== Reverse RGBA to BGRA before displaying the image ===== */
        /* Swap R and B because Win32 expects the color components in the
         * opposite order they are currently in
         */
        streamPtr->m_pxlBuf = static_cast<char*>( malloc( streamPtr->m_tempImage.getSize().x * streamPtr->m_tempImage.getSize().y * 4 ) );
        std::memcpy( streamPtr->m_pxlBuf , streamPtr->m_tempImage.getPixelsPtr() , streamPtr->m_tempImage.getSize().x * streamPtr->m_tempImage.getSize().y * 4 );

        char blueTemp;
        char redTemp;

        for ( unsigned int i = 0 ; i < streamPtr->m_tempImage.getSize().x * streamPtr->m_tempImage.getSize().y * 4 ; i += 4 ) {
            redTemp = streamPtr->m_pxlBuf[i];
            blueTemp = streamPtr->m_pxlBuf[i+2];
            streamPtr->m_pxlBuf[i] = blueTemp;
            streamPtr->m_pxlBuf[i+2] = redTemp;
        }
        /* ============================================================ */

        // Make HBITMAP from pixel array
        streamPtr->m_imageBuffer = CreateBitmap( streamPtr->m_tempImage.getSize().x , streamPtr->m_tempImage.getSize().y , 1 , 32 , streamPtr->m_pxlBuf );

        std::free( streamPtr->m_pxlBuf );
	}

    // If HBITMAP was created successfully, set a flag and reset the timer
    if ( streamPtr->m_imageBuffer != NULL  ) {
        // If that was the first image streamed
        if ( streamPtr->m_firstImage ) {
            streamPtr->m_firstImage = false;
        }

        // Reset the image age timer
        streamPtr->m_imageAge.restart();
    }

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
    m_waitTxtr.clear( sf::Color( 40 , 40 , 40 , 0 ) ); // fully transparent

    sf::RectangleShape waitBackground( sf::Vector2f( m_waitMsg.findCharacterPos( 100 ).x + 20.f ,
            m_waitMsg.getCharacterSize() + 10.f ) );
    waitBackground.setPosition( sf::Vector2f( ( windowSize.x - waitBackground.getSize().x ) / 2.f ,
            ( windowSize.y - waitBackground.getSize().y ) / 2.f ) );
    waitBackground.setFillColor( sf::Color( 120 , 120 , 120 , 70 ) );

    m_waitTxtr.draw( waitBackground );
    m_waitTxtr.draw( m_waitMsg );
    m_waitTxtr.display();

    m_imageMutex.unlock();
    /* ============================================= */
}
