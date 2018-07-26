
/******************************/
#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <memory>
#include <exception>
#include "md5.h"
using namespace std;
/******************************/
class Md5{
public:
	string get_Md5(const string &str){
		return MD5(str).toStr();
	}
};

template<typename T>
class CNode{
public:
	typedef vector<string> _Vec_Type;
	typedef T _Val_Type;
private:
	string _real_key;
	T _real_msg;
public:
	_Vec_Type _Vir_Node;
	CNode(string key, const T& msg);
	CNode(){}
	string Get_real_key();
	T Get_real_msg();
};

template<typename T>
class CNodeCtl{
public:
	typedef string (*MakeConnect)(const T&, int);
	typedef vector<string> _Vec_Type;
	typedef unordered_map<string, CNode<T>> _Map_Type;
	typedef T _Val_Type;
	_Map_Type _Node_map;
public:
	/* make a HOOK function */
	static MakeConnect MakeMsg;
	//static function<MakeConnect> MakeMsg;
	static void Set_Fun(MakeConnect cfunc);
	/* add node */
	string Add_Node(const T &msg);
	string Add_Vir_Node(string key, const int N);
	/* del node */
	vector<string> Del_Node(const T &msg);
	T Get_Msg(const string &key);
};
template<typename T>
typename CNodeCtl<T>::MakeConnect CNodeCtl<T>::MakeMsg = NULL;

template<typename T>
class CHash{
public:
	typedef unordered_map<string, string> _Map_Type;
	typedef T _Val_Type;
private:
	/* static map */
	_Map_Type _Ip_map;
	/* real node -> vir node*/
	CNodeCtl<T> _One_Node;
	/* init number */
	int _Init_count;	
public:
	/* static instance */
	void Add_ser(const T &msg);

	void Del_ser(const T &msg);

	T Find_ser(const T &msg);

	void Show_ser();

	CHash(int n, const vector<T> &vec);
};

//CNode code
/********************************************/
template<typename T>
inline CNode<T>::CNode(string key, const T &msg)
:_real_key(key), _real_msg(msg){}

template<typename T>
inline string CNode<T>::Get_real_key(){
	return _real_key;
}

template<typename T>
inline T CNode<T>::Get_real_msg(){
	return _real_msg;
}
/********************************************/

//CNodeCtl code
/********************************************/
template<typename T>
inline void CNodeCtl<T>::Set_Fun(MakeConnect cfunc){
	if (NULL == cfunc) 
        throw string("NULL == cfunc\n");
	MakeMsg = cfunc;
}

template<typename T>
inline string CNodeCtl<T>::Add_Node(const T &msg){
	/* get md5 */
	string new_key = Md5().get_Md5(msg);
	/* add node */
	_Node_map[new_key] = CNode<T>(new_key, msg);
	return new_key;
}

template<typename T>
inline string CNodeCtl<T>::Add_Vir_Node(string key, const int N){
	if (N < 0) 
		throw string("N < 0\n");
	/* use HOOK func make msg */
	const T& new_msg = CNodeCtl<T>::MakeMsg(_Node_map[key].Get_real_msg(), N);
	string new_key = Md5().get_Md5(new_msg);
	/* vector push vir node */
	_Node_map[new_key]._Vir_Node.push_back(new_key);

	return new_key;
}

template<typename T>
inline vector<string> CNodeCtl<T>::Del_Node(const T &msg){
	string key = Md5().get_Md5(msg);
	/* find key */
	typename _Map_Type::iterator map_it = _Node_map.find(key);
	/* exception handle */
	
    if (map_it == _Node_map.end()) 
		throw string("not find msg\n");
	
    /* get vir vector */
    _Vec_Type vec = map_it->second._Vir_Node;
	/* push key */
	vec.push_back(key);
	_Node_map.erase(map_it);
	return vec;
}
template<typename T>
inline T CNodeCtl<T>::Get_Msg(const string &key){
	return _Node_map[key].Get_real_msg();
}
/********************************************/

//CHash code 
/********************************************/
template<typename T>
CHash<T>::CHash(int n, const vector<T> &vec)
:_Init_count(n)
{
	if (n != vec.size()) 
		throw string("init_numb error\n");
	string key;
	string new_key;
	int vir_num = 3;//1 real Node have 3 vir Node
	/*const vec can not use ordinary iterator*/
	typename vector<T>::const_iterator vec_it = vec.begin();

	for (; vec_it != vec.end(); ++vec_it){
		/* add real node*/
		key = _One_Node.Add_Node(*vec_it);
		/* add real key */
		_Ip_map[key] = key;
		/* looping add vir node */
		for (int i = 0; i < vir_num; ++i){
			new_key = _One_Node.Add_Vir_Node(key, i);
			_Ip_map[new_key] = key;
		}
	}
}

template<typename T>
inline void CHash<T>::Add_ser(const T &msg){
	string key = _One_Node.Add_Node(msg);
	/* add Ip */
	_Ip_map[key] = key;
	string new_key;
	/* looping push 3 vir node */
	for (int i = 0; i < 3; ++i){
		new_key = _One_Node.Add_Vir_Node(key, i);
		_Ip_map[new_key] = key;
	}
}

template<typename T>
inline void CHash<T>::Del_ser(const T &msg){
	/* get vir vector */
	vector<string> vir_Node(_One_Node.Del_Node(msg));

	for (unsigned int i = 0; i < vir_Node.size(); ++i){
		_Ip_map.erase(_Ip_map.find(vir_Node[i]));
	}
}

template<typename T>
inline T CHash<T>::Find_ser(const T &msg){
	string key = Md5().get_Md5(msg);
	/* use cosnt_iterator , can not modify */
	_Map_Type::const_iterator map_it = _Ip_map.begin();
	/* find a node */
	for (; map_it != _Ip_map.end(); ++map_it){
		if (map_it->first > key) break;
	}
	/* have noly a node */
	if (map_it == _Ip_map.end()) map_it = _Ip_map.begin();
	return _One_Node.Get_Msg(map_it->second);
}

template<typename T>
inline void CHash<T>::Show_ser(){
	cout << "Node : " << endl;
	CHash<T>::_Map_Type::const_iterator map_it = _Ip_map.begin();

	for (; map_it != _Ip_map.end(); ++map_it){
		cout << map_it->first << "--->" << _One_Node.Get_Msg(map_it->second) << endl;
	}
}
/********************************************/
