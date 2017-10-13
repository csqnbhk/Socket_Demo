#include<winsock2.h>
#include<Mswsock.h>
#include<windows.h>
#include<iostream>
#include<vector>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

void ResetConns();
int Fillevents();
//定义客户端信息结构体
struct ClientInfo
{
   SOCKET ClientSocket;
   char Buffer[1024];
   WSABUF wsabuffer;
   WSAOVERLAPPED overlapped;
   ClientInfo(SOCKET socket):ClientSocket(socket)
   {
	   wsabuffer.buf=Buffer;
	   wsabuffer.len=sizeof(Buffer);
	   ZeroMemory(&overlapped,sizeof(WSAOVERLAPPED));
	   overlapped.hEvent=WSACreateEvent();
   }
   

	
};
typedef vector<ClientInfo*> ConnectClients;
ConnectClients conns;
//定义事件内核对象数组
 HANDLE events[WSA_MAXIMUM_WAIT_EVENTS];


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
    SOCKET ListenSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);//采用WSASocket创建异步socket
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
   
	//创建一个预备连接客户端socket
	 SOCKET AcceptSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
	 
	 //创建监听重叠结构
	 WSAOVERLAPPED ListenOverlapped={0};
     //创建监听事件对象
	 HANDLE ListenEvent=WSACreateEvent();
	 ListenOverlapped.hEvent=ListenEvent;
     
	 char AcceptBuffer[2*(sizeof(SOCKADDR_IN)+16)]={0};
	 DWORD AcceptBytes=0;
	 //发出一个异步accept请求
	 if(!AcceptEx(ListenSocket,AcceptSocket,AcceptBuffer,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,&AcceptBytes,&ListenOverlapped))
	 {
      int lastError=WSAGetLastError();
	  if(lastError!=ERROR_IO_PENDING)
	 {
         cout<<"AcceptEx() failed :"<<WSAGetLastError()<<endl;
		 return -1;
	 }
	 }
	 //给定义事件内核对象数组赋值并且放入ListenEvent
	 for(int i=0;i<WSA_MAXIMUM_WAIT_EVENTS;i++)
	 {
		 events[i]=NULL;
	 }
	 events[0]=ListenOverlapped.hEvent;
     cout<<"进入循环...."<<endl;
	 while(1)
	 {
        ResetConns();
        int nEvents=Fillevents();

		int nRet=WSAWaitForMultipleEvents(nEvents,events,false,WSA_INFINITE,false);
	//	cout<<"事件等待函数返回!"<<endl;
		if(nRet==WSA_WAIT_FAILED)
		{
			cout<<"WSAWaitForMultipleEvents() failed: "<<WSAGetLastError()<<endl;
			return -1;
		}
         cout<<"nRet="<<nRet<<endl;
        nRet=nRet-WSA_WAIT_EVENT_0;
		cout<<"nRet="<<nRet<<endl;
		//Sleep(5000);
		for(int nIndex=nRet;nIndex<nEvents;nIndex++)
		{
             
			nRet=WSAWaitForMultipleEvents(1,&events[nIndex],true,0,false);
            if(nRet==WSA_WAIT_FAILED||nRet==WSA_WAIT_TIMEOUT)
		    continue;
            
			WSAResetEvent(events[nIndex]);
			cout<<"重置监听事件对象成功"<<endl;
			//处理监听套接字
			if(nIndex==0)
			{   
				 cout<<"Accept 有响应!"<<endl;
                 DWORD flag=0;
				 DWORD bytes=0;
				 if(!WSAGetOverlappedResult(ListenSocket,&ListenOverlapped,&bytes,false,&flag))
				 {
					 cout<<"WSAGetOverlappedResult() failed :"<<WSAGetLastError()<<endl;
					 continue;

				 }
				 if(conns.size()>=63)
				 {
					 cout<<"单线程已经达到最大连接数!"<<endl;
					 continue;
				 }
                 //保存客户端信息
				 ClientInfo*p=new  ClientInfo(AcceptSocket);
				 conns.push_back(p);
				 //发出客户端第一个recv请求
				 int recv_return=WSARecv(p->ClientSocket,&(p->wsabuffer),1,NULL,&flag,&p->overlapped,NULL);
				 int error=WSAGetLastError();
				 if(recv_return==SOCKET_ERROR&&error!=WSA_IO_PENDING)
				 {
					 cout<<"WSARecv() failed :"<<WSAGetLastError()<<endl;
					 p->ClientSocket=INVALID_SOCKET;
					 continue;
				 }
                 //获取客户端信息
                 SOCKADDR *premoteaddr=NULL;
				 SOCKADDR *plocaladdr=NULL;
				 int remotelen=sizeof(SOCKADDR);
				 int locallen=sizeof(SOCKADDR);
                 GetAcceptExSockaddrs(AcceptBuffer,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,&plocaladdr,&locallen,&premoteaddr,&remotelen);
				 SOCKADDR_IN RemoteAddr={0};
				 SOCKADDR_IN LocalAddr={0};
				 memcpy(&RemoteAddr,premoteaddr,sizeof(SOCKADDR_IN));
				 memcpy(&LocalAddr,plocaladdr,sizeof(SOCKADDR_IN));
				 char* RemoteIp=inet_ntoa(RemoteAddr.sin_addr);
				 char*LocalIp=inet_ntoa(LocalAddr.sin_addr);
				 u_short RemotePort=ntohs(RemoteAddr.sin_port);
				 u_short LocalPort=ntohs(LocalAddr.sin_port);
				 cout<<"服务器IP:"<<LocalIp<<"  "<<"服务器Port:"<<LocalPort<<endl;
                 cout<<"客户端IP:"<<RemoteIp<<"   "<<"客户端port:"<<RemotePort<<endl;
				 memset(AcceptBuffer,0,sizeof(AcceptBuffer));
                 //服务器再次发起accept请求
                  AcceptSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
                  if(!AcceptEx(ListenSocket,AcceptSocket,AcceptBuffer,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,&AcceptBytes,&ListenOverlapped))
				  {
                      int lastError=WSAGetLastError();
	                  if(lastError!=ERROR_IO_PENDING)
					  {
                         cout<<"AcceptEx() failed :"<<WSAGetLastError()<<endl;
		                 return -1;
					  }
				  }
               
                 
			}
			else
			{

				 ClientInfo*p=conns[nIndex-1];
                 DWORD flag=0;
				 DWORD bytes=0;
				 if(!WSAGetOverlappedResult(p->ClientSocket,&p->overlapped,&bytes,false,&flag))
				 {
					 cout<<"WSAGetOverlappedResult() failed :"<<WSAGetLastError()<<endl;
					 p->ClientSocket=INVALID_SOCKET;
					 continue;

				 }
                 cout<<"接收客户Socket:"<<p->ClientSocket<<"的信息为:"<<p->Buffer<<endl;
				 send(p->ClientSocket,p->Buffer,sizeof(p->Buffer),0);
				 memset(p->Buffer,0,sizeof(p->Buffer));
				
				 //再发出客户端第一个recv请求
				 int recv_return=WSARecv(p->ClientSocket,&p->wsabuffer,1,NULL,&flag,&p->overlapped,NULL);
				 int error=WSAGetLastError();
				 if(recv_return==SOCKET_ERROR&&error!=WSA_IO_PENDING)
				 {
					 cout<<"WSARecv() failed :"<<WSAGetLastError()<<endl;
					 p->ClientSocket=INVALID_SOCKET;
					 continue;
				 }
				
			}


		}


	 }
	return 0;
}

//检查连接客户端客户端
void ResetConns()
{
    vector<ClientInfo*>::iterator it=conns.begin();
	while(it!=conns.end())
	{
		ClientInfo*p=*it;
		if(p->ClientSocket==INVALID_SOCKET)
		{
		    WSACloseEvent(p->overlapped.hEvent);
			delete p;
			it=conns.erase(it);

		}
		else
		it++;
	}
}
//填充事件对象数组
int Fillevents()
{  
	int i=1;
	int Count=1;
	vector<ClientInfo*>::iterator it=conns.begin();
    
	while(it!=conns.end())
	{     
		  ClientInfo*p=*it;
          events[i]=p->overlapped.hEvent;
		  it++;
		  i++;
		  Count++;

	}

	return Count;
}