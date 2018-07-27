#pragma once
#include <iostream>
using namespace std;
//////���峣��
const int  ngx_pagesize = 4096;
const int NGX_ALIGN = sizeof(unsigned long);
/////////�Զ�����������
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;
///////�����ṹ��
typedef struct my_ngx_data_s my_ngx_data_t;

typedef struct my_ngx_pool_s my_ngx_pool_t;

typedef struct my_ngx_pool_large_s my_ngx_pool_large_t;

typedef struct my_ngx_pool_cleanup_s my_ngx_pool_cleanup_t;

typedef void(*my_ngx_pool_cleanup_pt)(void *data);
/////�����ֽڶ��뺯��
inline ulong my_ngx_align(uchar d, ulong a){return (((d)+(a - 1)) & ~(a - 1));}
inline uchar* my_ngx_align_ptr(uchar *p, ulong a){
	return ((uchar *)(((uint)(p)+((uint)a - 1)) & ~((uint)a - 1)));
}
//�ڴ����ݶ��ڴ�ṹ
typedef struct my_ngx_data_s{
	uchar *last;
	uchar *end;
	my_ngx_pool_t *next;
	uint failed;
}my_ngx_data_t;
//��ʼ�ڴ�ؽṹ
typedef struct my_ngx_pool_s{
	my_ngx_data_t data;
	size_t max;
	my_ngx_pool_t *current;
	my_ngx_pool_large_t *large;
	my_ngx_pool_cleanup_t *cleanup;
}my_ngx_pool_t;
//����ڴ�ṹ
typedef struct my_ngx_pool_large_s{
	my_ngx_pool_large_t *next;
	void *alloc;
}my_ngx_pool_large_t;
//�Զ�������ṹ
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
	void *my_ngx_alloc(size_t size);//������ʼ�������ڴ�

	void *my_ngx_calloc(size_t size);//��ʼ��Ϊ0�����ڴ�

	my_ngx_pool_t *my_ngx_create_pool(size_t size);//�����ڴ��

	void my_ngx_destroy_pool();//�����ڴ��

	void my_ngx_reset_pool();//�����ڴ��

	void *my_ngx_palloc(size_t size);//���ֽڶ��뷽ʽ���ڴ�ػ����ڴ�

	void *my_ngx_pnalloc(size_t size);//�Է��ֽڶ��뷽ʽ���ڴ�ػ����ڴ�

	void *my_ngx_palloc_small(size_t size, bool align);//�ڴ��С���ڴ滮��

	void *my_ngx_palloc_block(size_t size);//�ڴ��ȡִ�к���

	void *my_ngx_palloc_large(size_t size);//����ڴ��ȡ����

	uint my_ngx_pfree(void *p);//����ڴ��ͷź���

	my_ngx_pool_cleanup_t *my_ngx_pool_cleanup_add(size_t size);//�Զ�����������ʼ��

	void my_ngx_pool_run_cleanup_file(int fd);//�����ļ�����

	void my_ngx_pool_cleanup_file(void *data);

	void my_ngx_pool_delete_file(void *data);

private:
	my_ngx_pool_t *_pool;
};


