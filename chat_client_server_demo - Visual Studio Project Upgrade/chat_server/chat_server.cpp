//************************************************************************
// Implementation of CIPMessage class and main.
//************************************************************************

#include "chat_server.h"


CChatServer CServerObj;

THREAD ServerRecThread(LPVOID pParam)
{	
	//SOCKET sRecSocket = reinterpret_cast<SOCKET>(pParam);
	Client sRecClient = *((Client*)(pParam));
	delete (Client*)(pParam);
	while(1)
	{
		//if(CServerObj.RecClient(sRecSocket))
		if(CServerObj.RecClient(sRecClient))
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
	struct sockaddr_in clientAddr;
	int iStat;
	socklen_t clientLength = sizeof(clientAddr);
	#define BUFSIZE 4096
	char tempBuf[BUFSIZE];
	std::string recieveMsg = "Where";
	std::string respondMsg = "Here";
	while(true)
	{
		// Listen for clients:
		iStat = recvfrom(sRecSocket, tempBuf, BUFSIZE, 0, (struct sockaddr*) &clientAddr, &clientLength);
		if(iStat == -1){
			break;
		}

		// Respond to client:
		iStat = sendto(sRecSocket, respondMsg.c_str(), respondMsg.size()+1, 0, (struct sockaddr*) &clientAddr, clientLength);
		if(iStat == -1){
			break;
		}
	}
	return 0;
}



CChatServer::CChatServer()
{
    std::cout << "Starting up TCP Chat server\n";
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


	if(bind(m_SListenClient, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR || bind(unknownListenClient, (sockaddr*)&unknown, sizeof(unknown)) == SOCKET_ERROR)
    {
        return;
    }

	#if defined(WIN32)
	  HANDLE tID;
	  tID = CreateThread(NULL, 0, LookoutThread, reinterpret_cast<void*>(unknownListenClient), 0, NULL);  
	#else
	  pthread_t tID;
	  pthread_create(&tID, NULL, LookoutThread, reinterpret_cast<void*>(unknownListenClient));
	#endif

    if(listen(m_SListenClient,10) != 0)
    {
        return;
    }

	m_bIsConnected = true;
	closing = false;
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

	if(m_SClient != INVALID_SOCKET){
		Client *newClient = new Client();
		newClient->sock = m_SClient;
		ClientList.push_back(*newClient);
		std::cout<<inet_ntoa(from.sin_addr)<<" has connected."<<std::endl;

		#if defined(WIN32)
			HANDLE tID;
			//tID = CreateThread(NULL, 0, ServerRecThread, reinterpret_cast<void*>(m_SClient), 0, NULL);
			tID = CreateThread(NULL, 0, ServerRecThread, newClient, 0, NULL);
		#elif defined(__APPLE__)
			pthread_t tID;
			//pthread_create(&tID, NULL, ServerRecThread, reinterpret_cast<void*>(m_SClient));
			pthread_create(&tID, NULL, ServerRecThread, newClient);
		#endif
	}
}

int CChatServer::SendMessageTo(Client sClient, std::string MSG)
{
		int iStat = send(sClient.sock, MSG.c_str(), MSG.size()+1, 0);
		if(iStat == -1){
			ClientList.remove(sClient);
			std::cout<<sClient.name + " has left."<<std::endl;
			SendMessageAll(sClient.name + " has left.");
			return 1;
		}
		return 0;
}
int CChatServer::SendMessageAll(std::string MSG, Client aClient)
{
		int iStat = 0;
		std::list<Client>::iterator itl;
		if(ClientList.size() == 0)
			return 0;

		for(itl = ClientList.begin();itl != ClientList.end();itl++)
		{
			if(itl->name != aClient.name && !itl->name.empty()){
				iStat = send(itl->sock,MSG.c_str(),MSG.size()+1,0);
				if(iStat == -1){
					std::cout<<itl->name + " has left."<<std::endl;
					SendMessageAll(itl->name + " has left.", *itl);
					ClientList.remove(*itl);
					return 1;
				}
			}
		}
		return 0;
}

std::list<Client>::iterator CChatServer::FindClient(Client sRecClient){
	std::list<Client>::iterator itl;
	if(ClientList.size() == 0)
		return itl;

	for(itl = ClientList.begin();itl != ClientList.end();itl++)
	{
		if(itl->sock == sRecClient.sock){
			return itl;
		}
	}
}

int CChatServer::RecClient(Client& sRecClient)
{
    char temp[4096];
	int iStat;

    //std::cout <<inet_ntoa(from.sin_addr) <<":"<<temp<<"\r\n";
	iStat = recv(sRecClient.sock,temp,4096,0);
	if(!closing){
	if(iStat == -1)
	{
		if(ClientList.size())
			ClientList.remove(sRecClient);
		std::cout<<sRecClient.name + " has left."<<std::endl;
		SendMessageAll(sRecClient.name + " has left.");
		return 1;
	}
	else
	{
		if(sRecClient.name.empty()){
			std::string reqName = temp;
			std::list<Client>::iterator itl;
			if(ClientList.size() == 0)
				return 0; // Massive problems if this happens. Restart server.
			for(itl = ClientList.begin();itl != ClientList.end();itl++)
			{
				if(itl->name == reqName){
					// Send Not Valid Name
					SendMessageTo(sRecClient, "Invalid Name!");
					return 0;
				}
			}
			// Send Was Valid Name
			SendMessageTo(sRecClient, "Valid Name!");
			FindClient(sRecClient)->name = reqName;
			sRecClient.name = reqName;
			std::cout<<reqName + " has entered chat."<<std::endl;
			SendMessageAll(reqName + " has entered chat.", sRecClient);
		}else{
			std::string msg = sRecClient.name + ": " + temp;
			std::cout<<msg<<std::endl;
			SendMessageAll(msg, sRecClient);
		}
		return 0;
	}
	}
	return 0;
}

int CChatServer::Shutdown(){
	closing = true;
	int iStat = 0;
	std::list<Client>::iterator itl;
	if(ClientList.size() == 0)
		return 0;

	for(itl = ClientList.begin();itl != ClientList.end();itl++)
	{
		closesocket(itl->sock);
		shutdown(itl->sock, SD_BOTH);
		//ClientList.remove(*itl);
	}
	#ifdef WIN32
		WSACleanup();
	#endif
}


int main(int argc, char* argv[])
{
    int nRetCode = 0;
	char buf[4096];
	
	std::cout << "This aplication act as a chat server.\n";
	std::cout << "Messages from any pc will be broadcasted to all connected pcs.\n";
	std::cout << "Connect to the server pc port 8084.\n";
    std::cout << "Press ONLY ENTER to quit.\n";
	std::cout << "=================================================\n";

	if(!CServerObj.IsConnected())
	{
		std::cout<<"\nFailed to initialise server socket";
		std::cout<<"\nSigning off : Bye";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return 1;
	}

	// TODO: Check if thread failed.
	#if defined(WIN32)
	  HANDLE tID;
	  tID = CreateThread(NULL, 0, ServerListenThread, 0, 0, NULL);
	#else
	  pthread_t tID;
	  pthread_create(&tID, NULL, ServerListenThread, 0);
	#endif

	std::string ServerName = "Server";
	while(gets(buf))
	{
		if(strlen(buf) == 0)
			break;
		if(CServerObj.SendMessageAll(ServerName + ": " + buf))
		{
			std::cout<<"Problem in connecting to server. Check whether server is running\n";
			break;
		}
	}

	CServerObj.Shutdown();
	std::cout<<"Signed off, closing..."<<std::endl;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	#ifndef WIN32
	  // dispose all threads
	  pthread_exit(NULL);
	#endif
    return nRetCode;
}