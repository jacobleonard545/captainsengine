//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: clientsocket.hh                                                 |
//  ==================================================================

#ifndef CLIENTSOCKET_HH
#define CLIENTSOCKET_HH

#include <Engine/system/socket.h>

using std::string;

//! client side socket
/*! provides simple socket operations
 */
class ClientSocket : private CGE::Socket
{
 public:
  //! Creates a socket and connects it
  /*! \param host The hostname (can be specified as IP-address or as URL)
   *  \param port The port to connect on host
   */
  ClientSocket(std::string host, int port);
  //! Destructor
  virtual ~ClientSocket();
  //! write string to socket
  const ClientSocket& operator << ( const string& ) const;
  //! read string from socket
  const ClientSocket& operator >> ( string& ) const;
  //! write integer to socket
  const ClientSocket& operator << ( const int&) const;
};


#endif
