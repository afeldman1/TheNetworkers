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
#include <cstdint>
#include <string>
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
  typedef intptr_t SOCKET;
  #define INVALID_SOCKET  (SOCKET)(~0)
  typedef unsigned int UINT;
  #define closesocket(sock) close(sock)
  #define THREAD void*
#endif

struct Client{
	SOCKET sock;
	string name;

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
	int SendMessageTo(Client sClient, string MSG);
	int SendMessageAll(string MSG, Client aClient=Client());
	int RecClient(Client& sRecClient); // receive message for a particulat socket
	list<Client>::iterator FindClient(Client sRecClient);
	int Shutdown();
private:
	bool m_bIsConnected,closing; // true - connected false - not connected
	int m_iServerPort;
	list<Client> ClientList; // All clients
	SOCKET m_SClient;
	SOCKET unknownListenClient;
	SOCKET m_SListenClient; // socket listening for client calls
};
