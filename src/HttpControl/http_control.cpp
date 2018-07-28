/*************************************************************************
> File Name: http_control.cpp
> Author:Darkfaker
> Mail:Darkfaker1024@163.com
> Created Time: Fri 27 Jul 2018 07:27:27 PM CST
************************************************************************/

#include <iostream>
#include "http_control.h"

using namespace std;

string Control_Http::get_Line(const string &data){
	size_t index = data.find("\r\n");
	if (index == data.npos) 
		throw "http format error\n";
	return string(data.substr(0, index));
}

string Control_Http::get_Head(const string &data){
	size_t index = data.find("\r\n");
	if (index == data.npos) 
		throw "http format error\n";
	return string(data.substr(index));
}

void Control_Http::make_Respons(){
	/*get line*/
	if (_flag == needLine){
		size_t index = _data_buff.find(" ");
		if (index == _data_buff.npos)
			throw "http format error\n";
		((_ip_Item_buf->http_line)->request_method).reserve(16);
		((_ip_Item_buf->http_line)->request_method) 
			= _data_buff.substr(0, index);
		index = _data_buff.find(" ", index);
		if (index == _data_buff.npos) 
			throw "http format error\n";
		size_t index_end = _data_buff.length() - 1;
		if (index_end == _data_buff.npos)
			throw "http format error\n";
		((_ip_Item_buf->http_line)->protocol_version).reserve(16);
		((_ip_Item_buf->http_line)->protocol_version) 
			= _data_buff.substr(index + 1, index_end - index);
	}
	/*get head*/
	else if (_flag == needHead){
		size_t index = _data_buff.find(" ");
		if (index == _data_buff.npos)
			throw "http format error\n";
		((_ip_Item_buf->http_head->http_line_info).request_method).reserve(16);
		((_ip_Item_buf->http_line)->request_method)
			= _data_buff.substr(0, index);
		index = _data_buff.find(" ", index);
		if (index == _data_buff.npos)
			throw "http format error\n";
		size_t index_end = _data_buff.find("\r\n", index);
		if (index_end == _data_buff.npos)
			throw "http format error\n";
		((_ip_Item_buf->http_head->http_line_info).protocol_version).reserve(16);
		((_ip_Item_buf->http_line)->protocol_version)
			= _data_buff.substr(index + 1, index_end - index);

		index = _data_buff.find("\n", index_end) + 1;
		index_end = _data_buff.find(" ", index);
		if (index == _data_buff.npos || index_end == _data_buff.npos)
			throw "http format error\n";
		(_ip_Item_buf->http_head)->accept_langeuage.reserve(16);
		(_ip_Item_buf->http_head)->accept_langeuage 
			= _data_buff.substr(index, index_end - index);
		index = index_end;
		index_end = _data_buff.find("\r\n", index_end);
		if (index_end == _data_buff.npos)
			throw "http format error\n";
		(_ip_Item_buf->http_head)->connection_type.reserve(16);
		(_ip_Item_buf->http_head)->connection_type
			= _data_buff.substr(index + 1, index_end - index);
	}
}

bool Control_Http::judge_file_exist(const string &file_name){
	fstream file(file_name, ios::in);
	if (file)
		return true;
	return false;
}

void Control_Http::show(){
	if (_flag == needLine){
		cout << _ip_Item_buf->http_line->request_method << endl;
		cout << _ip_Item_buf->http_line->protocol_version << endl;
	}
	else if (_flag == needHead){
		cout << "1: " << _ip_Item_buf->http_head->http_line_info.request_method << endl;
		cout << "2: " << _ip_Item_buf->http_head->http_line_info.protocol_version << endl;
		cout << "3: " << _ip_Item_buf->http_head->connection_type << endl;
		cout << "4: " << _ip_Item_buf->http_head->accept_langeuage << endl;
	}
}

UN_LONG Control_Http::getSize(int s){
	return (sizeof(ipItem_head_t)+s) > MAX_FILE_SIZE ? (sizeof(ipItem_head_t)+s) : MAX_FILE_SIZE;
}

