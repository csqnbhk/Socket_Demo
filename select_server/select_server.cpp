
#include"select_server.h"

Server::Server()
{
	this->DefaultPort=1680;
	this->ServerSocket=INVALID_SOCKET;
	memset(&this->ServerAddr,0,sizeof(this->ServerAddr));
   
	
	this->conns.clear();
	FD_ZERO(&this->fdExcept);
	FD_ZERO(&this->fdWrite);
	FD_ZERO(&this->fdRead);
	this->exit=1;
}
//���ӿͻ���
void Server::AddClient(SOCKET clientsocket)
{
    
	this->conns.push_back(new ClientInfo(clientsocket));
	cout<<"���ӿͻ��˳ɹ�!"<<endl;
	 
}
//ɾ���ͻ���
void Server::DelClient(SOCKET clientsocket)
{
	vector<ClientInfo*>::iterator it=this->conns.begin();
	while(it!=this->conns.end())
	{

		ClientInfo*p=*it;
		if(p->ClientSocket==clientsocket)
		{
            closesocket(p->ClientSocket);
			this->conns.erase(it);
            delete p;
			cout<<"ɾ�����ִ������ӿͻ��˳ɹ�!"<<endl;
			return ;
		}
        it++;
	}
   
}


//�󶨼��������׽���
SOCKET Server::BindListen()
{

     this->ServerSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	 if(this->ServerSocket==INVALID_SOCKET)
	 {
		 cout<<"Create server socket failed:"<<GetLastError()<<endl;
		 return this->ServerSocket;
	 }
    
     this->ServerAddr.sin_family=AF_INET;
	 this->ServerAddr.sin_port=htons(this->DefaultPort);
	 this->ServerAddr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");

	 int bind_return=bind(this->ServerSocket,(SOCKADDR*)&this->ServerAddr,sizeof(this->ServerAddr));
     if(bind_return==SOCKET_ERROR)
	 {
		 cout<<"Server bind failed:"<<GetLastError()<<endl;
		 this->ServerSocket=INVALID_SOCKET;
		 return this->ServerSocket;
	 }
     int listen_return=listen(this->ServerSocket,SOMAXCONN);
     if(listen_return==SOCKET_ERROR)
	 {
		 cout<<"Server listen failed:"<<GetLastError()<<endl;
		 this->ServerSocket=INVALID_SOCKET;
		 return this->ServerSocket;
	 }
	u_long nNoBlock=1;
    if(ioctlsocket(this->ServerSocket,FIONBIO,&nNoBlock)==SOCKET_ERROR)
	{
		cout<<"ioctlsocket() failed!"<<endl;

	}
	
	cout<<"�����׽���׼���ɹ�!"<<endl;

	return this->ServerSocket;
}
//�������ü���
void Server::ResetFDSet()
{

    FD_ZERO(&this->fdExcept);
	FD_ZERO(&this->fdWrite);
	FD_ZERO(&this->fdRead);

    FD_SET(this->ServerSocket,&this->fdRead);
	FD_SET(this->ServerSocket,&this->fdExcept);
    
	vector<ClientInfo*>::iterator it=this->conns.begin();
    while(it!=conns.end())
	{
       ClientInfo*p=*it;

	   FD_SET(p->ClientSocket,&this->fdExcept);
	   FD_SET(p->ClientSocket,&this->fdRead);
	   FD_SET(p->ClientSocket,&this->fdWrite);

	   it++;
	}
  //cout<<"�������ü��ϳɹ�!"<<endl;

}
//����Ƿ����µĿͻ�������
void Server::CheckAccept()
{

	if(FD_ISSET(this->ServerSocket,&this->fdExcept))
	{
		cout<<"�������׽��ֳ��ִ���:"<<endl;
		return ;
	}
	if(FD_ISSET(this->ServerSocket,&this->fdRead))
	{
		if(this->conns.size()>=63)
        {
			cout<<"��ǰ�������Ѵ�63�����������ӽ����ͻ���!"<<endl;
			return ;
		}

		SOCKET ClientSocket;
        SOCKADDR_IN ClientAddr;
		int size=sizeof(ClientAddr);
        ClientSocket=accept(this->ServerSocket,(SOCKADDR*)&ClientAddr,&size);
		if(ClientSocket!=INVALID_SOCKET)
		{	
		    u_long nNoBlock=1;
           if(ioctlsocket(this->ServerSocket,FIONBIO,&nNoBlock)==SOCKET_ERROR)
		   {
		      cout<<"ioctlsocket() failed!"<<endl;

		   }
            this->AddClient(ClientSocket);
			
		}
     
	}
}
//���ͻ��˵ļ����Ƿ��б仯
void Server::CheckClient()
{
	
	vector<ClientInfo*>::iterator it=this->conns.begin();

	while(it!=this->conns.end())
	{
       ClientInfo*p=*it;
       it++;
	   if(FD_ISSET(p->ClientSocket,&this->fdExcept))
	   {
		   cout<<"��ǰ�ͻ��˳������Ӵ���,ɾ�������ӳɹ�!"<<endl;
		   this->DelClient(p->ClientSocket);
		   FD_CLR(p->ClientSocket,&this->fdExcept);
		   FD_CLR(p->ClientSocket,&this->fdWrite);
		   FD_CLR(p->ClientSocket,&this->fdRead);
		   break;
	   }
       if(FD_ISSET(p->ClientSocket,&this->fdRead))
	   {
             int nByte=recv(p->ClientSocket,p->Buffer,sizeof(p->Buffer),0);
		     if(nByte==SOCKET_ERROR)
			 {
               
				 cout<<"���տͻ�����Ϣʧ��:"<<GetLastError()<<"ɾ���ÿͻ��˳ɹ�!"<<endl;
				 this->DelClient(p->ClientSocket);
                 FD_CLR(p->ClientSocket,&this->fdExcept);
		         FD_CLR(p->ClientSocket,&this->fdWrite);
		         FD_CLR(p->ClientSocket,&this->fdRead);
				 break;
			 }
			 cout<<"����ClientSocket:"<<p->ClientSocket<<"��Ϣ:"<<p->Buffer<<endl;
		
		
			
	   }
       if(FD_ISSET(p->ClientSocket,&this->fdWrite))
	   {
		   int nByte=send(p->ClientSocket,p->Buffer,sizeof(p->Buffer),0);
		   if(nByte==SOCKET_ERROR)
		   {
				cout<<"���͸��ͻ�����Ϣʧ��:"<<GetLastError()<<"ɾ���ÿͻ��˳ɹ�!"<<endl;
                this->DelClient(p->ClientSocket);
                FD_CLR(p->ClientSocket,&this->fdExcept);
		        FD_CLR(p->ClientSocket,&this->fdWrite);
		        FD_CLR(p->ClientSocket,&this->fdRead);
				break;
			}
		   cout<<"����fdWrite"<<endl;
           //cout<<"����ClientSocket:"<<p->ClientSocket<<"��Ϣ:"<<p->Buffer<<endl;
		  // memset(p->Buffer,0,sizeof(p->Buffer));
	   }
     
      
	}


}
//������Դ
void Server::ClearResource()
{
	vector<ClientInfo*>::iterator it=this->conns.begin();
	while(it!=this->conns.end())
	{
		ClientInfo*p=*it;
		closesocket(p->ClientSocket);
		delete p;
		this->conns.erase(it);
		it++;
	}
	closesocket(this->ServerSocket);
	cout<<"������Դ���!"<<endl;
}
void Server::DoWork()
{
	this->ServerSocket=this->BindListen();
	while(this->exit)
	{
		this->ResetFDSet();
		int nRet=select(0,&this->fdRead,&this->fdWrite,&this->fdExcept,NULL);
		if(nRet<=0)
		{
			cout<<"select() failed"<<WSAGetLastError()<<endl;
			break;
		}
		this->CheckAccept();
		this->CheckClient();
	}
    this->ClearResource();

}