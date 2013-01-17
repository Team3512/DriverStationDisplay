//=============================================================================
//File Name: MjpegStream.cpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#include "../WinGDI/UIFont.hpp"
#include "MjpegStream.hpp"
#include "../Resource.h"

#include <sstream>
#include <wingdi.h>
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
        m_connectDC( NULL ) ,
        m_connectMsg( Vector2i( 0 , 0 ) , UIFont::getInstance()->segoeUI18() , RGB( 255 , 255 , 255 ) , RGB( 40 , 40 , 40 ) ) ,
        m_disconnectDC( NULL ) ,
        m_disconnectMsg( Vector2i( 0 , 0 ) , UIFont::getInstance()->segoeUI18() , RGB( 255 , 255 , 255 ) , RGB( 40 , 40 , 40 ) ) ,
        m_waitDC( NULL ) ,
        m_waitMsg( Vector2i( 0 , 0 ) , UIFont::getInstance()->segoeUI18() , RGB( 255 , 255 , 255 ) , RGB( 40 , 40 , 40 ) ) ,

        m_pxlBuf( NULL ) ,

        m_firstImage( true ) ,

        m_streamInst( NULL ) ,

        m_stopReceive( true ) {

    // Set correct text for messages
    m_connectMsg.setString( L"Connecting..." );
    m_disconnectMsg.setString( L"Disconnected" );
    m_waitMsg.setString( L"Waiting..." );

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

    /* ===== Initialize the stream toggle button ===== */
    HGDIOBJ hfDefault = GetStockObject( DEFAULT_GUI_FONT );

    m_toggleButton = CreateWindowEx( 0,
        "BUTTON",
        "Start Stream",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        xPosition ,
        yPosition + height + 5,
        100,
        28,
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
    recreateGraphics( Vector2i( width , height ) );

    // Set up the callback description structure
    ZeroMemory( &m_callbacks , sizeof(struct mjpeg_callbacks_t) );
    m_callbacks.readcallback = readCallback;
    m_callbacks.donecallback = doneCallback;
    m_callbacks.optarg = this;
}

MjpegStream::~MjpegStream() {
    stopStream();
    DeleteObject( m_imageBuffer );

    DeleteDC( m_connectDC );
    DeleteDC( m_disconnectDC );
    DeleteDC( m_waitDC );
    DeleteDC( m_backgroundDC );

    DestroyWindow( m_streamWin );
    DestroyWindow( m_toggleButton );
}

Vector2i MjpegStream::getPosition() {
    RECT windowPos;

    m_displayMutex.lock();
    GetWindowRect( m_streamWin , &windowPos );
    m_displayMutex.unlock();

    return Vector2i( windowPos.left , windowPos.top );
}

void MjpegStream::setPosition( const Vector2i& position ) {
    m_displayMutex.lock();

    // Set position of stream window
    SetWindowPos( m_streamWin , NULL , position.X , position.Y , getSize().X , getSize().Y , SWP_NOZORDER );

    // Set position of stream button below it
    SetWindowPos( m_toggleButton , NULL , position.X , position.Y + 240 + 5 , 100 , 24 , SWP_NOZORDER );

    m_displayMutex.unlock();
}

Vector2i MjpegStream::getSize() {
    RECT windowPos;

    m_displayMutex.lock();
    GetClientRect( m_streamWin , &windowPos );
    m_displayMutex.unlock();

    return Vector2i( windowPos.right , windowPos.bottom );
}

void MjpegStream::setSize( const Vector2i& size ) {
    m_displayMutex.lock();
    SetWindowPos( m_streamWin , NULL , getPosition().X , getPosition().Y , size.X , size.Y , SWP_NOZORDER );
    m_displayMutex.unlock();

    recreateGraphics( size );
}

void MjpegStream::startStream() {
    if ( m_stopReceive == true ) { // if stream is closed, reopen it
        m_stopReceive = false;

        m_imageAge.restart();

        // Launch the MJPEG receiving/processing thread
        m_streamInst = mjpeg_launchthread( const_cast<char*>( m_hostName.c_str() ) , m_port , &m_callbacks );

        // Send message to parent window about the stream opening
        PostMessage( m_parentWin , WM_MJPEGSTREAM_START , 0 , 0 );
    }
}

void MjpegStream::stopStream() {
    if ( m_stopReceive == false ) { // if stream is open, close it
        m_stopReceive = true;

        // Close the receive thread
        if ( m_streamInst != NULL ) {
            mjpeg_stopthread( m_streamInst );
            // Send message to parent window about the stream closing
            PostMessage( m_parentWin , WM_MJPEGSTREAM_STOP , 0 , 0 );
        }
    }
}

bool MjpegStream::isStreaming() {
    return !m_stopReceive;
}

void MjpegStream::display() {
    // Get DC of window to which to draw
    HDC windowDC = GetDC( m_streamWin );

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

            // Display connect graphic from another device context
            BitBlt( windowDC , 0 , 0 , getSize().X , getSize().Y , m_connectDC , 0 , 0 , SRCCOPY );

            m_displayMutex.unlock();
            m_imageMutex.unlock();
        }

        // If it's been too long since we received our last image
        else if ( sImageAge > 1000 ) {
            // Display "Waiting..." over the last image received
            m_imageMutex.lock();
            m_displayMutex.lock();

            // Display connect graphic from another device context
            BitBlt( windowDC , 0 , 0 , getSize().X , getSize().Y , m_backgroundDC , 0 , 0 , SRCCOPY );
            BitBlt( windowDC , 0 , 0 , getSize().X , getSize().Y , m_waitDC , 0 , 0 , SRCPAINT );

            m_displayMutex.unlock();
            m_imageMutex.unlock();
        }

        // Else display the image last received
        else {
            m_imageMutex.lock();
            m_displayMutex.lock();

            // Create offscreen DC for image to go on
            HDC imageHdc = CreateCompatibleDC( windowDC );

            // Put the image into the offscreen DC and save the old one
            HBITMAP imageBackup = static_cast<HBITMAP>( SelectObject( imageHdc , m_imageBuffer ) );

            // Copy image from offscreen DC to window's DC
            BitBlt( windowDC , 0 , 0 , getSize().X , getSize().Y , imageHdc , 0 , 0 , SRCCOPY );

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

        // Display connect graphic from another device context
        SetBkColor( windowDC , RGB( 40 , 40 , 40 ) );
        //BitBlt( windowDC , 0 , 0 , getSize().X , getSize().Y , m_backgroundDC , 0 , 0 , SRCCOPY );
        BitBlt( windowDC , 0 , 0 , getSize().X , getSize().Y , m_disconnectDC , 0 , 0 , SRCPAINT );

        m_displayMutex.unlock();
        m_imageMutex.unlock();
    }

    // Release window's device context
    ReleaseDC( m_streamWin , windowDC );

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
        streamPtr->m_pxlBuf = static_cast<char*>( malloc( streamPtr->getSize().X * streamPtr->getSize().Y * 4 ) );
        std::memcpy( streamPtr->m_pxlBuf , streamPtr->m_tempImage.getPixelsPtr() , streamPtr->getSize().X * streamPtr->getSize().Y * 4 );

        char blueTemp;
        char redTemp;

        for ( int i = 0 ; i < streamPtr->getSize().X * streamPtr->getSize().Y * 4 ; i += 4 ) {
            redTemp = streamPtr->m_pxlBuf[i];
            blueTemp = streamPtr->m_pxlBuf[i+2];
            streamPtr->m_pxlBuf[i] = blueTemp;
            streamPtr->m_pxlBuf[i+2] = redTemp;
        }
        /* ============================================================ */

        // Make HBITMAP from pixel array
        DeleteObject( streamPtr->m_imageBuffer ); // free previous image if there is one
        streamPtr->m_imageBuffer = CreateBitmap( streamPtr->getSize().X , streamPtr->getSize().Y , 1 , 32 , streamPtr->m_pxlBuf );

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

        // Send message to parent window about the new image
        PostMessage( streamPtr->m_parentWin , WM_MJPEGSTREAM_NEWIMAGE , 0 , 0 );
    }

    streamPtr->m_imageMutex.unlock();
}

void MjpegStream::recreateGraphics( const Vector2i& windowSize ) {
    /* ===== Free all DC's so we can make new ones of the correct size ===== */
    DeleteObject( m_connectBmp );
    m_connectBmp = NULL;
    DeleteDC( m_connectDC );
    m_connectDC = NULL;

    DeleteObject( m_disconnectBmp );
    m_disconnectBmp = NULL;
    DeleteDC( m_disconnectDC );
    m_disconnectDC = NULL;

    DeleteObject( m_waitBmp );
    m_waitBmp = NULL;
    DeleteDC( m_waitDC );
    m_waitDC = NULL;

    DeleteObject( m_backgroundBmp );
    m_backgroundBmp = NULL;
    DeleteDC( m_backgroundDC );
    m_backgroundDC = NULL;
    /* ===================================================================== */

    // Create new device contexts
    HDC streamWinDC = GetDC( m_streamWin );
    m_connectDC = CreateCompatibleDC( streamWinDC );
    m_disconnectDC = CreateCompatibleDC( streamWinDC );
    m_waitDC = CreateCompatibleDC( streamWinDC );
    m_backgroundDC = CreateCompatibleDC( streamWinDC );

    // Create a 1:1 relationship between logical units and pixels
    SetMapMode( m_connectDC , MM_TEXT );
    SetMapMode( m_disconnectDC , MM_TEXT );
    SetMapMode( m_waitDC , MM_TEXT );
    SetMapMode( m_backgroundDC , MM_TEXT );

    // Create the bitmaps used for graphics
    m_connectBmp = CreateCompatibleBitmap( streamWinDC , getSize().X , getSize().Y );
    m_disconnectBmp = CreateCompatibleBitmap( streamWinDC , getSize().X , getSize().Y );
    m_waitBmp = CreateCompatibleBitmap( streamWinDC , getSize().X , getSize().Y );
    m_backgroundBmp = CreateCompatibleBitmap( streamWinDC , getSize().X , getSize().Y );

    ReleaseDC( m_streamWin , streamWinDC );

    // Give each graphic a bitmap to use
    SelectObject( m_connectDC , m_connectBmp );
    SelectObject( m_disconnectDC , m_disconnectBmp );
    SelectObject( m_waitDC , m_waitBmp );
    SelectObject( m_backgroundDC , m_backgroundBmp );

    // Create brushes and regions for backgrounds
    HBRUSH backgroundBrush = CreateSolidBrush( RGB( 40 , 40 , 40 ) );
    HRGN backgroundRegion = CreateRectRgn( 0 , 0 , windowSize.X , windowSize.Y );

    HBRUSH transparentBrush = CreateSolidBrush( RGB( 0 , 0 , 0 ) );

    HBRUSH waitBrush = CreateSolidBrush( RGB( 128 , 128 , 128 ) );
    HRGN waitRegion = CreateRectRgn( windowSize.X / 3 , windowSize.Y / 3 , 2 * windowSize.X / 3 , 2 * windowSize.Y / 3 );

    /* ===== Fill graphics with a background color ===== */
    FillRgn( m_connectDC , backgroundRegion , backgroundBrush );

    // Need a special background color since they will be transparent
    FillRgn( m_disconnectDC , backgroundRegion , transparentBrush );
    FillRgn( m_waitDC , backgroundRegion , transparentBrush );

    // Add transparent rectangle
    FillRgn( m_disconnectDC , waitRegion , waitBrush );
    //FillRgn( m_waitDC , waitRegion , waitBrush );

    // Create background
    FillRgn( m_backgroundDC , backgroundRegion , backgroundBrush );
    /* ================================================= */

    // Free the brushes and regions
    DeleteObject( backgroundBrush );
    DeleteObject( backgroundRegion );
    DeleteObject( waitRegion );
    DeleteObject( waitBrush );

    /* ===== Recenter the messages ===== */
    m_connectMsg.setPosition( Vector2i( windowSize.X / 2.f ,
            windowSize.Y / 2.f ) );

    m_disconnectMsg.setPosition( Vector2i( windowSize.X / 2.f ,
            windowSize.Y / 2.f ) );

    m_waitMsg.setPosition( Vector2i( windowSize.X / 2.f ,
            windowSize.Y / 2.f ) );
    /* ================================= */

    /* ===== Fill device contexts with messages ===== */
    int oldBkMode;
    UINT oldAlign;
    m_imageMutex.lock();

    oldBkMode = SetBkMode( m_connectDC , TRANSPARENT );
    oldAlign = SetTextAlign( m_connectDC , TA_CENTER | TA_BASELINE );
    m_connectMsg.draw( m_connectDC );
    SetTextAlign( m_connectDC , oldAlign );
    SetBkMode( m_connectDC , oldBkMode );

    oldBkMode = SetBkMode( m_disconnectDC , TRANSPARENT );
    oldAlign = SetTextAlign( m_disconnectDC , TA_CENTER | TA_BASELINE );
    m_disconnectMsg.draw( m_disconnectDC );
    SetTextAlign( m_disconnectDC , oldAlign );
    SetBkMode( m_disconnectDC , oldBkMode );

    oldBkMode = SetBkMode( m_waitDC , TRANSPARENT );
    oldAlign = SetTextAlign( m_waitDC , TA_CENTER | TA_BASELINE );
    m_waitMsg.draw( m_waitDC );
    SetTextAlign( m_waitDC , oldAlign );
    SetBkMode( m_waitDC , oldBkMode );

    m_imageMutex.unlock();
    /* ============================================== */
}
