
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
		cout<<"��ʼ���׽��ֿ�ʧ��!"<<endl;
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
		cout<<"���ӷ�����ʧ��!"<<endl;
		return 0;
	}

	cout<<"���ӷ������ɹ�!"<<endl;
	char Buffer[1024]={0};
	while(1)
	{//
         send(clientsocket,"����ͨ��selectģ�ͷ������Ƿ�ɹ�!",sizeof("����ͨ��selectģ�ͷ������Ƿ�ɹ�!"),0);
	    cout<<"�������ݳɹ�!"<<endl;
		
		 recv(clientsocket,Buffer,sizeof(Buffer),0);
		 cout<<"Buffer:"<<Buffer<<endl;
		
           Sleep(3000);
	}

   closesocket(clientsocket);
	return 0;
}