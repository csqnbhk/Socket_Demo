#include<iostream>
#include<winsock2.h>
#include<windows.h>
#include<time.h>
#include"serve.h"
using namespace std;

//构造函数
serveinfo::serveinfo()
{
  
	this->ServePort=1680;
	this->ServeSocket=NULL;
	this->PublicICOP=INVALID_HANDLE_VALUE;
	memset(this->ThreadArry,0,sizeof(this->ThreadArry));
	memset(&this->data,0,sizeof(data));
	InitializeCriticalSection(&this->cs);
	

}
//析构函数
serveinfo::~serveinfo()
{
   
}
//初始化
bool serveinfo::Initialize()
{
   int ReturnResult=WSAStartup(MAKEWORD(2,2),&this->wsadata);
   if(ReturnResult!=0)
   {
	   MessageBox(0,"调用WSAStartup()函数失败!","程序提示",MB_OK|MB_ICONWARNING);
	   return false;
   }
   if(2!=LOBYTE(wsadata.wVersion)||2!=HIBYTE(wsadata.wVersion))
   {
	   MessageBox(0,"套接字版本出错!","程序提示",MB_OK|MB_ICONWARNING);
	   return false;
   }
   
   return true;
}
//创建服务端socket·并监听
SOCKET&serveinfo::Listen()
{
   this->ServeSocket=socket(AF_INET,SOCK_STREAM,0);
   if(this->ServeSocket==INVALID_SOCKET)
   {
	   MessageBox(0,"创建socket失败!","程序提示",MB_OK|MB_ICONWARNING);
	   return this->ServeSocket;
   }
   this->ServeAddr.sin_port=htons(this->ServePort);
   this->ServeAddr.sin_family=AF_INET;
   this->ServeAddr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
   int bindresult=bind(this->ServeSocket,(SOCKADDR*)&this->ServeAddr,sizeof(this->ServeAddr));
   if(bindresult==SOCKET_ERROR)
   {
	   MessageBox(0,"bind函数调用失败!","程序提示",MB_OK|MB_ICONWARNING);
	   return this->ServeSocket;
   }
   int listenresult=listen(this->ServeSocket,SOMAXCONN);
   if(listenresult==SOCKET_ERROR)
   {
	   MessageBox(0,"listen函数调用失败!","程序提示",MB_OK|MB_ICONWARNING);
	   return this->ServeSocket;
   }
   return this->ServeSocket;
}
//创建完成端口并和服务器socket关联（其实可以不把服务器socket放进该完成端口,如何调用AcceptEx要关联，并投递WSAAccept）
void serveinfo::CreateICOPAndThreadPool()
{
    this->PublicICOP=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	if(this->PublicICOP==NULL)
	{
		MessageBox(0,"创建完成端口失败!","程序提示",MB_OK|MB_ICONWARNING);
		return ;
	}
    /*HANDLE TempHandle=CreateIoCompletionPort((HANDLE)this->ServeSocket,this->PublicICOP,0,0);
	if(TempHandle==NULL)
	{
		MessageBox(0,"完成端口和服务器套接字关联失败!","程序提示",MB_OK|MB_ICONWARNING);
		return ;
	}
    */
	GetSystemInfo(&this->systeminfo);
	for(int i=0;i<this->systeminfo.dwNumberOfProcessors*2;i++)
	{
        this->ThreadArry[i]=CreateThread(0,0,WorkThread,this,0,0);//给静态的线程函数传入this指针，可以调用类的成员函数。静态成员函数本来只可以调用
		CloseHandle(this->ThreadArry[i]);
      
	}
  

}
//等待接收客户端的连接
bool serveinfo::AcceptClient()
{
    SOCKET AcceptSocket=NULL;
	SOCKADDR_IN ClientAddr={0};
	int Length=sizeof(SOCKADDR_IN);
	while(1)
	{
		AcceptSocket=accept(this->ServeSocket,(SOCKADDR*)&ClientAddr,&Length);
		if(AcceptSocket==INVALID_SOCKET)
		{
			MessageBox(0,"accept函数调用失败!","程序提示",MB_OK|MB_ICONWARNING);
			return false;
		}
	   this->PublicICOP=CreateIoCompletionPort((HANDLE)AcceptSocket,this->PublicICOP,0,0);
	   if(this->PublicICOP==NULL)
	   {	
		    MessageBox(0,"CreateIoCompletionPort和连接的客户端关联失败!","程序提示",MB_OK|MB_ICONWARNING);
			return false;
	   }
	   IOContext*p=new IOContext(AcceptSocket,ClientAddr);
       this->AddIOContext(p);
	   DWORD dw=0;
	   int nRet=WSARecv(AcceptSocket,&(p->wsabuf),1,NULL,&dw,(LPWSAOVERLAPPED)p,NULL);
       
	   int lastErr=WSAGetLastError();
	   if(nRet==SOCKET_ERROR&&ERROR_IO_PENDING!=lastErr)
	   {
			 cout<<"WSARecv error"<<endl<<lastErr<<endl;
			 this->RemoveContext(p);
		
	   }
	  
	   
	  this->ShowClientInfo();

	   
	   
	}
	return true;
}
//遍历vector容器群发消息
void serveinfo::FSend(char*Buffer)
{
      vector<IOContext*>::iterator it=this->ClientArry.begin();
	  while(it!=this->ClientArry.end())
	  {
         int result=send((*it)->socket,Buffer,sizeof(Buffer),0);
		 if(result==SOCKET_ERROR)
		 {
			 closesocket((*it)->socket);
			 this->ClientArry.erase(it);
			 delete (*it);
		 }
		 it++;

	  }
}

//线程函数
DWORD WINAPI serveinfo::WorkThread(LPVOID lpvoid)
{   
    //L:
      static  serveinfo*STR=(serveinfo*)lpvoid;
      static IOContext*p=NULL;
      static FILE*pFile=NULL;
      static char FileName[1024]={0};
	  static int AllCount=0;
	  static int SendCount=0;
     
	 static DWORD dwBytes=0;
	 static DWORD ulKey=0;
	 static DWORD dwFlag=0;


     const char StartFlag[20]="文件_开始!";
	 const char EndFlag[20]="文件_结束!";
	 
      
	
    while(1)
	{

	    int  bOK=GetQueuedCompletionStatus(STR->PublicICOP,&dwBytes,&ulKey,(LPOVERLAPPED*)&p,INFINITE);
	    if(!bOK)
		{
             
			STR->RemoveContext(p);
			continue;
		}
        if(dwBytes==0)
		{
             cout<<"总共发送字节:"<<AllCount<<endl;
             fclose(pFile);//一定要记得关闭文件指针句柄
			 pFile=NULL;
			 AllCount=0;
			 SendCount=0;
			 continue;
		}

     //判断文件是否为开头或者结束
    /***********************************************************************************************************************************/
	  	if(strcmp(StartFlag,p->Buffer)==0)
		{
          cout<<"文件开始传输:"<<p->Buffer<<endl;   
		
          SYSTEMTIME sys={0};
          GetLocalTime(&sys);
          wsprintf(FileName,"C:\\Users\\Administrator\\Desktop\\%d%d%d%d%d%d%d%d.jpg",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
         // ::MessageBox(0,FileName,"eee",MB_OK);
          pFile=fopen(FileName,"wb");  

          DWORD Flags=0;
          ZeroMemory(p->Buffer,sizeof(p->Buffer));
          int nRet=WSARecv(p->socket,&(p->wsabuf),1,NULL,&Flags,p,NULL);
	      int lastError=WSAGetLastError();
	      if(nRet==SOCKET_ERROR&&WSA_IO_PENDING!=lastError)
		  {
			cout<<"WSARecv error"<<lastError<<endl;
			STR->RemoveContext(p);

		  }
        // GetFileName=true;
		 continue;  
		}

      

       if(strcmp(EndFlag,p->Buffer)==0)
		{
            //MessageBox(0,"文件_结束!","提示",MB_OK);
			cout<<"文件结束传输!"<<p->Buffer<<endl;
            cout<<"传输文件的次数:"<<SendCount<<endl;
			cout<<"总共发送字节:"<<AllCount<<endl;
			fclose(pFile);
			pFile=NULL;
			AllCount=0;
			SendCount=0;
          
            DWORD Flags=0;
	        ZeroMemory(p->Buffer,sizeof(p->Buffer));
	        int nRet=WSARecv(p->socket,&(p->wsabuf),1,NULL,&Flags,p,NULL);
	        int lastError=WSAGetLastError();
	        if(nRet==SOCKET_ERROR&&WSA_IO_PENDING!=lastError)
		   {
			cout<<"WSARecv error"<<lastError<<endl;
			STR->RemoveContext(p);

		   }
		
			continue;
		}

       
	
	     int a1=fwrite(p->Buffer,1,dwBytes,pFile);
		 cout<<"第"<<SendCount<<"次写入文件字节:"<<a1<<endl;
		 SendCount++;
		 AllCount+=a1;
      
      /*****************************************************************************************************************************************/
      //再投递WSARecv给完成端口
      /*****************************************************************************************************************************************/   
        DWORD Flags=0;
	    ZeroMemory(p->Buffer,sizeof(p->Buffer));
	    int nRet=WSARecv(p->socket,&(p->wsabuf),1,NULL,&Flags,p,NULL);
	    int lastError=WSAGetLastError();
	    if(nRet==SOCKET_ERROR&&WSA_IO_PENDING!=lastError)
		{
			cout<<"WSARecv error"<<lastError<<endl;
			STR->RemoveContext(p);

		}

	}

	return 0;
 }




	

//创建显示用户信息的线程
void serveinfo::ShowClientInfo()
{

     system("cls");
     cout<<"当前连接客户数目为:"<<this->ClientArry.size()<<endl;
	 cout<<"客户端Socket句柄"<<"   " <<"客户端IP地址"<<endl;
	 this->GetClientInof();
	
   	
}

//增加客户端
void serveinfo::AddIOContext(IOContext*p)
{
	EnterCriticalSection(&this->cs);
	this->ClientArry.push_back(p);
	LeaveCriticalSection(&cs);
}
//删除指定客户端
void serveinfo::RemoveContext(IOContext*p)
{
	EnterCriticalSection(&cs);
    vector<IOContext*>::iterator it=this->ClientArry.begin();
	while(it!=this->ClientArry.end())
	{
	    if((*it)==p)
		{
		closesocket((*it)->socket);
		this->ClientArry.erase(it);
		break;
		}
		it++;
	}
	LeaveCriticalSection(&cs);
}

//后去获取客户端的信息
void serveinfo::GetClientInof()
{
	EnterCriticalSection(&cs);
	vector<IOContext*>::iterator it=this->ClientArry.begin();
	while(it!=this->ClientArry.end())
	{
		cout<<(*it)->socket<<"                "<<inet_ntoa((*it)->ClientAddr.sin_addr)<<endl;
		it++;
	}
	LeaveCriticalSection(&cs);
}
//清空资源
void serveinfo::ClearResource()
{   
	EnterCriticalSection(&cs);
	vector<IOContext*>::iterator it=this->ClientArry.begin();
	while(it!=this->ClientArry.end())
	{
		closesocket((*it)->socket);
		delete (*it);
		it++;
	}
	this->ClientArry.clear();
	closesocket(this->ServeSocket);
	LeaveCriticalSection(&cs);
    DeleteCriticalSection(&cs);
}