#define _CRT_SECURE_NO_WARNINGS
#include "LinuxFun"
#include "STL"
#include "sockpair.h"
#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

int sockfd_init();
typedef unsigned long long UNLL;
void put_file(int, int, int);
void get_file(int, int, int);

const int B_SIZE = 1024;
//fork and iostream
class ForkExecv{
public:
	ForkExecv(){}
	~ForkExecv(){}
    /*fork sockpair control*/
	string Fork_pro_comd(const string &comd){
		char buff[B_SIZE];
		string tmpbuff;
		FILE  *fp;
		comd;
		fp = dpopen(comd.c_str());
		if (fp == NULL){
			perror("dpopen error");
			exit(1);
		}
		if (dphalfclose(fp) < 0){
			perror("dphalfclose error");
			exit(1);
		}
		while (1){
			if (fgets(buff, B_SIZE, fp) == NULL)
				break;
			tmpbuff += buff;
		}
		return string(tmpbuff);
		dpclose(fp);
	}
private:
};
//MD5 Control
class CMd5 : public ForkExecv{
public:
	typedef unordered_multimap<string, string> _Mytype;
	static CMd5 *GetMd5(){ return &_md5; }
	~CMd5(){}
    /*make md5*/
	string make_md5(const string &file_name){
		string _comd("md5sum");
		_comd += " ";
		_comd += file_name;
		_md5_Str = ForkExecv::Fork_pro_comd(_comd);

		string::iterator it = _md5_Str.begin();
		string tmp;

		int id = 0;
		while (_md5_Str[id] != '\n')
			++id;

		for (id; _md5_Str[id] != '\0'; ++id)
			tmp.push_back(_md5_Str[id]);
		tmp.erase(tmp.begin());
		id = file_name.size() + 3;
		while (id){
			tmp.pop_back();
			--id;
		}
		it = _comd.begin();
		for (; it != _comd.end(); ++it)
			it = _comd.erase(it);
		cout << "_md5_str: " << tmp << endl;
		return string(tmp);
	}
    /*insert*/
	void insert_md5(const string &m, const string &file){
		_Md5_map.insert(make_pair(m, file));
	}
    /*del*/
	void del_md5(const string &m){
		_Md5_map.erase(m);
	}
	_Mytype::iterator find_md5(const string &m){
		return _Md5_map.find(m);
	}
	_Mytype::iterator get_end(){
		return _Md5_map.end();
	}
	_Mytype::iterator get_begin(){
		return _Md5_map.begin();
	}
private:
	unordered_multimap<string, string> _Md5_map;
	CMd5(){};
	static CMd5 _md5;
	string _md5_Str;
};
CMd5 CMd5::_md5;

//cli control
class Client //:public CFile{
public:
	Client(){
		_sockfd = sockfd_init();
		_md5 = CMd5::GetMd5();
		_comd = new char[32]();
	}
	~Client(){}
	void cliRun();
	void putCtl(char **myargv);
	void getCtl(char **myargv);
private:
	int _sockfd;
	CMd5 *_md5;
	char *_comd;
};
/*Runing*/
void Client::cliRun(){
	while (1){
		cout << "+------------------------------------------+" << endl;
		cout << "input:" << endl;

		fgets(_comd, 32, stdin);
		_comd[strlen(_comd) - 1] = 0;

		cout << "_comd :" << _comd << endl;
		char *myargv[10] = { 0 };
		char *s = strtok(_comd, " ");
		if (NULL == s)
			continue;
		myargv[0] = s;
		int i = 0;
		while (NULL != (s = strtok(NULL, " ")))
			myargv[++i] = s;

		string cmd(myargv[0]);
		if (cmd == "end"){
			close(_sockfd);
			break;
		}
		else if (cmd == "clear"){
            system("pause");
			close(_sockfd);
		}
		else if (cmd == "get"){
			getCtl(myargv);
		}
		else if (cmd == "put"){
			putCtl(myargv);
		}
		else{
			char buff[2047];
			send(_sockfd, _comd, strlen(_comd), 0);
			int num = recv(_sockfd, buff, 2047, 0);
			if (num <= 0){
				close(_sockfd);
				continue;
			}
			printf("%s\n", buff);
		}
	}
}
/*send file*/
inline void put_file(int sockfd, int fd, int file_size){
	char buff[2];
	recv(sockfd, buff, 2, 0);/// recv 1
	if (strncmp(buff, "no", 2) == 0)
		return;
	sendfile(sockfd, fd, NULL, file_size);
	close(fd);
}
/*get file*/
inline void get_file(int sockfd, int file_size, int fd){
	cout << "get file......." << endl;
	send(sockfd, "ok", 2, 0);// send 1
	int fin_size(0);
	int numb(0);
	char *recv_buff = new char[1024]();
	while ((numb = recv(sockfd, recv_buff, 1024, 0)) > 0){
		fin_size += numb;
		write(fd, recv_buff, numb);
		memset(recv_buff, 0, 1024);
		if (fin_size == file_size)
			break;
	}
	delete[]recv_buff;
}
/*put fil control*/
inline void Client::putCtl(char **myargv){
	cout << "putctl:>>" << endl;
	string cmd(_comd);
	cmd += " ";
	cmd += myargv[1];
	cmd += " ";
	cmd += _md5->make_md5(string(myargv[1]));

	int fd = open(myargv[1], O_RDONLY);
	if (fd == -1){
		cout << "------have no file!" << endl;
		return;
	}
	int size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	char arr[32] = { 0 };
	sprintf(arr, "%d", size);
	cmd += " ";
	cmd += arr;

	send(_sockfd, cmd.c_str(), cmd.length(), 0);
	put_file(_sockfd, fd, size);
}
/*get fil control*/
inline void Client::getCtl(char **myargv){
	string cmd(myargv[0]);
	cmd += " ";
	cmd += myargv[1];
	cout << "cmd: " << cmd << endl;
	send(_sockfd, cmd.c_str(), cmd.length(), 0);

	char buff[32] = { 0 };
	recv(_sockfd, buff, 32, 0);
	cout << "buff :" << buff << endl;
	if (strncmp(buff, "no file", 7) == 0){
		cout << "this file is not exist!" << endl;
		return;
	}
	else{
		int size(0);
		sscanf(buff, "%d", &size);
		cout << "file_size: " << size << endl;
		int fd = open(myargv[1], O_WRONLY | O_CREAT, 0600);
		if (fd == -1)
			return;
		get_file(_sockfd, size, fd);
	}
}


int sockfd_init(){
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(6000);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int res = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
	assert(res != -1);
	return sockfd;
}

