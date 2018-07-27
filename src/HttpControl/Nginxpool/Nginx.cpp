#pragma once
#include "Nginx.h"
#include <iostream>
using namespace std;

void *NgxMemPool::my_ngx_alloc(size_t size){
	void *ptr;
	//封装malloc
	ptr = malloc(size);
	if (ptr == NULL)
		perror("malloc error!\n");
	return ptr;
}

void *NgxMemPool::my_ngx_calloc(size_t size){
	void *ptr;
	//调用my_ngx_alloc方法
	ptr = my_ngx_alloc(size);
	if (NULL == ptr)
		//再将内存清0
		memset(ptr, 0, size);
	return ptr;
}

my_ngx_pool_t *NgxMemPool::my_ngx_create_pool(size_t size){
	my_ngx_pool_t *pool;
	//相当于分配一块内存调用了 ngx_alloc(size, log)
	pool = static_cast<my_ngx_pool_t*>(my_ngx_alloc(size));
	if (NULL == pool)
		perror("ngx create pool error!\n");
	//last指向结构体末尾
	pool->data.last = (uchar*)pool + sizeof(my_ngx_pool_t);
	//指向整个内存块末尾
	pool->data.end = (uchar*)pool + size;
	//下一个ngx_pool_t 内存池地址  
	pool->data.next = NULL;
	//首次分配置0
	pool->data.failed = 0;
	size = size - sizeof(my_ngx_pool_t);
	pool->max = (size < ngx_pagesize) ? size : ngx_pagesize;
	pool->current = pool;
	pool->large = NULL;
	pool->cleanup = NULL;
	return pool;
}
//销毁内存池
void NgxMemPool::my_ngx_destroy_pool(){
	my_ngx_pool_t *ptmp;
	my_ngx_pool_t *ntmp;
	my_ngx_pool_large_t *latmp;
	my_ngx_pool_cleanup_t *ctmp;
	//首先清理pool->cleanup链表 
	ctmp = _pool->cleanup;
	while (ctmp){
		if (!ctmp->hander)//handler 为一个清理的回调函数 
			ctmp->hander(ctmp->data);
		ctmp = ctmp->next;
	}
	//清理pool->large链表(pool->large为单独的大数据内存块) 
	latmp = _pool->large;
	while (latmp){
		if (latmp->alloc)
			free(latmp->alloc);
		latmp = latmp->next;
	}
	// 对内存池的data数据区域进行释放
	for (ptmp = _pool, ntmp = _pool->data.next;; ptmp = ntmp, ntmp = ntmp->data.next){
		free(ptmp);
		if (NULL == ntmp)
			break;
	}
}
//重设内存池
void NgxMemPool::my_ngx_reset_pool(){
	my_ngx_pool_t  *ptmp;
	my_ngx_pool_large_t  *latmp;
	//清理pool->large链表（pool->large为单独的大数据内存块
	for (latmp = _pool->large; latmp; latmp = latmp->next){
		if (latmp->alloc)
			free(latmp->alloc);
	}
	// 循环重新设置内存池data区域的 p->d.last；data区域数据并不擦除
	for (ptmp = _pool; ptmp; ptmp = ptmp->data.next){
		ptmp->data.last = (uchar *)ptmp + sizeof(my_ngx_pool_t);
		ptmp->data.failed = 0;
	}

	_pool->current = _pool;
	_pool->large = NULL;
}
// 内存池分配一块内存，返回void类型指?
void *NgxMemPool::my_ngx_palloc(size_t size){
	// 判断每次分配的内存大小，如果超出pool->max的限制，需要调用大块内存分配
	if (size <= _pool->max)
		//传入true 表示需要考虑内存对齐
		return my_ngx_palloc_small(size, true);
	return my_ngx_palloc_large(size);
}

void *NgxMemPool::my_ngx_pnalloc(size_t size){
	if (size <= _pool->max)
		//传入false 表示需要考虑内存对齐
		return my_ngx_palloc_small(size, false);
	return my_ngx_palloc_large(size);
}

void *NgxMemPool::my_ngx_palloc_small(size_t size, bool align){
	uchar *ptmp;
	my_ngx_pool_t *ptr;

	ptr = _pool->current;
	do{
		ptmp = ptr->data.last;
		if (align)//通过传入的align 判断是否考虑内存对齐
			ptmp = my_ngx_align_ptr(ptmp, NGX_ALIGN);
		if ((size_t)((ptr->data.end) - ptmp) >= size){
			ptr->data.last = ptmp + size;
			return ptmp;
		}
		ptr = ptr->data.next;
	} while (ptr);
	//如果没有缓存池空间没有可以容纳大小为size的内存块，则需要重新申请一个缓存池pool节点
	return my_ngx_palloc_block(size);
}

void *NgxMemPool::my_ngx_palloc_block(size_t size){
	uchar *ptmp;
	size_t sizetmp;
	my_ngx_pool_t *ptr;
	my_ngx_pool_t *newptr;
	// 申请新的块        
	sizetmp = (size_t)(_pool->data.end - (uchar*)_pool);
	ptmp = (uchar*)my_ngx_alloc(size);
	if (NULL == ptmp)
		return NULL;

	newptr = (my_ngx_pool_t*)ptmp;
	newptr->data.end = ptmp + sizetmp;
	newptr->data.next = NULL;
	newptr->data.failed = 0;
	//分配size大小的内存块，返回m指针地址
	ptmp += sizeof(my_ngx_data_t);
	ptmp = my_ngx_align_ptr(ptmp, NGX_ALIGN);
	newptr->data.last = ptmp + size;

	for (ptr = _pool->current; ptr->data.next; ptr = ptr->data.next){
		if (ptr->data.failed++ > 4)//大于4次就查找下一个
			_pool->current = ptr->data.next;
	}
	ptr->data.next = newptr;

	return ptmp;
}
// 当分配的内存块大小超出pool->max限制的时候,需要分配在pool->large上 
void *NgxMemPool::my_ngx_palloc_large(size_t size){
	void *ptr;
	uint numb;
	my_ngx_pool_large_t *large;
	//申请一块独立的内存块
	ptr = my_ngx_alloc(size);
	if (NULL == ptr)
		return NULL;
	//pool->large链表上查询是否有NULL的，只在链表上往下查询3次，主要判断大数据块是否有被释放的，如果没有则只能跳出
	numb = 0;
	for (large = _pool->large; large; large = large->next){
		//寻找NULL
		if (NULL == large->alloc){
			large->alloc = ptr;
			return ptr;
		}
		//为了提高效率，如果3次还没找到空闲的large块，则创建一个
		if (numb++ > 3)//大于三次没找到就创建一个large
			break;
	}
	//分配一个ngx_pool_large_t 数据结构
	large = (my_ngx_pool_large_t*)my_ngx_palloc_small(sizeof(my_ngx_pool_large_t), true);
	if (NULL == large){
		free(ptr);//分配失败就先释放alloc开辟的空间
		return NULL;
	}

	large->alloc = ptr;
	large->next = _pool->large;
	_pool->large = large;
	return ptr;
}

uint NgxMemPool::my_ngx_pfree(void *p){
	//释放大块内存
	//在pool->large链上循环搜索，并且只释放内容区域，不释放ngx_pool_large_t数据结?
	my_ngx_pool_large_t *large;
	for (large = _pool->large; large; large = large->next){
		if (p == large->alloc){
			free(large->alloc);
			large->alloc = NULL;
			return 0;
		}
	}
	return -1;
}

my_ngx_pool_cleanup_t *NgxMemPool::my_ngx_pool_cleanup_add(size_t size){
	my_ngx_pool_cleanup_t *cptr;
	//从创建cleanup结构
	cptr = (my_ngx_pool_cleanup_t*)my_ngx_palloc(sizeof(my_ngx_pool_cleanup_t));
	if (NULL == cptr)
		return NULL;
	//如果size !=0 从pool->d 或pool->large分配一个内存块
	if (size){
		//分配一个size大小的内存
		cptr->data = my_ngx_palloc(size);
		if (NULL == cptr->data)
			return NULL;
	}
	else
		cptr->data = NULL;
	cptr->hander = NULL;
	cptr->next = _pool->cleanup;
	_pool->cleanup = cptr;

	return cptr;
}
