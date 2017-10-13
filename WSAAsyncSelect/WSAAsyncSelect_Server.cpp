#include<winsock2.h>
#include<windows.h>
#include<iostream>
#include<vector>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

#define  WM_SOCK (WM_USER+168)
HWND hwnd;
//结构体保存客户端信息
struct ClientInfo
{
    SOCKET clientsocket;
	char Buffer[1024];
	ClientInfo(SOCKET socket):clientsocket(socket)
	{
		memset(Buffer,0,sizeof(Buffer));
	}
};

typedef vector<ClientInfo*> ConnectClient;
ConnectClient conns;

LRESULT CALLBACK MyWinProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);
int  WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
    //注册窗口
    WNDCLASS wnd={0};

    wnd.cbClsExtra=0;
	wnd.cbWndExtra=0;
	wnd.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
	wnd.hCursor=LoadCursor(hInstance,IDC_ARROW);
	wnd.hIcon=LoadIcon(hInstance,IDI_APPLICATION);
	wnd.hInstance=hInstance;
	wnd.lpfnWndProc=MyWinProc;
	wnd.lpszClassName="WinClass";
	wnd.lpszMenuName=0;
	wnd.style=CS_HREDRAW|CS_VREDRAW;


	if(RegisterClass(&wnd)==0)
    {
		cout<<"RegisterClass() failed :"<<GetLastError()<<endl;
		return -1;
	}
    hwnd=CreateWindow("WinClass","WSAAsyncSelect_Server",WS_OVERLAPPEDWINDOW,100,100,300,300,0,0,hInstance,0);
    ShowWindow(hwnd,SW_SHOW);
	if(hwnd==NULL)
	{
		cout<<"CreateWidnow() failed :"<<GetLastError()<<endl;
		return -1;
	}
    if(AllocConsole()!=0) 
	{
		freopen("conin$","r+t",stdin);
		freopen("conout$","w+t",stdout);
		freopen("couout$","w+t",stderr);
	}
	cout<<"创建窗体成功!"<<endl;
	
   //创建服务器套接字
  
	WSADATA data={0};
    WSAStartup(MAKEWORD(2,2),&data);
   if(2!=LOBYTE(data.wHighVersion)||2!=HIBYTE(data.wVersion))
   {
	   cout<<"WSAStartup() failed :"<<WSAGetLastError()<<endl;
	   return -1;
   }
    SOCKET ListenSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(ListenSocket==INVALID_SOCKET)
	{
		cout<<"socket() failed :"<<WSAGetLastError()<<endl;
		return -1;
	}
    
    SOCKADDR_IN addr={0};
	addr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
	addr.sin_family=AF_INET;
	addr.sin_port=htons(1680);
	
	int bind_return=bind(ListenSocket,(SOCKADDR*)&addr,sizeof(addr));
    if(bind_return==SOCKET_ERROR)
	{
		cout<<"bind() failed :"<<GetLastError()<<endl;
		return -1;
	}
	listen(ListenSocket,SOMAXCONN);
	WSAAsyncSelect(ListenSocket,hwnd,WM_SOCK,FD_ACCEPT);

	conns.push_back(new ClientInfo(ListenSocket));

    ClientInfo*p=*conns.begin();
	strcpy(p->Buffer,"欢迎你的到来!");




	////////////////////////////////////////
	MSG msg;
	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

	}

    return 1;


}
LRESULT CALLBACK MyWinProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{


	switch(Message)
	{
	    case WM_DESTROY:
	//	MessageBox(0,"win32","11",MB_OK);
		PostQuitMessage(0);
		break;
		case WM_SOCK:
			{

				SOCKET temp=(SOCKET)wParam;
                ClientInfo*p=*conns.begin();
				int err=WSAGETSELECTERROR(lParam);
				int fd_event=WSAGETSELECTEVENT(lParam);
				switch(fd_event)
				{
				 case FD_ACCEPT:
					 {
                    
					 SOCKET AcceptSocket=accept(p->clientsocket,0,0);
					 if(AcceptSocket==SOCKET_ERROR)
					 {
						 cout<<"accept() failed :"<<GetLastError()<<endl;
						 break;
					 }
					 WSAAsyncSelect(AcceptSocket,hwnd,WM_SOCK,FD_READ|FD_CLOSE);
					 conns.push_back(new ClientInfo(AcceptSocket));
					 cout<<"又有一个客户端连接进来"<<endl;
					 send(AcceptSocket,p->Buffer,sizeof(p->Buffer),0);
					break;
					 }
				 case FD_READ:
					 { 
                         memset(p->Buffer,0,sizeof(p->Buffer));
                         recv(temp,p->Buffer,sizeof(p->Buffer),0);
                         cout<<"接收到客户端Socket:"<<temp<<"的消息"<<p->Buffer<<endl;
					     break;
					 }
				 case FD_WRITE:
					break;
				 default:
				    break;
				}

			}
		break;
    	default:
		return DefWindowProc(hwnd,Message,wParam,lParam);
		break;
	}


  return 1;

}