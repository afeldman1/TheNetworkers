//************************************************************************
// Boby Thomas Pazheparampil
// May 2006
// Implementation of CIPMessage class and main.
//************************************************************************
#include "chat_client.h"

//Global Message object
CIPMessage MyMessObj;


CIPMessage::CIPMessage()
{
	m_bIsConnected = false;
}

void CIPMessage::Init(std::string sIpAddress, int iPort)
{

	m_sServerIPAddress = sIpAddress;
	m_iServerPort = iPort;
	struct hostent *hp;
	unsigned int addr;
	struct sockaddr_in server;

	conn=socket(AF_INET,SOCK_STREAM,0);
	if(conn==INVALID_SOCKET)
		return;

	addr=inet_addr(m_sServerIPAddress.c_str());
	hp=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	
	if(hp==NULL)
	{
		closesocket(conn);
		return;
	}

	server.sin_addr.s_addr=*((unsigned long*)hp->h_addr);
	server.sin_family=AF_INET;
	server.sin_port=htons(m_iServerPort);
	if(connect(conn,(struct sockaddr*)&server,sizeof(server)))
	{
		closesocket(conn);
		return;	
	}
	m_bIsConnected = true;
	closing = false;
	return;
}

CIPMessage::~CIPMessage()
{
	if(m_bIsConnected)
		closesocket(conn);
}

int CIPMessage::SendMessagePort(std::string sMessage)
{
		int iStat = 0;

		iStat = send(conn,sMessage.c_str(),sMessage.size()+1,0);
		if(iStat == -1)
			return 1;

		return 0;

}

int CIPMessage::RecMessagePort()
{
	char acRetData[4096];
	int iStat = 0;

	iStat = recv(conn,acRetData,4096,0);
	if(iStat == -1){
		if(!closing){
			std::cout<<"Server has ended!"<<std::endl;
		}
		return 1;
	}
	std::cout<<acRetData<<"\n";

	return 0;
}


THREAD MessageRecThread(LPVOID pParam)
{	
	while(1)
	{
		if(MyMessObj.RecMessagePort())
			break;
	}
	return 0;
}



int main(int argc, char* argv[])
{
	int nRetCode = 0;
	char buf[4096];
	std::cout<<"This is a client TCP/IP application\nConnecting to port 8084\n";
	std::cout<<"\nPress ONLY ENTER to quit";
	std::cout<<"\n===============================================\n"<<std::endl;

	std::string sServerAddress;
	FILE *fp = fopen("server.ini","r");
	if(fp == NULL){
		std::cout<<"Unable to open server.ini!";
	}else{
		while((fgets(buf,4096,fp)) != NULL)
		{
			if(buf[0] == '#')
				continue;
			sServerAddress = buf;

		}
		fclose(fp);
		if(sServerAddress.size() == 0){
			std::cout<<"Unable to find server IP address in server.ini"<<std::endl;
		}else{
			std::cout<<"Using server IP address from server.ini: "<<sServerAddress<<std::endl;
		}
	}

	#if defined(WIN32)
	  WSADATA wsaData;
	  int wsaret=WSAStartup(0x101,&wsaData);
	  if(wsaret!=0)
	  {
		  return 0;
	  }
	#endif

	if(sServerAddress.size() == 0)
	{
		std::cout<<"Scanning for server..."<<std::endl;
		// Start broadcasting to find server:
		SOCKET broadSock;
		char opt = 1;
		// Create Broadcast Socket:
		broadSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(broadSock == -1){
			std::cout<<"Unable to create broadcast socket, closing..."<<std::endl;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return 0;
		}
 
		// Set socket option to Broadcast:
		setsockopt(broadSock, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(char));
		SOCKADDR_IN broadAddr;
		memset(&broadAddr,0, sizeof(broadAddr));
		broadAddr.sin_family = AF_INET;
		broadAddr.sin_port = htons(8085);
		broadAddr.sin_addr.s_addr = INADDR_BROADCAST;
		socklen_t len = sizeof(broadAddr);
		std::string recieveMsg = "Where";
		std::string respondMsg = "Here";
		// Send Broadcast:
		int iStat = sendto(broadSock, recieveMsg.c_str(), recieveMsg.size()+1, 0, (sockaddr*)&broadAddr, len);
		if(iStat == -1){
			std::cout<<"Unable to send broadcast, closing..."<<std::endl;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return 0;
		}
		// Recieve Msg:
		#define BUFSIZE 4096
		char buf[BUFSIZE];
		SOCKADDR_IN serverAddr;
		socklen_t serverLen = sizeof(serverAddr);
		iStat = recvfrom(broadSock, buf, BUFSIZE, 0, (struct sockaddr*) &serverAddr, &serverLen);
		if(iStat == -1){
			std::cout<<"Unable to retrieve broadcast response, closing..."<<std::endl;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return 0;
		}
		// Retrieve IP:
		sServerAddress = inet_ntoa(serverAddr.sin_addr);
		closesocket(broadSock);
		std::cout<<"Found server at: "<<sServerAddress<<std::endl;
	}

	MyMessObj.Init(sServerAddress.c_str(),8084);
	if(!MyMessObj.IsConnected())
	{
		std::cout<<"\nUnable to connect to the IP address!";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return 0;	
	}

	// TODO: Check if thread failed.
	#if defined(WIN32)
	  //AfxBeginThread(MessageRecThread,0);
	  HANDLE tID;
	  tID = CreateThread(NULL, 0, MessageRecThread, 0, 0, NULL);  
	#else
	  pthread_t tID;
	  pthread_create(&tID, NULL, MessageRecThread, 0);
	#endif

	std::cout<<"Enter name before entering the chat:"<<std::endl;
	while(gets(buf))
	{
		if(strlen(buf) == 0)
			break;
		if(MyMessObj.SendMessagePort(buf))
		{
			std::cout<<"Problem in connecting to server. Check whether server is running!\n";
			break;
		}
	}
	MyMessObj.closing = true;
	#ifdef WIN32
		WSACleanup();
	#endif
	std::cout<<"Signed off, closing..."<<std::endl;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	#ifndef WIN32
	  // dispose all threads
	  pthread_exit(NULL);
	#endif
	return nRetCode;
}