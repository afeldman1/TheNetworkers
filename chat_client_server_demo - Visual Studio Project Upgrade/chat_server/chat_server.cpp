//************************************************************************
// Implementation of CIPMessage class and main.
//************************************************************************

#include "chat_server.h"


CChatServer CServerObj;

THREAD ServerRecThread(LPVOID pParam)
{	
	SOCKET sRecSocket = reinterpret_cast<SOCKET>(pParam);
	while(1)
	{
		if(CServerObj.RecClient(sRecSocket))
			break;
	}
	return 0;
}

THREAD ServerListenThread(LPVOID pParam)
{	
	while(1)
		CServerObj.StartListenClient();
	return 0;
}



THREAD LookoutThread(LPVOID pParam)
{
	SOCKET sRecSocket = reinterpret_cast<SOCKET>(pParam);
	while(1)
	{//TO BE WRITTEN
		//if(CServerObj.RecClient(sRecSocket))
			//break;
	}
	return 0;
}



CChatServer::CChatServer()
{
    cout << "Starting up TCP Chat server\n";
	m_bIsConnected = false;

	#if defined(WIN32)
	  WSADATA wsaData;
	  int wsaret=WSAStartup(0x101,&wsaData);
	  if(wsaret!=0)
		  return;
	#endif

	sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_addr.s_addr=INADDR_ANY;
    local.sin_port=htons((u_short)8084);

	sockaddr_in unknown;
    unknown.sin_family=AF_INET;
    unknown.sin_addr.s_addr=INADDR_ANY;
    unknown.sin_port=htons((u_short)8085);

    m_SListenClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	unknownListenClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


    if(m_SListenClient == INVALID_SOCKET || unknownListenClient == INVALID_SOCKET)
    {
        return;
    }


    if(bind(m_SListenClient, (sockaddr*)&local, sizeof(local)) != 0 || bind(unknownListenClient, (sockaddr*)&unknown, sizeof(unknown)) != 0)
    {
        return;
    }

	#if defined(WIN32)
	  HANDLE tID;
	  tID = CreateThread(NULL, 0, LookoutThread, reinterpret_cast<void*>(unknownListenClient), 0, NULL);  
	#elif defined(__APPLE__)
	  pthread_t *tID;
	  pthread_create(tID, NULL, LookoutThread, reinterpret_cast<void*>(unknownListenClient));
	#endif

    if(listen(m_SListenClient,10) != 0)
    {
        return;
    }

	m_bIsConnected = true;
    return;
}

CChatServer::~CChatServer()
{
    closesocket(m_SListenClient);

	#if defined(WIN32)
	  WSACleanup();
	#endif
}

void CChatServer::StartListenClient()
{

    sockaddr_in from;
    socklen_t fromlen=sizeof(from);

    m_SClient=accept(m_SListenClient,(struct sockaddr*)&from,&fromlen);

	if(m_SClient != INVALID_SOCKET)
		m_vClientList.push_back(m_SClient);

	#if defined(WIN32)
	  HANDLE tID;
	  tID = CreateThread(NULL, 0, ServerRecThread, reinterpret_cast<void*>(m_SClient), 0, NULL);  
	#elif defined(__APPLE__)
	  pthread_t *tID;
	  pthread_create(tID, NULL, ServerRecThread, reinterpret_cast<void*>(m_SClient));
	#endif
}



int CChatServer::SendMessagePort(string sMessage)
{
		int iStat = 0;
		list<SOCKET>::iterator itl;

		if(m_vClientList.size() == 0)
			return 0;

		for(itl = m_vClientList.begin();itl != m_vClientList.end();itl++)
		{
			iStat = send(*itl,sMessage.c_str(),sMessage.size()+1,0);			
			if(iStat == -1)
				m_vClientList.remove(*itl);
		}

		if(iStat == -1)
			return 1;

		return 0;

}

int CChatServer::RecClient(SOCKET sRecSocket)
{
    char temp[4096];
	int iStat;

    //cout <<inet_ntoa(from.sin_addr) <<":"<<temp<<"\r\n";
		iStat = recv(sRecSocket,temp,4096,0);
		if(iStat == -1)
		{
			if(m_vClientList.size())
				m_vClientList.remove(sRecSocket);
			return 1;
		}
		else
		{
			cout <<":"<<temp<<"\n";
			SendMessagePort(temp);
			return 0;
		}
	return 0;

}


int main(int argc, char* argv[])
{
    int nRetCode = 0;
	char buf[4096];
	
	cout << "This aplication act as a chat server.\n";
	cout << "Messages from any pc will be broadcasted to all connected pcs.\n";
	cout << "Connect to the server pc port 8084.\n";
    cout << "Press ONLY ENTER to quit.\n";
	cout << "=================================================\n";

	if(!CServerObj.IsConnected())
	{
		cout<<"\nFailed to initialise server socket";
		cout<<"\nSigning off : Bye";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		return 1;
	}

	// TODO: Check if thread failed.
	#if defined(WIN32)
	  HANDLE tID;
	  tID = CreateThread(NULL, 0, ServerListenThread, 0, 0, NULL);
	#elif defined(__APPLE__)
	  pthread_t *tID;
	  pthread_create(tID, NULL, ServerListenThread, 0);
	#endif

	while(gets(buf))
	{
		if(strlen(buf) == 0)
			break;
		if(CServerObj.SendMessagePort(buf))
		{
			cout<<"Problem in connecting to server. Check whether server is running\n";
			break;
		}
	}

	cout<<"Signing off.";
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	#if defined(__APPLE__)
	  // dispose all threads
	  pthread_exit(NULL);
	#endif
    return nRetCode;
}

