
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
//增加客户端
void Server::AddClient(SOCKET clientsocket)
{
    
	this->conns.push_back(new ClientInfo(clientsocket));
	cout<<"增加客户端成功!"<<endl;
	 
}
//删除客户端
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
			cout<<"删除出现错误连接客户端成功!"<<endl;
			return ;
		}
        it++;
	}
   
}


//绑定监听服务套接字
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
	
	cout<<"监听套接字准备成功!"<<endl;

	return this->ServerSocket;
}
//重新设置集合
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
  //cout<<"重新设置集合成功!"<<endl;

}
//检查是否有新的客户端连接
void Server::CheckAccept()
{

	if(FD_ISSET(this->ServerSocket,&this->fdExcept))
	{
		cout<<"服务器套接字出现错误:"<<endl;
		return ;
	}
	if(FD_ISSET(this->ServerSocket,&this->fdRead))
	{
		if(this->conns.size()>=63)
        {
			cout<<"当前连接数已达63，不可再连接进来客户端!"<<endl;
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
//检查客户端的集合是否有变化
void Server::CheckClient()
{
	
	vector<ClientInfo*>::iterator it=this->conns.begin();

	while(it!=this->conns.end())
	{
       ClientInfo*p=*it;
       it++;
	   if(FD_ISSET(p->ClientSocket,&this->fdExcept))
	   {
		   cout<<"当前客户端出现连接错误,删除该连接成功!"<<endl;
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
               
				 cout<<"接收客户端信息失败:"<<GetLastError()<<"删除该客户端成功!"<<endl;
				 this->DelClient(p->ClientSocket);
                 FD_CLR(p->ClientSocket,&this->fdExcept);
		         FD_CLR(p->ClientSocket,&this->fdWrite);
		         FD_CLR(p->ClientSocket,&this->fdRead);
				 break;
			 }
			 cout<<"接收ClientSocket:"<<p->ClientSocket<<"信息:"<<p->Buffer<<endl;
		
		
			
	   }
       if(FD_ISSET(p->ClientSocket,&this->fdWrite))
	   {
		   int nByte=send(p->ClientSocket,p->Buffer,sizeof(p->Buffer),0);
		   if(nByte==SOCKET_ERROR)
		   {
				cout<<"发送给客户端信息失败:"<<GetLastError()<<"删除该客户端成功!"<<endl;
                this->DelClient(p->ClientSocket);
                FD_CLR(p->ClientSocket,&this->fdExcept);
		        FD_CLR(p->ClientSocket,&this->fdWrite);
		        FD_CLR(p->ClientSocket,&this->fdRead);
				break;
			}
		   cout<<"捕获到fdWrite"<<endl;
           //cout<<"发送ClientSocket:"<<p->ClientSocket<<"信息:"<<p->Buffer<<endl;
		  // memset(p->Buffer,0,sizeof(p->Buffer));
	   }
     
      
	}


}
//回收资源
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
	cout<<"回收资源完成!"<<endl;
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