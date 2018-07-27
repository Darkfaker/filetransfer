#pragma once
#include <iostream>
using namespace std;
//////定义常量
const int  ngx_pagesize = 4096;
const int NGX_ALIGN = sizeof(unsigned long);
/////////自定义数据类型
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;
///////声明结构体
typedef struct my_ngx_data_s my_ngx_data_t;

typedef struct my_ngx_pool_s my_ngx_pool_t;

typedef struct my_ngx_pool_large_s my_ngx_pool_large_t;

typedef struct my_ngx_pool_cleanup_s my_ngx_pool_cleanup_t;

typedef void(*my_ngx_pool_cleanup_pt)(void *data);
/////定义字节对齐函数
inline ulong my_ngx_align(uchar d, ulong a){return (((d)+(a - 1)) & ~(a - 1));}
inline uchar* my_ngx_align_ptr(uchar *p, ulong a){
	return ((uchar *)(((uint)(p)+((uint)a - 1)) & ~((uint)a - 1)));
}
//内存数据段内存结构
typedef struct my_ngx_data_s{
	uchar *last;
	uchar *end;
	my_ngx_pool_t *next;
	uint failed;
}my_ngx_data_t;
//初始内存池结构
typedef struct my_ngx_pool_s{
	my_ngx_data_t data;
	size_t max;
	my_ngx_pool_t *current;
	my_ngx_pool_large_t *large;
	my_ngx_pool_cleanup_t *cleanup;
}my_ngx_pool_t;
//大块内存结构
typedef struct my_ngx_pool_large_s{
	my_ngx_pool_large_t *next;
	void *alloc;
}my_ngx_pool_large_t;
//自定义清理结构
typedef struct my_ngx_pool_cleanup_s{
	my_ngx_pool_cleanup_pt hander;
	void *data;
	my_ngx_pool_cleanup_t *next;
}my_ngx_pool_cleanup_t;


class NgxMemPool{
public:
	NgxMemPool(size_t size){
		cout << "NgxMemPool(size_t size)" << endl;
		_pool = my_ngx_create_pool(size);
	}
	~NgxMemPool(){
		cout << "~NgxMemPool()" << endl;
		my_ngx_destroy_pool();
	}
	void *my_ngx_alloc(size_t size);//不出初始化分配内存

	void *my_ngx_calloc(size_t size);//初始化为0分配内存

	my_ngx_pool_t *my_ngx_create_pool(size_t size);//创建内存池

	void my_ngx_destroy_pool();//销毁内存池

	void my_ngx_reset_pool();//重置内存池

	void *my_ngx_palloc(size_t size);//以字节对齐方式从内存池划分内存

	void *my_ngx_pnalloc(size_t size);//以非字节对齐方式从内存池划分内存

	void *my_ngx_palloc_small(size_t size, bool align);//内存池小块内存划分

	void *my_ngx_palloc_block(size_t size);//内存获取执行函数

	void *my_ngx_palloc_large(size_t size);//大块内存获取函数

	uint my_ngx_pfree(void *p);//大块内存释放函数

	my_ngx_pool_cleanup_t *my_ngx_pool_cleanup_add(size_t size);//自定义清理函数初始化

	void my_ngx_pool_run_cleanup_file(int fd);//清理文件类型

	void my_ngx_pool_cleanup_file(void *data);

	void my_ngx_pool_delete_file(void *data);

private:
	my_ngx_pool_t *_pool;
};


