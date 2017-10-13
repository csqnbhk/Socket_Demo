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
 

 WIN32_FIND_DATA data;     //�����ļ����Խṹ��
 WSADATA wsadata;
 SOCKET ServeSocket;       //������socket
 int ServePort;            //�������˿�
 SOCKADDR_IN ServeAddr;    //ָ�����صĵ�ַ��Ϣ
 HANDLE PublicICOP;        //��������ɶ˿�
 SYSTEM_INFO systeminfo;   //��ȡϵͳCPU����
 HANDLE ThreadArry[30];    //����̳߳ص��߳̾��
 CRITICAL_SECTION cs;      //����һ���ٽ���
 vector<IOContext*> ClientArry;//vector������ӵĿͻ���
 
 public:
 
 bool Initialize();              //��ʼ��
 void CreateICOPAndThreadPool(); //������ɶ˿ں��̳߳�
 bool AcceptClient();            //�����������ӵĿͻ���
 void FSend(char*);      
 static DWORD WINAPI WorkThread(LPVOID);//�̺߳�������̬
 void ShowClientInfo();//������ʾ�ͻ�������Ϣ�߳�
 


 
 void AddIOContext(IOContext*);//�����ӽ����Ŀͻ��˴洢��vector������
 void RemoveContext(IOContext*);//ɾ��ָ���Ŀͻ���
 void GetClientInof();          //��ȡ���еĿͻ�����Ϣ

 void ClearResource();          //�ͷ�IOContext����Ŀռ䣬�ر��׽���

 SOCKET&Listen();
 
};

#endif