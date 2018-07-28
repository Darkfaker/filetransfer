#pragma once
#include "Nginx.h"
#include <iostream>
using namespace std;

void *NgxMemPool::my_ngx_alloc(size_t size){
	void *ptr;
	ptr = malloc(size);
	if (ptr == NULL)
		perror("malloc error!\n");
	return ptr;
}

void *NgxMemPool::my_ngx_calloc(size_t size){
	void *ptr;
	ptr = my_ngx_alloc(size);
	if (NULL == ptr)
		memset(ptr, 0, size);
	return ptr;
}

my_ngx_pool_t *NgxMemPool::my_ngx_create_pool(size_t size){
	my_ngx_pool_t *pool;
	pool = static_cast<my_ngx_pool_t*>(my_ngx_alloc(size));
	if (NULL == pool)
		perror("ngx create pool error!\n");

	pool->data.last = (uchar*)pool + sizeof(my_ngx_pool_t);
	pool->data.end = (uchar*)pool + size; 
	pool->data.next = NULL;
	pool->data.failed = 0;
	size = size - sizeof(my_ngx_pool_t);

	pool->max = (size < ngx_pagesize) ? size : ngx_pagesize;
	pool->current = pool;
	pool->large = NULL;
	pool->cleanup = NULL;
	pool->number = 0;
	return pool;
}

void NgxMemPool::my_ngx_destroy_pool(){
	my_ngx_pool_t *ptmp;
	my_ngx_pool_t *ntmp;
	my_ngx_pool_large_t *latmp;
	my_ngx_pool_cleanup_t *ctmp;
	ctmp = _pool->cleanup;

	while (ctmp){
		if (!ctmp->hander) 
			ctmp->hander(ctmp->data);
		ctmp = ctmp->next;
	}

	latmp = _pool->large;
	while (latmp){
		if (latmp->alloc)
			free(latmp->alloc);
		latmp = latmp->next;
	}

	for (ptmp = _pool, ntmp = _pool->data.next;; ptmp = ntmp, ntmp = ntmp->data.next){
		free(ptmp);
		if (NULL == ntmp)
			break;
	}
}

void NgxMemPool::my_ngx_reset_pool(){
	my_ngx_pool_t  *ptmp;
	my_ngx_pool_large_t  *latmp;

	for (latmp = _pool->large; latmp; latmp = latmp->next){
		if (latmp->alloc)
			free(latmp->alloc);
	}
	////////////////////////Ã÷ÌìÐÞ¸Ä
	
	for (ptmp = _pool; ptmp; ptmp = ptmp->data.next){
		ptmp->data.last = (uchar *)ptmp + sizeof(my_ngx_pool_t);
		ptmp->data.failed = 0;
	}

	_pool->current = _pool;
	_pool->large = NULL;
}

void *NgxMemPool::my_ngx_palloc(size_t size){

	if (size <= _pool->max)
		return my_ngx_palloc_small(size, true);
	return my_ngx_palloc_large(size);
}

void *NgxMemPool::my_ngx_pnalloc(size_t size){
	if (size <= _pool->max)
		return my_ngx_palloc_small(size, false);
	return my_ngx_palloc_large(size);
}

void *NgxMemPool::my_ngx_palloc_small(size_t size, bool align){
	uchar *ptmp;
	my_ngx_pool_t *ptr;

	ptr = _pool->current;
	do{
		ptmp = ptr->data.last;
		if (align)
			ptmp = my_ngx_align_ptr(ptmp, NGX_ALIGN);
		if ((size_t)((ptr->data.end) - ptmp) >= size){
			ptr->data.last = ptmp + size;
			return ptmp;
		}
		ptr = ptr->data.next;
	} while (ptr);
	return my_ngx_palloc_block(size);
}

void *NgxMemPool::my_ngx_palloc_block(size_t size){
	uchar *ptmp;
	size_t sizetmp;
	my_ngx_pool_t *ptr;
	my_ngx_pool_t *newptr;

	sizetmp = (size_t)(_pool->data.end - (uchar*)_pool);
	ptmp = (uchar*)my_ngx_alloc(size);
	if (NULL == ptmp)
		return NULL;

	newptr = (my_ngx_pool_t*)ptmp;
	newptr->data.end = ptmp + sizetmp;
	newptr->data.next = NULL;
	newptr->data.failed = 0;

	ptmp += sizeof(my_ngx_data_t);
	ptmp = my_ngx_align_ptr(ptmp, NGX_ALIGN);
	newptr->data.last = ptmp + size;

	for (ptr = _pool->current; ptr->data.next; ptr = ptr->data.next){
		if (ptr->data.failed++ > 4)
			_pool->current = ptr->data.next;
	}
	ptr->data.next = newptr;

	return ptmp;
}
 
void *NgxMemPool::my_ngx_palloc_large(size_t size){
	void *ptr;
	uint numb;
	my_ngx_pool_large_t *large;

	ptr = my_ngx_alloc(size);
	if (NULL == ptr)
		return NULL;
	numb = 0;
	for (large = _pool->large; large; large = large->next){
		if (NULL == large->alloc){
			large->alloc = ptr;
			return ptr;
		}
		if (numb++ > 3)
			break;
	}

	large = (my_ngx_pool_large_t*)my_ngx_palloc_small(sizeof(my_ngx_pool_large_t), true);
	if (NULL == large){
		free(ptr);
		return NULL;
	}

	large->alloc = ptr;
	large->next = _pool->large;
	_pool->large = large;
	return ptr;
}

uint NgxMemPool::my_ngx_pfree(void *p){
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
	cptr = (my_ngx_pool_cleanup_t*)my_ngx_palloc(sizeof(my_ngx_pool_cleanup_t));
	if (NULL == cptr)
		return NULL;
	if (size){

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
