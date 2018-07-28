#include "FTP.h"
#include "sockpair.h"
#include "STL"
#include "Dir.h"
#include "LinuxFun.h"
#include <iostream>
using namespace std;
typedef unsigned long long UNLL;

void send_file(int, int, int);
void recv_file(int, int, int);

const int B_SIZE = 1024;
//fork control
class ForkExecv{
public:
	ForkExecv(){}
	~ForkExecv(){}
    /*fork sockpair control*/
	string Fork_pro_comd(string comd){
		char buff[B_SIZE];
		string tmpbuff;
		FILE  *fp;
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

//file control class
class CFile{
public:
	typedef list<string> _Mytype;
	CFile(){}
	CFile(string file_name)
		: _file_name(file_name)
		, _link_count(0)
	{
		_file_list = new _Mytype;
	}
	// find file
	bool find_file(const string &file_name)
	{
		_Mytype::iterator it = find(_file_list->begin(), _file_list->end(), file_name);
		return it != _file_list->end();
	}
	// insert file
	void insert_file(const string &file_name){
		_file_list->push_front(file_name);
		++_link_count;
	}
	//del file
	void del_file(const string &file_name){
		_Mytype::iterator it = find(_file_list->begin(), _file_list->end(), file_name);
		_file_list->erase(it);
		--_link_count;
	}

	string _file_name;
	list<string> *_file_list;
	UNLL _link_count;
};

//MD5 control class
class CMd5 : public ForkExecv{
public:
	typedef unordered_map<string, CFile> _Mytype;
	
	static CMd5 *GetMd5(){ return &_md5; }
	~CMd5(){}
	string make_md5(const string &file_name){
		_comd += file_name;
		_md5_Str = ForkExecv::Fork_pro_comd(_comd);
	}
	//insert md5
	void insert_md5(const string &m, const CFile &file){
		_Md5_map.insert(make_pair(m, file));
	}
	//del md5
	void del_md5(const string &m){
		_Md5_map.erase(m);
	}
	//find md5
	_Mytype::iterator find_md5(const string &m){
		return _Md5_map.find(m);
	}
	//get end
	_Mytype::iterator get_end(){
		return _Md5_map.end();
	}
	//get begin
	_Mytype::iterator get_begin(){
		return _Md5_map.begin();
	}
private:
	unordered_map<string, CFile> _Md5_map;
	CMd5() :_comd("md5sum"){};
	string _comd;
	static CMd5 _md5;
	string _md5_Str;
};
CMd5 CMd5::_md5;

/*cli clasa control*/
class CProecss : public ForkExecv, public CFile{
public:
	typedef typename CMd5::_Mytype _MapType;
	CProecss(){
		_md5 = CMd5::GetMd5();
		_Dir = CDir::GetDirMap();
	}
	void init(int epollfd, int sockfd, struct sockaddr_in addr){
		_sockfd = sockfd;
	}
	void process();

	void Put_Ctl(char **myargv);

	void Get_Ctl(char **myargv);

	void Dir_Ctl(char **myargv);

	void Dir_file_Add(char **myargv, bool isfile = true);

	void show_ctl(char **myargv, bool isall = false);

	void Dir_Add(char **myargv);
private:
	string _comd;//cmd
	string _file_name;//file 
	int _port_addr;//addr
	int _sockfd;
	CMd5 *_md5;
	string _md5_str;
	bool _need_put;
	CDir *_Dir;
};

/*main loop*/
void CProecss::process(){
	cout << "run >> " << endl;
	char *myargv[10];
	char *comd = new char[128]();
	recv(_sockfd, comd, 128, 0);
	char *p = strtok(comd, " ");

	myargv[0] = p;
	int i = 0;
	while (NULL != (p = strtok(NULL, " ")))
		myargv[++i] = p;
	strcpy(comd, myargv[0]);

	_comd = comd;
	string path("/bin/");
	string cmd(myargv[0]);
	if (cmd == "get"){
		Get_Ctl(myargv);
		return;
	}
	else if (cmd == "put"){
		Put_Ctl(myargv);
		Dir_file_Add(myargv);
		return;
	}
	else if (cmd == "cd"){
		Dir_Ctl(myargv);
		return;
	}
	else if (cmd == "cp")
		return;
	else if (cmd == "ls")
		Dir_Ctl(myargv);
	else if (cmd == "mkdir")
		Dir_file_Add(myargv, false);
	else{
		string recv_buff = ForkExecv::Fork_pro_comd(_comd);
		send(_sockfd, recv_buff.c_str(), recv_buff.length(), 0);
		return;
	}
}

/*put file control*/
inline void CProecss::Put_Ctl(char **myargv){
	_MapType::iterator it = _md5->find_md5(string(myargv[2]));

	if (it == _md5->get_end()){
		int fw = open(myargv[1], O_WRONLY | O_CREAT, 0600);
		if (fw == -1)
			return;
		int file_size;
		sscanf(myargv[3], "%d", &file_size);
		CFile file = CFile(string(myargv[1]));
		file.insert_file(string(myargv[1]));
		_md5->insert_md5(string(myargv[2]), file);
		recv_file(_sockfd, file_size, fw);
	}
	else{
		it->second.insert_file(string(myargv[1]));
		send(_sockfd, "no", 2, 0);
		cout << "we have this file ......." << endl;
	}
}

/*get file control*/
inline void CProecss::Get_Ctl(char **myargv){
	_MapType::iterator it = _md5->find_md5(string(myargv[1]));
	if (it != _md5->get_end())
		send(_sockfd, "no file", 7, 0);
	else{
		cout << "open file " << endl;
		int fd = open(myargv[1], O_RDONLY);
		if (fd == -1){
			cout << "------have no file!----" << endl;
			return;
		}
		int size = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
		char buff[32] = { 0 };
		sprintf(buff, "%d", size);
		cout << "buff :" << endl;
		send(_sockfd, buff, 32, 0);
		send_file(_sockfd, fd, size);
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

	int res = bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));

	listen(sockfd, 5);
	return sockfd;
}

typedef unsigned long long UNLL;
/*reve file control*/
inline void recv_file(int sockfd, int file_size, int fw){
	cout << "-------put file......-------" << endl;
	cout << "file_size:" << file_size << endl;
	send(sockfd, "ok", 2, 0);
	int fin_size(0);
	int numb(0);
	char *recv_buff = new char[1024]();

	while ((numb = recv(sockfd, recv_buff, 1024, 0)) > 0){
		fin_size += numb;
		write(fw, recv_buff, numb);
		memset(recv_buff, 0, 1024);
		if (fin_size == file_size)
			break;
	}

	cout << "-------put finsh!-------" << endl;

	delete[]recv_buff;
	close(fw);
}

/*send file control*/
inline void send_file(int sockfd, int fd, int file_size){
	cout << "send_file......." << endl;
	char buff[2];
	recv(sockfd, buff, 2, 0);
	sendfile(sockfd, fd, NULL, file_size);
	cout << "send_file fin!" << endl;
	close(fd);
}

/*Dir control loop*/
inline void CProecss::Dir_Ctl(char **myargv){
	string w_dir = _Dir->whilch_dir();
	CDir::_MapType::iterator mapit = _Dir->find_dir(w_dir);
	string dir(myargv[1]);
	if (dir == "."){
		_Dir->change_dir(myargv[1]);
		_Dir->show_Dir(string(myargv[1]));
	}
	else if (dir == ".."){
		_Dir->change_dir(mapit->second.get_pre_dir().c_str());
		_Dir->show_Dir(string(myargv[1]));
	}
	else if (dir[0] == (char)"/"){
		char *dir[10] = { 0 };
		char *p = strtok(myargv[1], "/");
		if (NULL == p)
			return;
		dir[0] = p;
		int i = 0;
		while (NULL != (p = strtok(NULL, " ")))
			dir[++i] = p;
		CFileDir::_ListType::iterator it;
		CDir::_MapType::iterator it2;
		for (int j = 0; j < i; ++j){
			it2 = _Dir->find_dir(string(dir[j]));
			if (it2 == _Dir->get_end()){
				string buff("this is a file, not a dir!");
				cout << buff << endl;
				return;
			}
		}
		_Dir->change_dir(dir[i - 1]);
	}
	else{
		_Dir->change_dir(myargv[1]);
		_Dir->show_Dir(string(myargv[1]));
	}
}

/*dir file add control*/
inline void CProecss::Dir_file_Add(char **myargv, bool isfile = true){
	string w_dir = _Dir->whilch_dir();
	CDir::_MapType::iterator mapit = _Dir->find_dir(w_dir);

	if (mapit == _Dir->get_end()){
		cout << "not find this Dir!" << endl;
		throw "not find this Dir!";
	}
	else{
		CFileDirInfo fileinfo(isfile, string(myargv[1]), string(myargv[2]));
		CFileDir::_ListType::iterator fileit = mapit->second.find_file(fileinfo);
		if (fileit == mapit->second.get_end()){
			if (isfile)
				mapit->second.insert_file(fileinfo);
			else
				_Dir->insert_dir(string(myargv[1]), CFileDir(w_dir, string(myargv[1])));
		}
		else{
			string buff("There is one file have the same name!");
			cout << buff << endl;
			//send();
		}
	}
}

inline void CProecss::show_ctl(char **myargv, bool isall = false){
	string cmd(myargv[1]);

	_Dir->show_Dir_All();
}
