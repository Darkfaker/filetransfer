#ifndef _DIR_H
#define _DIR_H

#include "STL"
using namespace std;
typedef unsigned long long UNLL;

/*dir informations*/
class CFileDirInfo{
public:
	CFileDirInfo(){}
	CFileDirInfo(bool is_file, string file_name, string md5)
		: _is_file(is_file)
		, _file_name(file_name)
		, _md5(md5){}

	CFileDirInfo& operator=(const CFileDirInfo &src){
		_is_file = src._is_file;
		_file_name = src._file_name;
		return *this;
	}

	bool operator==(const CFileDirInfo &src){
		return _file_name == src._file_name;
	}
	bool operator>(const CFileDirInfo &src){
		return _file_name > src._file_name;
	}

	bool operator<(const CFileDirInfo &src){
		return _file_name < src._file_name;
	}

	~CFileDirInfo(){}
	bool _is_file;
	string _file_name;
	string _md5;
};

/*file dir control*/
class CFileDir : public CFileDirInfo{
public:
	typedef list<CFileDirInfo> _ListType;
	CFileDir(string p, string Dir_name)
		: _Dir_name(Dir_name)
		, _pre_Dir(p)
	{
		_Dir_List = new _ListType;
	}

	void insert_file(const CFileDirInfo &m){
		_Dir_List->push_front(m);
	}

	void del_file(const CFileDirInfo &m){
		_ListType::iterator it = find(_Dir_List->begin(), _Dir_List->end(), m);
		_Dir_List->erase(it);
	}

	_ListType::iterator find_file(const CFileDirInfo &m){
		return find(_Dir_List->begin(), _Dir_List->end(), m);
	}

	_ListType::iterator find_file(const string &m){
		_ListType::iterator it = _Dir_List->begin();
		for (; it != _Dir_List->end(); ++it){
			if (it->_file_name == m)
				return it;
		}
		return _Dir_List->end();
	}

	_ListType::iterator get_begin(){
		return _Dir_List->begin();
	}

	_ListType::iterator get_end(){
		return _Dir_List->end();
	}
	void show_File_Dir();

	void show_Filr_Dir_All();

	~CFileDir(){}

    /*get*/
	string &get_pre_dir(){
		return _pre_Dir;
	}

	bool is_exist(const string &m){
		return find_file(m) != _Dir_List->end();
	}

private:
	string _Dir_name;
	string _pre_Dir;
	list<CFileDirInfo> *_Dir_List;
};

/*main dir control*/
class CDir{
public:
	typedef unordered_map<string, CFileDir> _MapType;
	static CDir *GetDirMap(){ return &_CDir; }
	~CDir(){}

	void insert_dir(const string &m, const CFileDir &f){
		_Dir_map.insert(make_pair(m, f));
	}

	void del_dir(const string &m){
		_Dir_map.erase(m);
	}

	_MapType::iterator find_dir(const string &m){
		return _Dir_map.find(m);
	}

	CFileDir::_ListType::iterator find_all(const string &m){
		_MapType::iterator it = _Dir_map.begin();

		for (; it != _Dir_map.end(); ++it){
			CFileDir::_ListType::iterator fileit = it->second.find_file(m);
			if (fileit != it->second.get_end())
				return fileit;

		}
		return find_dir(_while_Dir)->second.get_end();
	}

	CFileDir::_ListType::iterator get_file_end(){
		return find_dir(_while_Dir)->second.get_end();
	}

	_MapType::iterator get_begin(){
		return _Dir_map.begin();
	}

	_MapType::iterator get_end(){
		return _Dir_map.end();
	}
	void show_Dir(const string &m);

	void show_Dir_All();

	string &whilch_dir(){
		return _while_Dir;
	}

	void change_dir(const char *m){
		_while_Dir.replace(_while_Dir.begin()
			, _while_Dir.end(), m);
	}

private:
	string _while_Dir;
	UNLL _Dir_count;
	unordered_map<string, CFileDir> _Dir_map;
	static CDir _CDir;
	CDir(string _pre_Dir)
		: _Dir_count(1), _while_Dir("ftpser"){}
};
CDir CDir::_CDir;

inline void CFileDir::show_File_Dir(){
	_ListType::iterator it = _Dir_List->begin();
	int i = 0;

	for (; it != _Dir_List->end(); ++it){
		cout << "dir:>>" << endl;
		if (!it->_is_file){
			if (i++ == 5)
				cout << endl;
			cout << it->_file_name << " ";
		}
	}

	for (it = _Dir_List->begin(); it != _Dir_List->end(); ++it){
		cout << "dir:>>" << endl;
		if (it->_is_file){
			if (i++ == 5)
				cout << endl;
			cout << it->_file_name << " ";
		}
	}
	cout << endl;
}

/*dir show*/
inline void CDir::show_Dir(const string &m){
	_MapType::iterator it = _Dir_map.find(m);
	if (it == _Dir_map.end())
		throw "not find this Dir!";
	it->second.show_File_Dir();
}

inline void CDir::show_Dir_All(){
	_MapType::iterator it = _Dir_map.begin();
	cout << "- " << it->first << "----" << endl;
	for (; it != _Dir_map.end(); ++it){
		it->second.show_Filr_Dir_All();
		it->second.show_File_Dir();
	}
}

#endif

