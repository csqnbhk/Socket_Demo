#ifndef _SERVE_H_
#define _SERVE_H_
#include<vector>
using namespace std;
#define nBufferSize 1024

struct IOContext:WSAOVERLAPPED
{
	SOCKET socket;
	SOCKADDR_IN ClientAddr;
	char Buffer[nBufferSize];
	WSABUF wsabuf;
	IOContext::IOContext(int ClientSocket,SOCKADDR_IN AddrInfo):socket(ClientSocket),ClientAddr(AddrInfo)
	{
		ZeroMemory(Buffer,sizeof(Buffer));
        ZeroMemory(this,sizeof(WSAOVERLAPPED));
		ZeroMemory(&wsabuf,sizeof(WSABUF));
		wsabuf.buf=Buffer;
		wsabuf.len=nBufferSize;
    
	}
};
class serveinfo
{

 public:
	serveinfo();
	~serveinfo();
 private:
 

 WIN32_FIND_DATA data;     //定义文件属性结构体
 WSADATA wsadata;
 SOCKET ServeSocket;       //服务器socket
 int ServePort;            //服务器端口
 SOCKADDR_IN ServeAddr;    //指定当地的地址信息
 HANDLE PublicICOP;        //创建的完成端口
 SYSTEM_INFO systeminfo;   //获取系统CPU个数
 HANDLE ThreadArry[30];    //存放线程池的线程句柄
 CRITICAL_SECTION cs;      //定义一个临界区
 vector<IOContext*> ClientArry;//vector存放连接的客户端
 
 public:
 
 bool Initialize();              //初始化
 void CreateICOPAndThreadPool(); //创建完成端口和线程池
 bool AcceptClient();            //监听接收连接的客户端
 void FSend(char*);      
 static DWORD WINAPI WorkThread(LPVOID);//线程函数，静态
 void ShowClientInfo();//创建显示客户连接信息线程
 


 
 void AddIOContext(IOContext*);//把连接进来的客户端存储在vector数组中
 void RemoveContext(IOContext*);//删除指定的客户端
 void GetClientInof();          //获取所有的客户端信息

 void ClearResource();          //释放IOContext申请的空间，关闭套接字

 SOCKET&Listen();
 
};

#endif