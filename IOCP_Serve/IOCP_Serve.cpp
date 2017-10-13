#include<iostream>
#include<winsock2.h>
#include<windows.h>
#include"serve.h"

serveinfo object;
using namespace std;
int main()
{
  
  object.Initialize();
  object.Listen();
  object.CreateICOPAndThreadPool();
  //serveinfo::WorkThread(lpvoid);
   object.AcceptClient();
   object.ClearResource();
	return 0;
}