
#include<iostream>
#include<winsock2.h>
#include<windows.h>
#include<string.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
int main()
{
    
	WSADATA data;
	WSAStartup(MAKEWORD(2,2),&data);
	if(2!=LOBYTE(data.wHighVersion)||2!=HIBYTE(data.wVersion))
	{
		cout<<"初始化套接字库失败!"<<endl;
		return -1;

	}
	SOCKET clientsocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	SOCKADDR_IN addr;
	addr.sin_port=htons(1680);
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
	int connect_return=connect(clientsocket,(SOCKADDR*)&addr,sizeof(addr));
	if(connect_return==SOCKET_ERROR)
    {
		cout<<"连接服务器失败!"<<endl;
		return 0;
	}

	cout<<"连接服务器成功!"<<endl;
	char Buffer[1024]={0};
	while(1)
	{//
         send(clientsocket,"就绪通告select模型服务器是否成功!",sizeof("就绪通告select模型服务器是否成功!"),0);
	    cout<<"发送数据成功!"<<endl;
		
		 recv(clientsocket,Buffer,sizeof(Buffer),0);
		 cout<<"Buffer:"<<Buffer<<endl;
		
           Sleep(3000);
	}

   closesocket(clientsocket);
	return 0;
}