#pragma once
#include "Nginx.h"
#include <iostream>
using namespace std;

void *NgxMemPool::my_ngx_alloc(size_t size){
	void *ptr;
	//��װmalloc
	ptr = malloc(size);
	if (ptr == NULL)
		perror("malloc error!\n");
	return ptr;
}

void *NgxMemPool::my_ngx_calloc(size_t size){
	void *ptr;
	//����my_ngx_alloc����
	ptr = my_ngx_alloc(size);
	if (NULL == ptr)
		//�ٽ��ڴ���0
		memset(ptr, 0, size);
	return ptr;
}

my_ngx_pool_t *NgxMemPool::my_ngx_create_pool(size_t size){
	my_ngx_pool_t *pool;
	//�൱�ڷ���һ���ڴ������ ngx_alloc(size, log)
	pool = static_cast<my_ngx_pool_t*>(my_ngx_alloc(size));
	if (NULL == pool)
		perror("ngx create pool error!\n");
	//lastָ��ṹ��ĩβ
	pool->data.last = (uchar*)pool + sizeof(my_ngx_pool_t);
	//ָ�������ڴ��ĩβ
	pool->data.end = (uchar*)pool + size;
	//��һ��ngx_pool_t �ڴ�ص�ַ  
	pool->data.next = NULL;
	//�״η�����0
	pool->data.failed = 0;
	size = size - sizeof(my_ngx_pool_t);
	pool->max = (size < ngx_pagesize) ? size : ngx_pagesize;
	pool->current = pool;
	pool->large = NULL;
	pool->cleanup = NULL;
	return pool;
}
//�����ڴ��
void NgxMemPool::my_ngx_destroy_pool(){
	my_ngx_pool_t *ptmp;
	my_ngx_pool_t *ntmp;
	my_ngx_pool_large_t *latmp;
	my_ngx_pool_cleanup_t *ctmp;
	//��������pool->cleanup���� 
	ctmp = _pool->cleanup;
	while (ctmp){
		if (!ctmp->hander)//handler Ϊһ������Ļص����� 
			ctmp->hander(ctmp->data);
		ctmp = ctmp->next;
	}
	//����pool->large����(pool->largeΪ�����Ĵ������ڴ��) 
	latmp = _pool->large;
	while (latmp){
		if (latmp->alloc)
			free(latmp->alloc);
		latmp = latmp->next;
	}
	// ���ڴ�ص�data������������ͷ�
	for (ptmp = _pool, ntmp = _pool->data.next;; ptmp = ntmp, ntmp = ntmp->data.next){
		free(ptmp);
		if (NULL == ntmp)
			break;
	}
}
//�����ڴ��
void NgxMemPool::my_ngx_reset_pool(){
	my_ngx_pool_t  *ptmp;
	my_ngx_pool_large_t  *latmp;
	//����pool->large����pool->largeΪ�����Ĵ������ڴ��
	for (latmp = _pool->large; latmp; latmp = latmp->next){
		if (latmp->alloc)
			free(latmp->alloc);
	}
	// ѭ�����������ڴ��data����� p->d.last��data�������ݲ�������
	for (ptmp = _pool; ptmp; ptmp = ptmp->data.next){
		ptmp->data.last = (uchar *)ptmp + sizeof(my_ngx_pool_t);
		ptmp->data.failed = 0;
	}

	_pool->current = _pool;
	_pool->large = NULL;
}
// �ڴ�ط���һ���ڴ棬����void����ָ?
void *NgxMemPool::my_ngx_palloc(size_t size){
	// �ж�ÿ�η�����ڴ��С���������pool->max�����ƣ���Ҫ���ô���ڴ����
	if (size <= _pool->max)
		//����true ��ʾ��Ҫ�����ڴ����
		return my_ngx_palloc_small(size, true);
	return my_ngx_palloc_large(size);
}

void *NgxMemPool::my_ngx_pnalloc(size_t size){
	if (size <= _pool->max)
		//����false ��ʾ��Ҫ�����ڴ����
		return my_ngx_palloc_small(size, false);
	return my_ngx_palloc_large(size);
}

void *NgxMemPool::my_ngx_palloc_small(size_t size, bool align){
	uchar *ptmp;
	my_ngx_pool_t *ptr;

	ptr = _pool->current;
	do{
		ptmp = ptr->data.last;
		if (align)//ͨ�������align �ж��Ƿ����ڴ����
			ptmp = my_ngx_align_ptr(ptmp, NGX_ALIGN);
		if ((size_t)((ptr->data.end) - ptmp) >= size){
			ptr->data.last = ptmp + size;
			return ptmp;
		}
		ptr = ptr->data.next;
	} while (ptr);
	//���û�л���ؿռ�û�п������ɴ�СΪsize���ڴ�飬����Ҫ��������һ�������pool�ڵ�
	return my_ngx_palloc_block(size);
}

void *NgxMemPool::my_ngx_palloc_block(size_t size){
	uchar *ptmp;
	size_t sizetmp;
	my_ngx_pool_t *ptr;
	my_ngx_pool_t *newptr;
	// �����µĿ�        
	sizetmp = (size_t)(_pool->data.end - (uchar*)_pool);
	ptmp = (uchar*)my_ngx_alloc(size);
	if (NULL == ptmp)
		return NULL;

	newptr = (my_ngx_pool_t*)ptmp;
	newptr->data.end = ptmp + sizetmp;
	newptr->data.next = NULL;
	newptr->data.failed = 0;
	//����size��С���ڴ�飬����mָ���ַ
	ptmp += sizeof(my_ngx_data_t);
	ptmp = my_ngx_align_ptr(ptmp, NGX_ALIGN);
	newptr->data.last = ptmp + size;

	for (ptr = _pool->current; ptr->data.next; ptr = ptr->data.next){
		if (ptr->data.failed++ > 4)//����4�ξͲ�����һ��
			_pool->current = ptr->data.next;
	}
	ptr->data.next = newptr;

	return ptmp;
}
// ��������ڴ���С����pool->max���Ƶ�ʱ��,��Ҫ������pool->large�� 
void *NgxMemPool::my_ngx_palloc_large(size_t size){
	void *ptr;
	uint numb;
	my_ngx_pool_large_t *large;
	//����һ��������ڴ��
	ptr = my_ngx_alloc(size);
	if (NULL == ptr)
		return NULL;
	//pool->large�����ϲ�ѯ�Ƿ���NULL�ģ�ֻ�����������²�ѯ3�Σ���Ҫ�жϴ����ݿ��Ƿ��б��ͷŵģ����û����ֻ������
	numb = 0;
	for (large = _pool->large; large; large = large->next){
		//Ѱ��NULL
		if (NULL == large->alloc){
			large->alloc = ptr;
			return ptr;
		}
		//Ϊ�����Ч�ʣ����3�λ�û�ҵ����е�large�飬�򴴽�һ��
		if (numb++ > 3)//��������û�ҵ��ʹ���һ��large
			break;
	}
	//����һ��ngx_pool_large_t ���ݽṹ
	large = (my_ngx_pool_large_t*)my_ngx_palloc_small(sizeof(my_ngx_pool_large_t), true);
	if (NULL == large){
		free(ptr);//����ʧ�ܾ����ͷ�alloc���ٵĿռ�
		return NULL;
	}

	large->alloc = ptr;
	large->next = _pool->large;
	_pool->large = large;
	return ptr;
}

uint NgxMemPool::my_ngx_pfree(void *p){
	//�ͷŴ���ڴ�
	//��pool->large����ѭ������������ֻ�ͷ��������򣬲��ͷ�ngx_pool_large_t���ݽ�?
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
	//�Ӵ���cleanup�ṹ
	cptr = (my_ngx_pool_cleanup_t*)my_ngx_palloc(sizeof(my_ngx_pool_cleanup_t));
	if (NULL == cptr)
		return NULL;
	//���size !=0 ��pool->d ��pool->large����һ���ڴ��
	if (size){
		//����һ��size��С���ڴ�
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
