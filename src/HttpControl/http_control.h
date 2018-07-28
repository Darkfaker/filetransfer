/*************************************************************************
> File Name: http_control.cpp
> Author: Darkfaker
> Mail: Darkfaker1024@163.com
> Created Time: Thu 26 Jul 2018 07:15:02 PM CST
************************************************************************/
#ifndef _HTTP_CONTROL_H
#define _HTTP_CONTROL_H
#include <iostream>
#include "../NginxPool/Nginx.h"
#include "head.h"
#include "myallocator.h"
using namespace std;
typedef unsigned long long UN_LONG;
UN_LONG const MAX_FILE_SIZE = 1024 * 1024 * 4;
/*struct HTTP line*/
typedef struct ipItem_line_s{
	uchar *line_pos;/*the pos of line in pool*/
	uchar *line_end;/*the end of line in pool*/
	string request_method;/*the request method of cli*/
	string status_mask;/*the status mask of cli*/
	string protocol_version;/*the protocal version of cli(ipv4/ipv6)*/
}ipItem_line_s;
typedef ipItem_line_s ipItem_line_t;
/*struct file*/
typedef struct file_info_s{
	int file_fd;/*file fd*/
	UN_LONG file_size;/*file size*/
	UN_LONG file_pos;/*the pos of file in Disk*/
	UN_LONG file_end;/*the end of file in Disk*/
}file_info_s;
typedef file_info_s file_info_t;
/*struct HTTP head*/
typedef struct ipItem_head_s{
	ipItem_line_s http_line_info;/*struct HTTP line*/
	string accept_langeuage;/*the langeuage of cli use*/
	string connection_type;/*the connextion type of cli*/
	file_info_t file_information;/*struct file*/
}ipItem_head_s;
typedef ipItem_head_s ipItem_head_t;
/*struct ip Item*/
typedef union ipItem_s{
	/*request line*/
	ipItem_line_t *http_line;
	/*request head*/
	ipItem_head_t *http_head;
}ipItem_s;
typedef ipItem_s ipItem_t;

/*class HTTP Control*/
class Control_Http{
public:
#define needLine true
#define needHead false
	typedef myallocator<ipItem_t> _Pool_Type;
private:
	ipItem_t *_ip_Item_buf;
	UN_LONG _max_file_size;
	string _data_buff;/*tmp buf*/
	_Pool_Type *pool;/*memory pool*/
	bool _flag;/*line or head*/
public:
	Control_Http(const string &data, bool f)
		: _max_file_size(MAX_FILE_SIZE)
		, _flag(f)
	{
		/*create memory pool*/
		pool = new _Pool_Type(getSize(data.length()));
		/*allocate memory*/
		ipItem_t *ptr = (ipItem_t*)pool->allocate(sizeof(ipItem_t) + 128);
		if (NULL == ptr)
			throw "memory is empty\n";
		/*create tmp buf*/
		_ip_Item_buf = new ipItem_t();
		/*choose line or head*/
		if (_flag == needLine){
			_ip_Item_buf->http_line = new ipItem_line_t();
			_data_buff = get_Line(data);
		}
		else if (_flag == needHead){
			_ip_Item_buf->http_head = new ipItem_head_t();
			_data_buff = get_Line(data);
			_data_buff += get_Head(data);
		}
		/*make respons*/
		make_Respons();
		/*construct ip Item*/
		pool->construct(ptr, *_ip_Item_buf);
		/*clear buff*/
		_data_buff.clear();
		/*delete tmp buff*/
		delete _ip_Item_buf;
		/*use the memory pool ip Item*/
		_ip_Item_buf = ptr;
	}
	~Control_Http(){
		pool->destroy(_ip_Item_buf);
		pool->deallocate(_ip_Item_buf);
	}
	/*get HTTP line*/
	string get_Line(const string &data);
	/*get HTTP head*/
	string get_Head(const string &data);
	/*make respons*/
	void make_Respons();
	/*juege the file is exist*/
	bool judge_file_exist(const string &file_name);
	void show();
	/*sum size*/
	UN_LONG getSize(int s);
};

#endif




