#ifndef _NGINXPOOLTYPEINFO_H
#define _NGINXPOOLTYPEINFO_H
#include <iostream>
#include <vector>
#include <memory>

using namespace std;
const int  ngx_pagesize = 4096;
const int NGX_ALIGN = sizeof(unsigned long);
/*my data type*/
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;

/*typedef struct*/
typedef struct my_ngx_data_s my_ngx_data_t;
typedef struct my_ngx_pool_s my_ngx_pool_t;
typedef struct pool_count_s pool_count_t;
typedef struct my_ngx_pool_large_s my_ngx_pool_large_t;
typedef struct my_ngx_pool_cleanup_s my_ngx_pool_cleanup_t;
typedef void(*my_ngx_pool_cleanup_pt)(void *data);

/*align*/
inline ulong my_ngx_align(uchar d, ulong a){ 
	return (((d)+(a - 1)) & ~(a - 1)); 
}
inline uchar* my_ngx_align_ptr(uchar *p, ulong a){
	return ((uchar *)(((uint)(p)+((uint)a - 1)) & ~((uint)a - 1)));
}

/*the number of pool*/
typedef struct pool_count_s{
	uint number;
	shared_ptr<my_ngx_pool_t> poolPtr;
	weak_ptr<my_ngx_pool_t> weak_poolPtr;
}pool_count_s;

/*struct data*/
typedef struct my_ngx_data_s{
	uchar *last;
	uchar *end;
	my_ngx_pool_t *next;
	uint failed;
}my_ngx_data_t;

/*struct pool*/
typedef struct my_ngx_pool_s{
	my_ngx_data_t data;
	size_t max;
	my_ngx_pool_t *current;
	my_ngx_pool_large_t *large;
	my_ngx_pool_cleanup_t *cleanup;
	uint number;
}my_ngx_pool_t;

/*struct large*/
typedef struct my_ngx_pool_large_s{
	my_ngx_pool_large_t *next;
	void *alloc;
}my_ngx_pool_large_t;

/*struct cleanup*/
typedef struct my_ngx_pool_cleanup_s{
	my_ngx_pool_cleanup_pt hander;
	void *data;
	my_ngx_pool_cleanup_t *next;
}my_ngx_pool_cleanup_t;


#endif