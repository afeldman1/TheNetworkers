//************************************************************************
// Boby Thomas Pazheparampil
// May 2006
// Implementation of CIPMessage class and main.
//************************************************************************

#ifdef WIN32
  #include <Afxwin.h>
  #include <winsock2.h>        // For socket(), connect(), send(), and recv()
#else
  #include <pthread.h>
  #include <sys/types.h>       // For data types
  #include <sys/socket.h>      // For socket(), connect(), send(), and recv()
  #include <netdb.h>           // For gethostbyname()
  #include <arpa/inet.h>       // For inet_addr()
  #include <unistd.h>          // For close()
  #include <netinet/in.h>      // For sockaddr_in
  typedef void *LPVOID;
#endif

#include <stdio.h>
#include <conio.h> 
#include <iostream>
using namespace std;


class CIPMessage
{
public:
	CIPMessage();
	~CIPMessage();
	void Init(string sIpAddress, int iPort);
	int SendMessagePort(string sMessage);
	int RecMessagePort();
	bool IsConnected(){return m_bIsConnected;}
private:
	bool m_bIsConnected; // true - connected false - not connected
	string m_sServerIPAddress;
	int m_iServerPort;
	SOCKET conn; // socket connected to server
};
