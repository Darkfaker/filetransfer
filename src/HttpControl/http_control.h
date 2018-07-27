/*************************************************************************
	> File Name: http_control.cpp
	> Author: Darkfaker
	> Mail: Darkfaker1024@163.com
	> Created Time: Thu 26 Jul 2018 07:15:02 PM CST
 ************************************************************************/
#ifndef _HTTP_CONTROL_H
#define _HTTP_CONTROL_H

#include<iostream>
using namespace std;
typedef unsigned long long UN_LONG;
const UN_LONG MAX_FILE_SIZE = 1024 * 1024 * 4; 

typedef struct ipItem{
    string ip_port;
    string status_mask;
    string cli_name;
    string 

    UN_LONG file_pos;
    UN_LONG file_end;
    int file_fd;
}ipItem;

class Control_Http{
public:

    private:
    ipItem _ip_Item_buf;
    UN_LONG _max_file_size;
    string &_data_buff;
public:
    Control_Http(const ipItem &data)
        : _data_buff(data)
        , _max_file_size(MAX_FILE_SIZE)
    {}
    ~Control_Http();
    string get_Head();
    string get_Data();
    void make_Respons();
    bool judge_file_exist();
    void show();
};

#endif




