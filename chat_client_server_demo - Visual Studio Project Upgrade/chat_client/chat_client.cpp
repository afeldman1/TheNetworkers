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

void CIPMessage::Init(string sIpAddress, int iPort)
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

int CIPMessage::SendMessagePort(string sMessage)
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
			cout<<"Server has ended!"<<endl;
		}
		return 1;
	}
	cout<<acRetData<<"\n";

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
	cout<<"This is a client TCP/IP application\nConnecting to port 8084\n";
	cout<<"\nPress ONLY ENTER to quit";
	cout<<"\n===============================================\n"<<endl;

	string sServerAddress;
	FILE *fp = fopen("server.ini","r");
	if(fp == NULL){
		cout<<"Unable to open server.ini!";
	}else{
		while((fgets(buf,4096,fp)) != NULL)
		{
			if(buf[0] == '#')
				continue;
			sServerAddress = buf;

		}
		fclose(fp);
		if(sServerAddress.size() == 0){
			cout<<"Unable to find server IP address in server.ini"<<endl;
		}else{
			cout<<"Using server IP address from server.ini: "<<sServerAddress<<endl;
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
		cout<<"Scanning for server..."<<endl;
		// Start broadcasting to find server:
		SOCKET broadSock;
		char opt = 1;
		// Create Broadcast Socket:
		broadSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(broadSock == -1){
			cout<<"Unable to create broadcast socket, closing..."<<endl;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			return 0;
		}
 
		// Set socket option to Broadcast:
		setsockopt(broadSock, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(char));
		SOCKADDR_IN broadAddr;
		memset(&broadAddr,0, sizeof(broadAddr));
		broadAddr.sin_family = AF_INET;
		broadAddr.sin_port = htons(8085);
		broadAddr.sin_addr.s_addr = INADDR_BROADCAST;
		unsigned int len = sizeof(broadAddr);
		string recieveMsg = "Where";
		string respondMsg = "Here";
		// Send Broadcast:
		int iStat = sendto(broadSock, recieveMsg.c_str(), recieveMsg.size(), 0, (sockaddr*)&broadAddr, len);
		if(iStat == -1){
			cout<<"Unable to send broadcast, closing..."<<endl;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			return 0;
		}
		// Recieve Msg:
		#define BUFSIZE 4096
		char buf[BUFSIZE];
		SOCKADDR_IN serverAddr;
		unsigned int serverLen = sizeof(serverAddr);
		iStat = recvfrom(broadSock, buf, BUFSIZE, 0, (struct sockaddr*) &serverAddr, &serverLen);
		if(iStat == -1){
			cout<<"Unable to retrieve broadcast response, closing..."<<endl;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			return 0;
		}
		// Retrieve IP:
		sServerAddress = inet_ntoa(serverAddr.sin_addr);
		closesocket(broadSock);
		cout<<"Found server at: "<<sServerAddress<<endl;
	}

	MyMessObj.Init(sServerAddress.c_str(),8084);
	if(!MyMessObj.IsConnected())
	{
		cout<<"\nUnable to connect to the IP address!";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
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

	cout<<"Enter name before entering the chat:"<<endl;
	while(gets(buf))
	{
		if(strlen(buf) == 0)
			break;
		if(MyMessObj.SendMessagePort(buf))
		{
			cout<<"Problem in connecting to server. Check whether server is running!\n";
			break;
		}
	}
	MyMessObj.closing = true;
	#ifdef WIN32
		WSACleanup();
	#endif
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	#ifndef WIN32
	  // dispose all threads
	  pthread_exit(NULL);
	#endif
	return nRetCode;
}

