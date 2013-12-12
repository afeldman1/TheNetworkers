//************************************************************************
// The Networkers
// Fall Semester 2013
// Based on: http://www.codeproject.com/Articles/14032/Chat-Client-Server
//************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <list>
#include <iostream>
#include <limits>       // std::numeric_limits
#include <string>

#ifdef WIN32
  #include <cstdint>
  #define NOMINMAX
  #include <winsock2.h>			// For socket(), connect(), send(), and recv()
  typedef int32_t socklen_t;	// Or use int (This function, unlike in SOCKET programming [uint], is int)
  #define THREAD DWORD WINAPI	// Requirements for running a function in a separate thread in WIN32.
#else
  #include <stdint.h>
  #include <cstring>
  using std::strlen;
  #include <pthread.h>
  #include <sys/types.h>       // For data types
  #include <sys/socket.h>      // For socket(), connect(), send(), and recv()
  #include <netdb.h>           // For gethostbyname()
  #include <arpa/inet.h>       // For inet_addr()
  #include <unistd.h>          // For close()
  #include <netinet/in.h>      // For sockaddr_in
  typedef void* LPVOID;
  typedef intptr_t SOCKET;
  #define INVALID_SOCKET  (SOCKET)(~0)
  typedef unsigned int UINT;
  #define closesocket(sock) close(sock)
  #define THREAD void*
  #define SOCKET_ERROR -1
  #define SD_BOTH 0x02
#endif


class CIPMessage
{
public:
	CIPMessage();
	~CIPMessage();
	void Init(std::string sIpAddress, int iPort);
	int SendMessagePort(std::string sMessage);
	bool RecMessagePort();
	bool IsConnected(){return m_bIsConnected;}
	bool closing;
private:
	bool m_bIsConnected; // true - connected false - not connected
	std::string m_sServerIPAddress;
	int m_iServerPort;
	SOCKET conn; // socket connected to server
};
