#include "head.h"
#include <iostream>
using namespace std;
const int DIS_PORT = 7800;
const int SER_PORT = 8000;
const char* ip = "127.0.0.1";

class CSockInit
{
public:
	static CSockInit* GetSockfd(){
		return &_psock;
	}
		
	void bind_sock_init(){
		_bind_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		assert(_bind_sockfd != -1);

		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(DIS_PORT);
		saddr.sin_addr.s_addr = inet_addr(ip);
		int res = bind(_bind_sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
		if(res == -1){
			cout << "bind error" << endl;
			throw "bind error";
		}
		listen(_bind_sockfd, 5);
	}
	
	int connect_sock_init(){
		_connect_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		assert(_connect_sockfd != -1);

		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(SER_PORT);
		saddr.sin_addr.s_addr = inet_addr(ip);
		int res = connect(_connect_sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
		if(res == -1){
			cout << "connect error" << endl;
			throw "bind error";
		}
		return _connect_sockfd;
	}

	int Accept(struct sockaddr* addr, socklen_t *len){
		int c = accept(_bind_sockfd, addr, len);	
		if(c < 0){
			cout << "--------" << endl;
			throw "accept error\n";
		}
		return c;
	}
	
	ssize_t Send(int c, char *buff, const int size){
		return send(c, buff, size, 0);
	}
	
	ssize_t Recv(int c, char *buff, const int size){
		return recv(c, buff, size, 0);
	}

	~CSockInit(){
		close(_connect_sockfd);
		close(_bind_sockfd);
	}

private:
	
	CSockInit(){}
	int _connect_sockfd;
	int _bind_sockfd;
	static CSockInit _psock;
};
CSockInit CSockInit::_psock;

int main(){
	CSockInit *psock = CSockInit::GetSockfd();
	
	try{
		psock->bind_sock_init();
		int connect_sockfd = psock->connect_sock_init();
	}catch(const char *str){
		cout << str << endl;
		return 0;
	}
		
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t len = sizeof(addr);

	bool isRuning = true;
	while(isRuning)
	{
		int c = psock->Accept((struct sockaddr*)&addr, &len);
		if(c < 0)
			isRuning = false;
		cout << "send  " << c << "   to  " << connect_sockfd << endl;
		char *buff = new char[1024];
		int size;
		if((size = psock->Recv(c, buff, 1023)) < 0)
		{
			isRuning = false;
			close(c);
		}	
		psock->Send(connect_sockfd, buff, strlen(buff));

		delete[]buff;
	}
	return 0;
}
