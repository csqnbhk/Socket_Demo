#include<winsock2.h>
#include<windows.h>
#include<iostream>
#include<vector>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
const char SendText[15]="Welcome you!";
void ResetConns();
int FillEventsArray();
//体客户端结构
struct ClientInfo
{
    SOCKET clientsocket;
	char Buffer[1024];
	WSAEVENT eventobject;
	ClientInfo(SOCKET socket):clientsocket(socket)
	{
		memset(Buffer,0,sizeof(Buffer));
		eventobject=WSACreateEvent();
        WSAEventSelect(clientsocket,eventobject,FD_READ|FD_CLOSE|FD_WRITE);
	}
};

typedef vector<ClientInfo*> ConnectClient;
ConnectClient conns;

//事件对象数组
WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];

int main()
{
 
     //创建服务器套接字
  	WSADATA data={0};
    WSAStartup(MAKEWORD(2,2),&data);
   if(2!=LOBYTE(data.wHighVersion)||2!=HIBYTE(data.wVersion))
   {
	   cout<<"WSAStartup() failed :"<<WSAGetLastError()<<endl;
	   return -1;
   }
    SOCKET ListenSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(ListenSocket==INVALID_SOCKET)
	{
		cout<<"socket() failed :"<<WSAGetLastError()<<endl;
		return -1;
	}
    
    SOCKADDR_IN addr={0};
	addr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
	addr.sin_family=AF_INET;
	addr.sin_port=htons(1680);
	
	int bind_return=bind(ListenSocket,(SOCKADDR*)&addr,sizeof(addr));
    if(bind_return==SOCKET_ERROR)
	{
		cout<<"bind() failed :"<<GetLastError()<<endl;
		return -1;
	}
	listen(ListenSocket,SOMAXCONN);

	//把事件对象赋值为无效
    for(int i=0;i<WSA_MAXIMUM_WAIT_EVENTS;i++)
	{
		events[i]=WSA_INVALID_EVENT;
	}
    //服务器监听事件对象创建
     WSAEVENT ListenEvent=WSACreateEvent();
    if(WSAEventSelect(ListenSocket,ListenEvent,FD_ACCEPT)==SOCKET_ERROR)
	{
		cout<<" ListenSocket WSAEventSelect() failed :"<<WSAGetLastError()<<endl;
		return -1;
	}
	events[0]=ListenEvent;
  
	while(1)
	{
           ResetConns();
		   int nEvents=FillEventsArray();
           int nRet= WSAWaitForMultipleEvents(nEvents,events,false,WSA_INFINITE,false);
           if(nRet==WSA_WAIT_FAILED)
		   {
			   cout<<"WSAWaitForMultipleEvents() failed :"<<WSAGetLastError()<<endl;
			   return -1;
		   }
          nRet=nRet-WSA_WAIT_EVENT_0;
		  for(int nIndex=nRet;nIndex<nEvents;nIndex++)
		  {
                nRet=WSAWaitForMultipleEvents(1,&events[nIndex],true,0,false);
				if(nRet==WSA_WAIT_FAILED||nRet==WSA_WAIT_TIMEOUT )
                continue;
			
                cout<<"nIndex="<<nIndex<<endl;

				SOCKET hSocket=ListenSocket;
				WSAEVENT hEvent=ListenEvent;
				if(nIndex>0)
				{
					hSocket=conns[nIndex-1]->clientsocket;
					hEvent=conns[nIndex-1]->eventobject;
				}
			
                WSANETWORKEVENTS wsaEvents;
				WSAEnumNetworkEvents(hSocket,hEvent,&wsaEvents);

				if(wsaEvents.lNetworkEvents&FD_ACCEPT)
				{
					
					SOCKET acceptsocket=accept(ListenSocket,0,0);
                    if(acceptsocket==INVALID_SOCKET)
					{
						cout<<"accept() failed :"<<WSAGetLastError()<<endl;
						return -1;
					}
					conns.push_back(new ClientInfo(acceptsocket));
					cout<<"又有一个客户端连接进来!"<<endl;
					send(acceptsocket,SendText,sizeof(SendText),0);

				}
				if(wsaEvents.lNetworkEvents&FD_CLOSE)
				{
					conns[nIndex-1]->eventobject=WSA_INVALID_EVENT;
					cout<<"一个客户端退出连接!"<<endl;
				   

				}
				if(wsaEvents.lNetworkEvents&FD_READ)
				{ 
                        
					     recv(conns[nIndex-1]->clientsocket,conns[nIndex-1]->Buffer,sizeof(conns[nIndex-1]->Buffer),0);
						 send(conns[nIndex-1]->clientsocket,p->Buffer,sizeof(p->Buffer),0);
                         
					
                        cout<<"接收到消息为:"<<conns[nIndex-1]->Buffer<<endl;
                        memset(conns[nIndex-1]->Buffer,0,sizeof(conns[nIndex-1]->Buffer));
					
						
				}
				if(wsaEvents.lNetworkEvents&FD_WRITE)
				{
                     
                      
				}
				

		  }

	}

	return 0;
}
//ResetConns函数实现
void ResetConns()
{

	vector<ClientInfo*>::iterator it=conns.begin();
	while(it!=conns.end())
	{
		ClientInfo*p=*it;

		if(p->eventobject==WSA_INVALID_EVENT)
		{
			
			WSACloseEvent(p->eventobject);
			delete p;
            it=conns.erase(it);//注意这里
		}
		else
		{
		  it++;
		}
	}
	

}
//FillEventsArray函数实现
int FillEventsArray()
{
	int Count=1;
    int i=1;
   vector<ClientInfo*>::iterator it=conns.begin(); 
   while(it!=conns.end())
   {
	   ClientInfo*p=*it;
	   events[i]=p->eventobject;
	   it++;
	   i++;
       Count++;
   }

	return Count;
}