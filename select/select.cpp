#include"select_server.h"
using namespace std;
int main()
{

	WSADATA data;
	WSAStartup(MAKEWORD(2,2),&data);
	if(2!=LOBYTE(data.wHighVersion)||2!=HIBYTE(data.wVersion))
	{
	cout<<"��ʼ���׽��ֿ�ʧ��!"<<endl;
	return 0;
	}
	Server object;
	object.DoWork();
	return 0;
}