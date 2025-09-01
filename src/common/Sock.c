#pragma once

#include "../unity.h"  // IWYU pragma: keep

// @class Sock
// Function | Purpose
// --- | ---
// Sock__async(socket) | Set socket to non-blocking I/O mode
// Sock__noNagle(socket) | Disable Nagle's algorithm to prevent buffering
// Sock__close(socket) | Close a socket and update its state
// Sock__setup() | Perform one-time global socket initialization
// Sock__init(sock, addr, port, opts) | Initialize a socket with address, port, and options
// Sock__listen(socket) | Put a socket into listen mode for incoming connections
// Sock__accept(socket) | Accept a new connection socket from a listening socket
// Sock__connect(socket) | Connect the socket to a remote server
// Sock__read(socket, len) | Read up to len bytes from the socket
// Sock__write(socket, buf, len) | Write len bytes from buf to the socket
// Sock__shutdown(socket) | Shut down a non-listening socket
// Sock__free(socket) | Free socket resources
// Sock__destroy() | Perform global socket cleanup

// Set socket to non-blocking i/o mode
void Sock__async(Socket* socket) {
#ifdef __linux__
  fcntl(socket->_nix_socket, F_SETFL, O_NONBLOCK);
#endif

#ifdef _WIN32
  u_long mode = 1;
  ioctlsocket(socket->_win_socket, FIONBIO, &mode);
#endif
}

// Disable Nagle's algorithm (prevent buffering)
void Sock__noNagle(Socket* socket) {
  int noDelay = 1;

#ifdef __linux__
  ASSERT_CONTEXT(
      0 == setsockopt(socket->_nix_socket, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay)),
      "setsockopt TCP_NODELAY failed.");
#endif

#ifdef _WIN32
  ASSERT_CONTEXT(
      setsockopt(
          socket->_win_socket,
          IPPROTO_TCP,
          TCP_NODELAY,
          (const char*)&noDelay,
          sizeof(noDelay) != SOCKET_ERROR),
      "setsockopt failed with error: %d\n",
      WSAGetLastError());
#endif
}

// close a Socket
void Sock__close(Socket* socket) {
  if (SOCKET_CLOSED == socket->state)
    return;
  socket->state = SOCKET_CLOSED;
  socket->sessionState = SESSION_SERVER_HUNGUP;

  LOG_DEBUGF("Setting socket closed %s:%s", socket->addr, socket->port);

#ifdef __linux__
  close(socket->_nix_socket);
  socket->_nix_socket = 0;
#endif

#ifdef _WIN32
  closesocket(socket->_win_socket);
  socket->_win_socket = INVALID_SOCKET;
#endif

#ifdef __EMSCRIPTEN__
      int r = EM_ASM_INT(
          {
            try {
              var socket = Module.__libwebsocket.sockets.get($0);
              if (!socket) return -1;
    
              socket.close();
              return 1;
}
catch(e) {
  console.error('websocket close failed', e);
  return -1;
}
},
          socket->_web_socket);
if (1 == r)
  socket->_web_socket = 0;
#endif
}

// one-time global initialization
void Sock__setup() {
#ifdef _WIN32
  WSADATA wsaData;
  int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
  ASSERT_CONTEXT(0 == r, "WSAStartup failed with error: %d", r);
#endif

#ifdef __EMSCRIPTEN__
        EM_ASM_({
    var libwebsocket = {};
    libwebsocket.sockets = new Map();
    libwebsocket.on_event = Module.cwrap(
        'libwebsocket_cb',
        'number',
        [ 'number', 'number', 'number', 'number', 'number', 'number', 'number' ]);
    libwebsocket.connect = function(url, user_data, csock){try{var socket = new WebSocket(url);
    socket.binaryType = "arraybuffer";
    socket.user_data = user_data;

    socket.onopen = this.on_connect;
    socket.onmessage = this.on_message;
    socket.onclose = this.on_close;
    socket.onerror = this.on_error;
    socket.destroy = this.destroy;

    socket.id = this.sockets.size + 1;
    socket.csock = csock;
    this.sockets.set(socket.id, socket);
    return socket;
            } catch (e) {
    console.error("Socket creation failed:" + e);
    return 0;
            }
};
libwebsocket.on_connect = function() {
  var stack = stackSave();
  // WS_CLIENT_ESTABLISHED = 2
  ret = libwebsocket.on_event(this.protocol_id, this.csock, this.id, 2, this.user_data, 0, 0);
  stackRestore(stack);
};
libwebsocket.on_message = function(event) {
  var stack = stackSave();
  var len = event.data.byteLength;

  // Allocate memory on the stack for the received data
  var ptr = stackAlloc(len);

  var data = new Uint8Array(event.data);

  // Copy each byte from the WebSocket data to the allocated memory
  for (var i = 0, buf = ptr; i < len; ++i) {
    setValue(buf, data[i], 'i8');
    buf++;
  }

  // Call the C function with the pointer to the data buffer
  libwebsocket.on_event(this.protocol_id, this.csock, this.id, 4, this.user_data, ptr, len);

  stackRestore(stack);
};
libwebsocket.on_close = function() {
  // WS_CLOSED = 3
  libwebsocket.on_event(this.protocol_id, this.csock, this.id, 3, this.user_data, 0, 0);
  this.destroy();
};
libwebsocket.on_error = function() {
  // WS_CLIENT_CONNECTION_ERROR = 1
  libwebsocket.on_event(this.protocol_id, this.csock, this.id, 1, this.user_data, 0, 0);
  this.destroy();
};
libwebsocket.destroy = function() {
  libwebsocket.sockets.set(this.id, undefined);
  // WS_WSI_DESTROY = 5
  libwebsocket.on_event(this.protocol_id, this.csock, this.id, 5, this.user_data, 0, 0);
};

Module.__libwebsocket = libwebsocket;
});
#endif
}

// per-Socket initialization
void Sock__init(Socket* sock, char* addr, char* port, SocketOpts opts) {
  memcpy(sock->addr, addr, strlen(addr) + 1);
  memcpy(sock->port, port, strlen(port) + 1);

#ifdef __linux__
  // Create socket
  ASSERT_CONTEXT(
      (sock->_nix_socket = socket(AF_INET, SOCK_STREAM, 0)) >= 0,
      "Socket creation failed.");

  Sock__async(sock);
  Sock__noNagle(sock);

  // Set up the server address structure
  sock->_nix_addr.sin_family = AF_INET;
  sock->_nix_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all available interfaces
  sock->_nix_addr.sin_port = htons(atoi(port));  // Port to listen on

  // Convert IP address to binary form
  if (inet_pton(AF_INET, addr, &sock->_nix_addr.sin_addr) != 1) {
    LOG_DEBUGF("Socket connect invalid address or address not supported.");
    Sock__close(sock);
    return;
  }
#endif

#ifdef _WIN32
  sock->_win_socket = INVALID_SOCKET;
  struct addrinfo* result = NULL;
  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  if (opts & SERVER_SOCKET) {
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
  } else if (opts & CLIENT_SOCKET) {
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
  }

  // resolve the address and port strings
  int r = getaddrinfo(addr, port, &hints, &result);
  ASSERT_CONTEXT(0 == r, "getaddrinfo failed. error: %d, address: %s, port: %s", r, addr, port);

  sock->_win_addr = result;

  // Create a SOCKET
  sock->_win_socket = socket(
      sock->_win_addr->ai_family,
      sock->_win_addr->ai_socktype,
      sock->_win_addr->ai_protocol);
  ASSERT_CONTEXT(
      sock->_win_socket != INVALID_SOCKET,
      "socket create failed with error: %ld",
      WSAGetLastError());

  Sock__async(sock);
  Sock__noNagle(sock);
#endif
}

// put a Socket into listen mode
void Sock__listen(Socket* socket) {
#ifdef __linux__
  if (bind(socket->_nix_socket, (struct sockaddr*)&socket->_nix_addr, sizeof(socket->_nix_addr)) <
      0) {
    LOG_DEBUGF("Socket bind failed %s.", strerror(errno));
    Sock__close(socket);
    return;
  }

  if (listen(socket->_nix_socket, 3) < 0) {
    LOG_DEBUGF("Socket listen failed.");
    Sock__close(socket);
    return;
  }
#endif

#ifdef _WIN32
  // Setup the TCP listening socket
  int r = bind(socket->_win_socket, socket->_win_addr->ai_addr, (int)socket->_win_addr->ai_addrlen);
  ASSERT_CONTEXT(r != SOCKET_ERROR, "socket bind failed with error: %d", WSAGetLastError());

  // Begin listening
  r = listen(socket->_win_socket, SOMAXCONN);
  ASSERT_CONTEXT(r != SOCKET_ERROR, "socket listen failed with error: %d", WSAGetLastError());

  // Set the listen socket to non-blocking
  u_long mode = 1;  // 1 to enable non-blocking
  ioctlsocket(socket->_win_socket, FIONBIO, &mode);
#endif

#ifdef __EMSCRIPTEN__
// browser has no listen capability.
// server is not be hosted in wasm.
#endif
}

// accept one new connection Socket, forked from listening Socket
void Sock__accept(Socket* socket) {
  if (SOCKET_CLOSED == socket->state)
    return;
  Socket* csocket;

#ifdef __linux__
  socklen_t len = sizeof(socket->_nix_addr);
  int r = accept(socket->_nix_socket, (struct sockaddr*)&socket->_nix_addr, &len);
  if (r == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // no pending connections; its fine
      return;
    }
    LOG_DEBUGF("Socket accept failed. Will stop listening.");
    Sock__close(socket);  // stop listening
    return;
  }

  _G->onsockalloc(&csocket);
  csocket->_nix_socket = r;

  // Get the peer address information
  if (getpeername(csocket->_nix_socket, (struct sockaddr*)&csocket->_nix_addr, &len) == -1) {
    LOG_DEBUGF("Socket accept failed to getpeername.");
    return;
  }

  // Convert the IP address to a string
  if (NULL == inet_ntop(AF_INET, &csocket->_nix_addr.sin_addr, csocket->addr, INET_ADDRSTRLEN)) {
    LOG_DEBUGF("Socket accept failed to inet_ntop.");
    return;
  }

  // Get the port number
  int port = ntohs(csocket->_nix_addr.sin_port);
  sprintf(csocket->port, "%d", port);

  Sock__async(csocket);
  Sock__noNagle(csocket);
#endif

#ifdef _WIN32
  // Initialize the file descriptor set
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(socket->_win_socket, &readfds);

  // Set a zero timeout for non-blocking behavior
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;  // 0 microseconds, makes select() non-blocking

  // Check if there's a new incoming connection
  int r = select(0, &readfds, NULL, NULL, &timeout);
  if (!(r > 0 && FD_ISSET(socket->_win_socket, &readfds)))
    return;
  // Accept a connection from a new client socket
  struct sockaddr_in clientAddr;
  int clientAddrLen = sizeof(clientAddr);
  _G->onsockalloc(&csocket);
  csocket->_win_socket = accept(socket->_win_socket, (struct sockaddr*)&clientAddr, &clientAddrLen);
  ASSERT_CONTEXT(
      socket->_win_socket != INVALID_SOCKET,
      "socket accept failed. error: %d",
      WSAGetLastError());

  // Convert the IP address and port to C strings
  char ipStr[INET_ADDRSTRLEN], portStr[6];
  inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, sizeof(ipStr));
  snprintf(portStr, sizeof(portStr), "%d", ntohs(clientAddr.sin_port));
  u32 len = strlen(ipStr) + 1;
  memcpy(csocket->addr, ipStr, len);
  len = strlen(portStr) + 1;
  memcpy(csocket->port, portStr, len);

  // csocket->_win_addr = clientAddr; // TODO: must convert the type manually

  Sock__async(csocket);
  Sock__noNagle(csocket);
#endif

#ifdef __EMSCRIPTEN__
// browser has no listen capability.
// server is not be hosted in wasm.
#endif

  csocket->state = SOCKET_CONNECTED;
  _G->onsockaccept(socket, csocket);
}

void Sock__connect(Socket* socket) {
#ifdef __linux__
  // Connect to the external server
  int r =
      connect(socket->_nix_socket, (struct sockaddr*)&socket->_nix_addr, sizeof(socket->_nix_addr));
  if (r == -1) {
    if (errno == EINPROGRESS) {
      // connection in progress; it's fine
    } else {
      LOG_DEBUGF("Socket connect connection failed.");
      Sock__close(socket);
      return;
    }
  }

  // TODO: wait for connection async w/ select(), poll(), or epoll()
  socket->state = SOCKET_CONNECTED;
  _G->onsockconnect(socket);
#endif

#ifdef _WIN32
  // Connect to a server
  int r =
      connect(socket->_win_socket, socket->_win_addr->ai_addr, (int)socket->_win_addr->ai_addrlen);
  if (r == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAEWOULDBLOCK) {
      // everything is fine; conn is async
    } else {
      ASSERT_CONTEXT(r != SOCKET_ERROR, "socket connect failed. error: %d", WSAGetLastError());
    }
  }
  ASSERT_CONTEXT(
      socket->_win_socket != INVALID_SOCKET,
      "Unable to connect to server. error: %d",
      WSAGetLastError());

  // Set the incoming socket to non-blocking
  u_long mode = 1;  // 1 to enable non-blocking
  ioctlsocket(socket->_win_socket, FIONBIO, &mode);

  // TODO: wait for connection async
  socket->state = SOCKET_CONNECTED;
  _G->onsockconnect(socket);
#endif

#ifdef PROD_BUILD
  const char* proto = "wss";
#else
  const char* proto = "ws";
#endif

#ifdef __EMSCRIPTEN__
  int r2 = EM_ASM_INT(
      {
        var url =  //
            "" + UTF8ToString($0) +  // proto
            "://" + UTF8ToString($1) +  // host
            ":" + UTF8ToString($2) +  //port
            "/sock";  // uri
        var socket = Module.__libwebsocket.connect(url, $2, $3);
        if (!socket) {
          return 0;
        }

        return socket.id;
      },
      proto,
      socket->addr,
      socket->port,
      socket);
  ASSERT_CONTEXT(0 != r2, "socket connect failed.");
  socket->_web_socket = r2;
#endif
}

s8 Sock__read(Socket* socket, u32 len) {
  if (len < 1) {
    return -1;  // buffer full
  }
  if (SOCKET_CLOSED == socket->state)
    return -1;  // cannot write

  u8 buf[len];

#ifdef __linux__
  // Read data from the client socket
  int bytesRead = read(socket->_nix_socket, buf, len);
  if (bytesRead > 0) {
    _G->onsockrecv(socket, buf, bytesRead);
    return 1;  // successful read
  }
  if (-1 == bytesRead) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;  // read succeeded, despite no data in buffer
    } else {
      LOG_DEBUGF("Socket read failed.");
      Sock__close(socket);
      return -1;  // unexpected error
    }
  } else if (0 == bytesRead) {  // CLOSE_WAIT
    // remote side sent FIN and OS is waiting on app to close socket
    Sock__close(socket);
    return -1;  // cannot write
  }
#endif

#ifdef _WIN32
  // Initialize the file descriptor set
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(socket->_win_socket, &readfds);

  // Check if there is data to read from the client
  if (socket->_win_socket != INVALID_SOCKET && FD_ISSET(socket->_win_socket, &readfds)) {
    int bytesRead = recv(socket->_win_socket, (char*)buf, len, 0);
    if (NET_DEBUG_RAW && bytesRead > 0) {
      LOG_DEBUGF("requested to read %d got %d", len, bytesRead);
    }
    if (bytesRead > 0) {
      _G->onsockrecv(socket, buf, bytesRead);
      return 1;  //successful read
    }

    if (SOCKET_ERROR == bytesRead) {
      int err = WSAGetLastError();
      if (WSAEWOULDBLOCK == err) {
        return 0;  // nothing to read
      } else {  // WSAECONNRESET WSAENETRESET WSAETIMEDOUT WSAECONNREFUSED
        LOG_DEBUGF("socket recv failed. error: %d", err);
        Sock__close(socket);
        return -1;  // cannot write
      }
    } else if (0 == bytesRead) {  // CLOSE_WAIT
      // remote side sent FIN and OS is waiting on app to close socket
      Sock__close(socket);
      return -1;  // cannot write
    }
  }
#endif

#ifdef __EMSCRIPTEN__
// we don't pull from websocket in browser,
// instead it has an EventListener callback which pushes.
// see: libwebsocket_cb() WS_CLIENT_RECEIVE
#endif

  return -1;  // read failed (shouldn't reach here)
}

// writes literal bytes as-is; without framing/encoding
s8 Sock__write(Socket* socket, u8* buf, u32 len) {
  if (len < 1) {
    return -1;  // buffer empty
  }
  if (SOCKET_CLOSED == socket->state) {
    return -1;  // cannot write
  }

#ifdef __linux__
  int bytesWritten = send(socket->_nix_socket, buf, len, 0);
  if (bytesWritten == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // the OS outbound socket buffer is full
      LOG_DEBUGF("Socket write failed; OS reports outbound socket buffer full.");
      return -1;  // cannot write
    } else {
      LOG_DEBUGF("Socket write failed. errno: %d", errno);
      Sock__close(socket);
      return -1;  // cannot write
    }
  }
#endif

#ifdef _WIN32
  if (send(socket->_win_socket, (char*)buf, len, 0) == SOCKET_ERROR) {
    LOG_DEBUGF("socket send failed with error: %d", WSAGetLastError());
    Sock__close(socket);
  }
#endif

  // clang-format off
  #ifdef __EMSCRIPTEN__
  int r = EM_ASM_INT({
    try {
      var socket = Module.__libwebsocket.sockets.get($0);
      if (!socket) {
        console.error('websocket handle invalid', $0);
        return -1;
      }

      let data = new Uint8Array(Module.HEAPU8.buffer, $1, $2);
      socket.send(data);
      return 1;
    }
    catch(e) {
      console.error('websocket send failed', e);
      return -1;
    }
  },
  socket->_web_socket,
  buf,
  // NOTICE: valid types passed to EM_ASM are: int, float, double, string, and pointer (NOT u64)
  len);

  if (1 != r) {
    return r;
  }
  #endif
  
  _G->onsocksend(socket, buf, len);
  return 1;  // successful write
// clang-format on
}

// destructor for a [non-listening] socket
void Sock__shutdown(Socket* socket) {
  if (SOCKET_CLOSED == socket->state)
    return;
#ifdef _WIN32
  int r = shutdown(socket->_win_socket, SD_BOTH);  // stop sending and receiving
  ASSERT_CONTEXT(r != SOCKET_ERROR, "socket shutdown failed with error: %d", WSAGetLastError());
#endif
}

// global free
void Sock__free(Socket* socket) {
#ifdef _WIN32
  freeaddrinfo(socket->_win_addr);
#endif
}

// global destructor
void Sock__destroy() {
#ifdef _WIN32
  WSACleanup();
#endif
}

// -- Websockets --
#ifdef __EMSCRIPTEN__
#ifdef __cplusplus
extern "C" {
#endif
int EMSCRIPTEN_KEEPALIVE libwebsocket_cb(
    int protocol,
    Socket* socket,
    int socketId,
    int callbackFnId,
    void* websocket,
    void* data,
    size_t len) {
  // LOG_DEBUGF(
  //     "JS called on_event() -> libwebsocket_cb(). params: "
  //     "protocol %d, "
  //     "context %p, "
  //     "socketId %d, "
  //     "callbackFnId %d, "
  //     "socket %p, "
  //     "data %p, "
  //     "len %d",
  //     protocol,
  //     socket,
  //     socketId,
  //     callbackFnId,
  //     websocket,
  //     data,
  //     len);

  if (WS_CLIENT_ESTABLISHED == callbackFnId) {
    socket->state = SOCKET_CONNECTED;
    socket->sessionState = SESSION_CLIENT_WASM_CONNECT_CB;
  }

  else if (WS_CLIENT_RECEIVE == callbackFnId) {
    _G->onsockrecv(socket, data, len);
  }

  // if (reason == WS_WSI_DESTROY) {
  //   context->protocols[protocol].callback(context, wsi, reason, user, data, len);
  //   // TODO See if we need to destroy the user_data..currently we dont allocate it, so we never would need to free it.
  //   return 0;
  // } else {
  //   return context->protocols[protocol].callback(context, wsi, reason, user, data, len);
  // }

  return 0;
}
#ifdef __cplusplus
}
#endif
#endif