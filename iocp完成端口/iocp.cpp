/******************************************************************
    
      author:Demon
      time:2017/6/13

******************************************************************/



#include<iostream>
#include<winsock2.h>
#include<windows.h>
#include<vector>//�ǵ�Ҫ��vector����Ҫ������ͷ�ļ�
#pragma commect(lib,"ws2_32.lib")
using namespace std;
#define nBufferSize 4
#define MAX_THREAD_NUM 8
#define EXIT_THREAD 2017;
SOCKET g_hListenSocket=INVALID_SOCKET;
DWORD WINAPI WorkerThread(LPVOID lpvoid);
typedef enum _IO_OPERATION//����ö�٣�������д
{
	IoRead,
	IoWrite
}IO_OPERATION;
struct IOContext:WSAOVERLAPPED//Ϊÿ���ͻ��˴���һ�����󱣴���Ϣ
{
   SOCKET hSocket;
   char Buffer[nBufferSize];
   int nBytesInBuf;
   WSABUF wsaBuffer;
   IO_OPERATION op;
   IOContext(SOCKET sock):hSocket(sock),nBytesInBuf(0)
   {
	   wsaBuffer.buf=Buffer;
	   wsaBuffer.len=nBufferSize;
	   op=IoRead;
	   ZeroMemory(this,sizeof(WSAOVERLAPPED));
   }

};
 typedef vector<IOContext*> ConnectionList;
 ConnectionList g_conns;
 CRITICAL_SECTION g_csConns;
 void AddToList(IOContext *pIOContext)//��Ӷ���
 {
    EnterCriticalSection(&g_csConns);
	cout<<"�߳̽����ٽ���!"<<endl;
	g_conns.push_back(pIOContext);
	LeaveCriticalSection(&g_csConns);
 }
 void RemoveFromList(IOContext *pIOContext)// ��vectorɾ��ָ��Ԫ��
 {
	 EnterCriticalSection(&g_csConns);
	 ConnectionList::iterator it=g_conns.begin();
	 while(it!=g_conns.end())
	 {
		 if((*it)==pIOContext)
		 {
		 g_conns.erase(it);
		 break;
		 }
		 ++it;

	 }
	 LeaveCriticalSection(&g_csConns);
 }
 void CloseIO(IOContext *pIOContext)//�ͷŶ���
 {
	 closesocket(pIOContext->hSocket);
	 RemoveFromList(pIOContext);
	 delete pIOContext;
 }
 void DoWork()//����DoWork
 {
	 HANDLE hThreads[MAX_THREAD_NUM];
	 for(int i=0;i<MAX_THREAD_NUM;i++)
	 {
		 hThreads[i]=INVALID_HANDLE_VALUE;
	 }
	 DWORD dwThreadCount=0;
	 InitializeCriticalSection(&g_csConns);
	 WSADATA wsadata;
	 WSAStartup(MAKEWORD(2,2),&wsadata);
	 if(2!=LOBYTE(wsadata.wVersion)&&2!=HIBYTE(wsadata.wVersion))
	 {
		 cout<<"��ʼ���׽���ʧ��!"<<endl;
	 }
	 g_hListenSocket=socket(AF_INET,SOCK_STREAM,0);
	 SOCKADDR_IN serveaddr={0};
	 serveaddr.sin_port=htons(6666);
	 serveaddr.sin_family=AF_INET;
        bind(g_hListenSocket,(SOCKADDR*)&serveaddr,sizeof(serveaddr));
	 listen(g_hListenSocket,SOMAXCONN);
	 SYSTEM_INFO systemInfo;
	 GetSystemInfo(&systemInfo);
	 DWORD dwCPU=systemInfo.dwNumberOfProcessors;
	 HANDLE hIOCP=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	 for(dwThreadCount=0;dwThreadCount<dwCPU*2;++dwThreadCount)
	 {
		 unsigned long dwThreadId=0;
		 HANDLE hThread=CreateThread(0,0,WorkerThread,hIOCP,0,&dwThreadId);
         hThreads[dwThreadCount]=hThread;
	 }
    while(1)
	{
	//	 cout<<"���������ڼ���!"<<endl;
		SOCKET hAcceptSocket=accept(g_hListenSocket,0,0);
		hIOCP=CreateIoCompletionPort((HANDLE)hAcceptSocket,hIOCP,0,0);
		IOContext*pIOContext=new IOContext(hAcceptSocket);
		AddToList(pIOContext);
		DWORD dwFlags=0;
		int nRet=WSARecv(hAcceptSocket,&(pIOContext->wsaBuffer),1,NULL,&dwFlags,(LPWSAOVERLAPPED)pIOContext,NULL);
	}
 }
int main()
{
	DoWork();
    return 0;
}
DWORD WINAPI WorkerThread(LPVOID para)
{
	//cout<<"�߳�����!"<<endl;
	HANDLE hIOCP=(HANDLE)para;
	IOContext *pIOContext=NULL;
	DWORD dwBytes=0;
	ULONG ulKey=0;
	DWORD dwFlags=0;
	int nRet=0;
	while(1)
	{
		BOOL bOK=GetQueuedCompletionStatus(hIOCP,&dwBytes,&ulKey,(LPOVERLAPPED*)&pIOContext,INFINITE);
	cout<<"�߳�����!"<<endl;
		if(pIOContext->op==IoRead)
		{
		   
			pIOContext->nBytesInBuf+=dwBytes;
			pIOContext->wsaBuffer.buf=pIOContext->Buffer;
			pIOContext->wsaBuffer.len=pIOContext->nBytesInBuf;
             
			ZeroMemory(pIOContext,sizeof(WSAOVERLAPPED));
			pIOContext->op=IoWrite;
			dwFlags=0;
           cout<<pIOContext->Buffer<<endl;
			nRet=WSASend(pIOContext->hSocket,&(pIOContext->wsaBuffer),1,NULL,dwFlags,pIOContext,NULL);
		}
		else if(pIOContext->op==IoWrite)
		{
            pIOContext->nBytesInBuf-=dwBytes;
		    if(pIOContext->nBytesInBuf>0)
			{
				memmove(pIOContext->Buffer,pIOContext->Buffer+dwBytes,pIOContext->nBytesInBuf);
			}
            pIOContext->wsaBuffer.buf=pIOContext->Buffer+pIOContext->nBytesInBuf;
			pIOContext->wsaBuffer.len=nBufferSize-pIOContext->nBytesInBuf;
			ZeroMemory(pIOContext,sizeof(WSAOVERLAPPED));
			pIOContext->op=IoRead;
			dwFlags=0;
			nRet=WSARecv(pIOContext->hSocket,&(pIOContext->wsaBuffer),1,NULL,&dwFlags,pIOContext,NULL);


		}
	}
	return 0;
}