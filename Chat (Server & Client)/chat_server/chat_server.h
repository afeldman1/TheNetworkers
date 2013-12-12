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

struct Client{
	SOCKET sock;
	std::string name;

	Client(){
		sock = NULL;
		name = "";
	}

	bool operator==(const Client& a){
		return (this->name==a.name && this->sock==a.sock);
	}
};

class CChatServer
{
public:
	CChatServer();
	~CChatServer();
	bool IsConnected(){return m_bIsConnected;} // returns connection status
	void StartListenClient(); // Listen to client
	int SendMessageTo(Client sClient, std::string MSG);
	int SendMessageAll(std::string MSG, Client aClient=Client());
	bool RecClient(Client& sRecClient); // receive message for a particulat socket
	std::list<Client>::iterator FindClient(Client sRecClient);
	int Shutdown();
private:
	bool m_bIsConnected,closing; // true - connected false - not connected
	int m_iServerPort;
	std::list<Client> ClientList; // All clients
	SOCKET m_SClient;
	SOCKET unknownListenClient;
	SOCKET m_SListenClient; // socket Listening for client calls
};
