//=============================================================================
//File Name: MjpegStream.hpp
//Description: Receives an MJPEG stream and displays it in a child window with
//             the specified properties
//Author: Tyler Veness
//=============================================================================

#ifndef MJPEG_STREAM_HPP
#define MJPEG_STREAM_HPP

/* This class creates a child window that receives MJPEG images and displays
 * them from a separate thread.
 *
 * To start using this class, just create an instance of it; everything else
 * is handled in a spawned thread
 *
 * Call startStream() to start the MJPEG stream or stopStream() to stop it
 * manually. This won't open or close the window.
 *
 * startStream() and stopStream() are called automatically in the constructor
 * and destructor respectively, but they can be called manually if desired.
 *
 * Change the button ID from IDC_STREAM_BUTTON to another ID if you want to
 * process more than one stream at once in WndProc
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../SFML/Graphics/Image.hpp"

#include "../SFML/System/Clock.hpp"
#include "../SFML/System/Mutex.hpp"
#include "../SFML/System/Thread.hpp"

#include "../WinGDI/Text.hpp"
#include "../WinGDI/Vector.hpp"

extern "C" {
#include "mjpegrx.h"
}

#define WM_MJPEGSTREAM_START     (WM_APP + 0x0001)
#define WM_MJPEGSTREAM_STOP      (WM_APP + 0x0002)
#define WM_MJPEGSTREAM_NEWIMAGE  (WM_APP + 0x0003)

class MjpegStream {
public:
    MjpegStream( const std::string& hostName ,
            unsigned short port ,
            const std::string& reqPath,
            HWND parentWin ,
            int xPosition ,
            int yPosition ,
            int width ,
            int height ,
            HINSTANCE appInstance
            );
    virtual ~MjpegStream();

    Vector2i getPosition();
    void setPosition( const Vector2i& position );

    Vector2i getSize();
    void setSize( const Vector2i& size );

    // Starts Kinect stream
    void startStream();

    // Stops Kinect stream
    void stopStream();

    // Returns true if streaming is on
    bool isStreaming();

    // Displays the stream or a message if the stream isn't working
    void display();

protected:
    static void doneCallback( void* optarg );
    static void readCallback( char* buf , int bufsize , void* optarg );

private:
    std::string m_hostName;
    unsigned short m_port;
    std::string m_requestPath;

    HWND m_parentWin;

    HWND m_streamWin;

    // Holds pointer to button which toggles streaming
    HWND m_toggleButton;

    // Contains "Connecting" message
    HDC m_connectDC;
    HBITMAP m_connectBmp;
    Text m_connectMsg;

    // Contains "Disconnected" message
    HDC m_disconnectDC;
    HBITMAP m_disconnectBmp;
    Text m_disconnectMsg;

    // Contains "Waiting..." message
    HDC m_waitDC;
    HBITMAP m_waitBmp;
    Text m_waitMsg;

    // Contains background color
    HDC m_backgroundDC;
    HBITMAP m_backgroundBmp;

    // Holds image most recently received from the host
    sf::Image m_tempImage;
    sf::Mutex m_imageMutex;

    // Stores image before displaying it on the screen
    HBITMAP m_imageBuffer;
    char* m_pxlBuf;

    /* Used to determine when to draw the "Connecting..." message
     * (when the stream first starts)
     */
    bool m_firstImage;

    // Used for streaming MJPEG frames from host
    struct mjpeg_callbacks_t m_callbacks;
    struct mjpeg_inst_t* m_streamInst;

    // Determines when a video frame is old
    sf::Clock m_imageAge;

    // Locks display so only one thread can access or draw to it at a time
    sf::Mutex m_displayMutex;

    /* If true:
     *     Lets receive thread run
     * If false:
     *     Closes receive thread
     */
    volatile bool m_stopReceive;

    /* Recreates the graphics that display messages in the stream window
     * (Resizes them and recenters the text in the window)
     */
    void recreateGraphics( const Vector2i& windowSize );
};

#endif // MJPEG_STREAM_HPP
