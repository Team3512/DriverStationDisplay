////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2012 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

/* !!! THIS IS AN EXTREMELY ALTERED AND PURPOSE-BUILT VERSION OF SFML !!!
 * This distribution is designed to possess only a limited subset of the
 * original library's functionality and to only build on Win32 platforms.
 * The original distribution of this software has many more features and
 * supports more platforms.
 */

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../SFML/Network/Socket.hpp"
#include <iostream>
#include <cstring>


namespace sf
{
////////////////////////////////////////////////////////////
Socket::Socket(Type type) :
m_type      (type),
#ifdef _WIN32
m_socket    (INVALID_SOCKET),
#else
m_socket    (-1),
#endif
m_isBlocking(true)
{

}


////////////////////////////////////////////////////////////
Socket::~Socket()
{
    // Close the socket before it gets destructed
    close();
}


////////////////////////////////////////////////////////////
void Socket::setBlocking(bool blocking)
{
    mjpeg_sck_setnonblocking(m_socket, blocking ? 0 : 1);

    m_isBlocking = blocking;
}


////////////////////////////////////////////////////////////
bool Socket::isBlocking() const
{
    return m_isBlocking;
}


////////////////////////////////////////////////////////////
unsigned int Socket::getHandle() const
{
    return m_socket;
}


////////////////////////////////////////////////////////////
void Socket::create()
{
    // Don't create the socket if it already exists
    if (!mjpeg_sck_valid(m_socket))
    {
        mjpeg_socket_t handle = socket(PF_INET, m_type == Tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
        create(handle);
    }
}


////////////////////////////////////////////////////////////
void Socket::create(mjpeg_socket_t handle)
{
    // Don't create the socket if it already exists
    if (!mjpeg_sck_valid(m_socket))
    {
        // Assign the new handle
        m_socket = handle;

        // Set the current blocking state
        setBlocking(m_isBlocking);

        if (m_type == Tcp)
        {
            // Disable the Nagle algorithm (ie. removes buffering of TCP packets)
            int yes = 1;
            if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&yes), sizeof(yes)) == -1)
            {
                std::cerr << "Failed to set socket option \"TCP_NODELAY\" ; "
                      << "all your TCP packets will be buffered" << std::endl;
            }
        }
        else
        {
            // Enable broadcast by default for UDP sockets
            int yes = 1;
            if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&yes), sizeof(yes)) == -1)
            {
                std::cerr << "Failed to enable broadcast on UDP socket" << std::endl;
            }
        }
    }
}


////////////////////////////////////////////////////////////
void Socket::close()
{
    mjpeg_sck_close(m_socket);
}


////////////////////////////////////////////////////////////
sockaddr_in Socket::createAddress(uint32_t address, unsigned short port)
{
    sockaddr_in addr;
    std::memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);

    return addr;
}

////////////////////////////////////////////////////////////
Socket::Status Socket::getErrorStatus()
{
#ifdef _WIN32
    switch (WSAGetLastError())
    {
        case WSAEWOULDBLOCK :  return Socket::NotReady;
        case WSAECONNABORTED : return Socket::Disconnected;
        case WSAECONNRESET :   return Socket::Disconnected;
        case WSAETIMEDOUT :    return Socket::Disconnected;
        case WSAENETRESET :    return Socket::Disconnected;
        case WSAENOTCONN :     return Socket::Disconnected;
        default :              return Socket::Error;
    }
#else
    // The followings are sometimes equal to EWOULDBLOCK,
    // so we have to make a special case for them in order
    // to avoid having double values in the switch case
    if ((errno == EAGAIN) || (errno == EINPROGRESS))
        return Socket::NotReady;

    switch (errno)
    {
        case EWOULDBLOCK :  return Socket::NotReady;
        case ECONNABORTED : return Socket::Disconnected;
        case ECONNRESET :   return Socket::Disconnected;
        case ETIMEDOUT :    return Socket::Disconnected;
        case ENETRESET :    return Socket::Disconnected;
        case ENOTCONN :     return Socket::Disconnected;
        case EPIPE :        return Socket::Disconnected;
        default :           return Socket::Error;
    }
#endif
}

#ifdef _WIN32
struct SocketInitializer
{
    SocketInitializer()
    {
        WSADATA init;
        WSAStartup(MAKEWORD(2, 2), &init);
    }

    ~SocketInitializer()
    {
        WSACleanup();
    }
};

SocketInitializer globalInitializer;
#endif

} // namespace sf
