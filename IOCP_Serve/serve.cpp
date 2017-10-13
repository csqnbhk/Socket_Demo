#include<iostream>
#include<winsock2.h>
#include<windows.h>
#include<time.h>
#include"serve.h"
using namespace std;

//���캯��
serveinfo::serveinfo()
{
  
	this->ServePort=1680;
	this->ServeSocket=NULL;
	this->PublicICOP=INVALID_HANDLE_VALUE;
	memset(this->ThreadArry,0,sizeof(this->ThreadArry));
	memset(&this->data,0,sizeof(data));
	InitializeCriticalSection(&this->cs);
	

}
//��������
serveinfo::~serveinfo()
{
   
}
//��ʼ��
bool serveinfo::Initialize()
{
   int ReturnResult=WSAStartup(MAKEWORD(2,2),&this->wsadata);
   if(ReturnResult!=0)
   {
	   MessageBox(0,"����WSAStartup()����ʧ��!","������ʾ",MB_OK|MB_ICONWARNING);
	   return false;
   }
   if(2!=LOBYTE(wsadata.wVersion)||2!=HIBYTE(wsadata.wVersion))
   {
	   MessageBox(0,"�׽��ְ汾����!","������ʾ",MB_OK|MB_ICONWARNING);
	   return false;
   }
   
   return true;
}
//���������socket��������
SOCKET&serveinfo::Listen()
{
   this->ServeSocket=socket(AF_INET,SOCK_STREAM,0);
   if(this->ServeSocket==INVALID_SOCKET)
   {
	   MessageBox(0,"����socketʧ��!","������ʾ",MB_OK|MB_ICONWARNING);
	   return this->ServeSocket;
   }
   this->ServeAddr.sin_port=htons(this->ServePort);
   this->ServeAddr.sin_family=AF_INET;
   this->ServeAddr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
   int bindresult=bind(this->ServeSocket,(SOCKADDR*)&this->ServeAddr,sizeof(this->ServeAddr));
   if(bindresult==SOCKET_ERROR)
   {
	   MessageBox(0,"bind��������ʧ��!","������ʾ",MB_OK|MB_ICONWARNING);
	   return this->ServeSocket;
   }
   int listenresult=listen(this->ServeSocket,SOMAXCONN);
   if(listenresult==SOCKET_ERROR)
   {
	   MessageBox(0,"listen��������ʧ��!","������ʾ",MB_OK|MB_ICONWARNING);
	   return this->ServeSocket;
   }
   return this->ServeSocket;
}
//������ɶ˿ڲ��ͷ�����socket��������ʵ���Բ��ѷ�����socket�Ž�����ɶ˿�,��ε���AcceptExҪ��������Ͷ��WSAAccept��
void serveinfo::CreateICOPAndThreadPool()
{
    this->PublicICOP=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	if(this->PublicICOP==NULL)
	{
		MessageBox(0,"������ɶ˿�ʧ��!","������ʾ",MB_OK|MB_ICONWARNING);
		return ;
	}
    /*HANDLE TempHandle=CreateIoCompletionPort((HANDLE)this->ServeSocket,this->PublicICOP,0,0);
	if(TempHandle==NULL)
	{
		MessageBox(0,"��ɶ˿ںͷ������׽��ֹ���ʧ��!","������ʾ",MB_OK|MB_ICONWARNING);
		return ;
	}
    */
	GetSystemInfo(&this->systeminfo);
	for(int i=0;i<this->systeminfo.dwNumberOfProcessors*2;i++)
	{
        this->ThreadArry[i]=CreateThread(0,0,WorkThread,this,0,0);//����̬���̺߳�������thisָ�룬���Ե�����ĳ�Ա��������̬��Ա��������ֻ���Ե���
		CloseHandle(this->ThreadArry[i]);
      
	}
  

}
//�ȴ����տͻ��˵�����
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
			MessageBox(0,"accept��������ʧ��!","������ʾ",MB_OK|MB_ICONWARNING);
			return false;
		}
	   this->PublicICOP=CreateIoCompletionPort((HANDLE)AcceptSocket,this->PublicICOP,0,0);
	   if(this->PublicICOP==NULL)
	   {	
		    MessageBox(0,"CreateIoCompletionPort�����ӵĿͻ��˹���ʧ��!","������ʾ",MB_OK|MB_ICONWARNING);
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
//����vector����Ⱥ����Ϣ
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

//�̺߳���
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


     const char StartFlag[20]="�ļ�_��ʼ!";
	 const char EndFlag[20]="�ļ�_����!";
	 
      
	
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
             cout<<"�ܹ������ֽ�:"<<AllCount<<endl;
             fclose(pFile);//һ��Ҫ�ǵùر��ļ�ָ����
			 pFile=NULL;
			 AllCount=0;
			 SendCount=0;
			 continue;
		}

     //�ж��ļ��Ƿ�Ϊ��ͷ���߽���
    /***********************************************************************************************************************************/
	  	if(strcmp(StartFlag,p->Buffer)==0)
		{
          cout<<"�ļ���ʼ����:"<<p->Buffer<<endl;   
		
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
            //MessageBox(0,"�ļ�_����!","��ʾ",MB_OK);
			cout<<"�ļ���������!"<<p->Buffer<<endl;
            cout<<"�����ļ��Ĵ���:"<<SendCount<<endl;
			cout<<"�ܹ������ֽ�:"<<AllCount<<endl;
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
		 cout<<"��"<<SendCount<<"��д���ļ��ֽ�:"<<a1<<endl;
		 SendCount++;
		 AllCount+=a1;
      
      /*****************************************************************************************************************************************/
      //��Ͷ��WSARecv����ɶ˿�
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




	

//������ʾ�û���Ϣ���߳�
void serveinfo::ShowClientInfo()
{

     system("cls");
     cout<<"��ǰ���ӿͻ���ĿΪ:"<<this->ClientArry.size()<<endl;
	 cout<<"�ͻ���Socket���"<<"   " <<"�ͻ���IP��ַ"<<endl;
	 this->GetClientInof();
	
   	
}

//���ӿͻ���
void serveinfo::AddIOContext(IOContext*p)
{
	EnterCriticalSection(&this->cs);
	this->ClientArry.push_back(p);
	LeaveCriticalSection(&cs);
}
//ɾ��ָ���ͻ���
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

//��ȥ��ȡ�ͻ��˵���Ϣ
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
//�����Դ
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