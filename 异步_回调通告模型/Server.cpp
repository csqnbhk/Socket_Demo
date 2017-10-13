#include<winsock2.h>
#include<Mswsock.h>
#include<windows.h>
#include<iostream>
#include<vector>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")
using namespace std;
//定义客户端结构体
enum IO_OPERATION
{
	IoRead,
    IoWrite
};
struct ClientInfo:WSAOVERLAPPED
{

   SOCKET ClientSocket;
   char Buffer[1024];
   char IP[30];
   u_short Port;
   WSABUF wsabuffer;
   IO_OPERATION op;
   ClientInfo(SOCKET socket):ClientSocket(socket)
   {
	  
	   
	   ZeroMemory(this,sizeof(WSAOVERLAPPED));
	   memset(IP,0,sizeof(IP));
	   memset(Buffer,0,sizeof(Buffer));
       wsabuffer.buf=Buffer;
	   wsabuffer.len=sizeof(Buffer);
	   op=IoRead;
   }


};
typedef vector<ClientInfo*> ConnectClients;
ConnectClients Conns;

//函数声明
void RemoveClient(ClientInfo*);
void CALLBACK CompletionRoutine(DWORD,DWORD,LPWSAOVERLAPPED,DWORD);

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
	 cout<<"进入循环...."<<endl;
	  while(1)
	  {
            


		  int nRet=WaitForSingleObjectEx(ListenOverlapped.hEvent,INFINITE,true);
		  if(nRet==WAIT_OBJECT_0)
		  {
			 //这里处理Accept事件
			  WSAResetEvent(ListenOverlapped.hEvent);
             DWORD nByte=0;
			 DWORD Flag=0;
             if(!WSAGetOverlappedResult(ListenSocket,&ListenOverlapped,&nByte,false,&Flag))
			 {
				 cout<<"WSAGetOverlappedResult() failed :"<<WSAGetLastError()<<endl;
				 return -1;
			 }
              
			 ClientInfo*p=new ClientInfo(AcceptSocket);
             Conns.push_back(p);
 
          //获取客户端的ip和端口
			 cout<<"开始获取客户IP和端口"<<endl;
			 SOCKADDR  *pRemoteSockaddr=NULL;
			 SOCKADDR  *pLocalSockaddr=NULL;
			 int RemoteSockAddrLength=sizeof(SOCKADDR);
			 int LocalSockAddrLength=sizeof(SOCKADDR);
			 GetAcceptExSockaddrs(AcceptBuffer,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,&pLocalSockaddr,&LocalSockAddrLength,&pRemoteSockaddr,&RemoteSockAddrLength);
             SOCKADDR_IN remoteaddr={0};
			 memcpy(&remoteaddr,pRemoteSockaddr,sizeof(SOCKADDR_IN));
			 
			 
			 char*premoteip=inet_ntoa(remoteaddr.sin_addr);
			 u_short Port=ntohs(remoteaddr.sin_port);
           //保存客户端信息并发起一个WSARecv请求
			 
			
			 strcpy(p->IP,premoteip);
			 p->Port=Port;
			 cout<<"客户端IP"<<"   "<<"客户端Port"<<endl;
			 cout<<p->IP<<"        "<<p->Port<<endl;
			 Flag=0;
			 int return_wsarecv=WSARecv(p->ClientSocket,&(p->wsabuffer),1,NULL,&Flag,(LPWSAOVERLAPPED)p,&CompletionRoutine);
			 int lasterror=WSAGetLastError();
			 if(return_wsarecv==SOCKET_ERROR&&lasterror!=WSA_IO_PENDING)
			 {
				 cout<<"发送WSARecv请求失败!"<<endl;
				 return -1;
			 }
            
          
            AcceptSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
	       if(AcceptSocket==INVALID_SOCKET)
		   {
	     	cout<<"socket() failed :"<<WSAGetLastError()<<endl;
		    return -1;
		   }
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
		  else if(nRet==WAIT_IO_COMPLETION)
		  {
			  //这里交给回调函数处理,其实回调函数已经处理完毕
			  continue;
		  }
		  else
		  {
			  cout<<"nRet出现错误!"<<endl;
			  return -1;
		  }


	  }



	return 0;
}
//移除无效的客户端套接字
void RemoveClient(ClientInfo*p)
{
	vector<ClientInfo*>::iterator it=Conns.begin();
	while(it!=Conns.end())
	{
		if((*it)==p)
		{
			Conns.erase(it);
		}
		else
		{
			it++;
		}
	}
}
//回调函数
void CALLBACK CompletionRoutine(DWORD dwError,DWORD cbTransferred,LPWSAOVERLAPPED lpoverlapped,DWORD dwFlag)
{
	ClientInfo*p=(ClientInfo*)lpoverlapped;
	if(dwError!=0)
	{
		cout<<"I/O Error:"<<dwError<<endl;
		if(dwError==ERROR_OPERATION_ABORTED)
		cout<<"I/O was cancelled"<<endl;
		else if(dwError==WSAECONNRESET)
		cout<<"Connection was closed unexpectedly"<<endl;
		closesocket(p->ClientSocket);
		RemoveClient(p);
		delete p;
		return ;
	}
	if(p->op==IoRead)
	{
	p->op=IoWrite;
    cout<<"IP:"<<p->IP<<"Port:"<<p->Port<<"发来信息"<<endl;
	cout<<p->Buffer<<endl;
	DWORD Flag=0;
    int return_wsasend=WSASend(p->ClientSocket,&(p->wsabuffer),1,NULL,Flag,(LPWSAOVERLAPPED)p,&CompletionRoutine);
	int lastError=WSAGetLastError();
	if(return_wsasend==SOCKET_ERROR&&lastError!=WSA_IO_PENDING)
	{
		cout<<"发送WSASend请求失败!"<<endl;
		closesocket(p->ClientSocket);
		RemoveClient(p);
		delete p;
		return;
	}

	}
    else if(p->op==IoWrite)//记得千万不要写成if(p->op==IoWrite),因为出现错误，已经释放p,下面有用到p.哈哈，会出现很大问题
	{
        p->op=IoRead;
		DWORD Flag=0;
        int return_wsarecv=WSARecv(p->ClientSocket,&(p->wsabuffer),1,NULL,&Flag,(LPWSAOVERLAPPED)p,&CompletionRoutine);
        int lasterror=WSAGetLastError();
		if(return_wsarecv==SOCKET_ERROR&&lasterror!=WSA_IO_PENDING)
		{
			cout<<"发送WSARecv请求失败!"<<endl;
			closesocket(p->ClientSocket);
		    RemoveClient(p);
		    delete p;
		    return;
		}
		
	}
	

}