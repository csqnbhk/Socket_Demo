/*************************************************************

   select_server该头文件的作用：定义所有服务器的操作函数
   作者：小菜
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
	void AddClient(SOCKET);    //增加客户端到vector
	void DelClient(SOCKET);    //删除客户端
    void ClearResource();      //回收资源


    SOCKET BindListen();       //绑定监听服务套接字
    void ResetFDSet();         //重新设置集合

	void CheckAccept();        //检查是否有新的客户端连接进来
	void CheckClient();        //检查是否客户端有信息到来

	void DoWork();             //处理
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