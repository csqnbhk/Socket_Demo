/*************************************************************

   select_server��ͷ�ļ������ã��������з������Ĳ�������
   ���ߣ�С��
   2017/7/9
***********************************************************/
#ifndef _SELECT_SERVER_H_
#define _SELECT_SERVER_H_

#include<winsock2.h>
#include<windows.h>
#include<iostream>
#include<vector>
#include<string.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
struct ClientInfo
{
    SOCKET ClientSocket;
	char Buffer[1024];
	ClientInfo(SOCKET socket):ClientSocket(socket)
	{
		memset(Buffer,0,sizeof(Buffer));
	}

};
typedef vector<ClientInfo*> ConnectClient;

class Server
{

public:
	Server();
	void AddClient(SOCKET);    //���ӿͻ��˵�vector
	void DelClient(SOCKET);    //ɾ���ͻ���
    void ClearResource();      //������Դ


    SOCKET BindListen();       //�󶨼��������׽���
    void ResetFDSet();         //�������ü���

	void CheckAccept();        //����Ƿ����µĿͻ������ӽ���
	void CheckClient();        //����Ƿ�ͻ�������Ϣ����

	void DoWork();             //����
private:

   ConnectClient conns;
   SOCKET ServerSocket;
   SOCKADDR_IN ServerAddr;
   int DefaultPort;
   fd_set fdRead;
   fd_set fdWrite;
   fd_set fdExcept;
   int exit;

};



#endif