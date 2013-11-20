//************************************************************************
// Boby Thomas Pazheparampil
// May 2006
// Implementation of CIPMessage class and main.
//************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <list>
#include <iostream>
#include <limits>       // std::numeric_limits
using namespace std;

#ifdef WIN32
  #define NOMINMAX
  #include <winsock2.h>			// For socket(), connect(), send(), and recv()
  typedef int32_t socklen_t;	// Or use int (This function, unlike in SOCKET programming [uint], is int)
  #define THREAD DWORD WINAPI	// Requirements for running a function in a separate thread in WIN32.
#else
  #include <pthread.h>
  #include <sys/types.h>       // For data types
  #include <sys/socket.h>      // For socket(), connect(), send(), and recv()
  #include <netdb.h>           // For gethostbyname()
  #include <arpa/inet.h>       // For inet_addr()
  #include <unistd.h>          // For close()
  #include <netinet/in.h>      // For sockaddr_in
  typedef void* LPVOID;
  typedef int SOCKET;
  #define INVALID_SOCKET  (SOCKET)(~0)
  typedef unsigned int UINT;
  #define closesocket(sock) close(sock)
  #define THREAD UINT
#endif

class CChatServer
{
public:
	CChatServer();
	~CChatServer();
	bool IsConnected(){return m_bIsConnected;} // returns connection status
	void StartListenClient(); // Listen to client
	int SendMessagePort(string sMessage); // Send message to sll clients.
	int RecClient(SOCKET sRecSocket); // receive message for a particulat socket
private:
	bool m_bIsConnected; // true - connected false - not connected
	int m_iServerPort;
	list<SOCKET> m_vClientList; // All socket connected to client
	SOCKET m_SClient;
	SOCKET unknownListenClient;
	SOCKET m_SListenClient; // socket listening for client calls
};
