#ifndef _NGINX_H
#define _NGINX_H

#include "NginxPoolTypeInfo.h"

/*class memory pool*/
class NgxMemPool{
public:
	typedef shared_ptr<vector<pool_count_t>> _Smart_Ptr_Type;
	typedef weak_ptr<vector<pool_count_t>> _Weak_Ptr_Type;
private:
	my_ngx_pool_t *_pool;
	_Smart_Ptr_Type _pool_Number_Ptr;
	_Weak_Ptr_Type _pool_Number_Weak_Ptr;
public:
	NgxMemPool(size_t size){
		cout << "NgxMemPool(size_t size)" << endl;
		_pool = my_ngx_create_pool(size);
	}

	~NgxMemPool(){
		cout << "~NgxMemPool()" << endl;
		my_ngx_destroy_pool();
	}
	/*reset memory pool*/
	void my_ngx_reset_pool();
	/*align pool*/
	void *my_ngx_palloc(size_t size);
	/*not align*/
	void *my_ngx_pnalloc(size_t size);
	my_ngx_pool_cleanup_t *my_ngx_pool_cleanup_add(size_t size);
private:
	/*create mempry pool*/
	my_ngx_pool_t *my_ngx_create_pool(size_t size);
	/*alloc memory*/
	void *my_ngx_alloc(size_t size);
	void *my_ngx_calloc(size_t size);
	/*alloc small memory*/
	void *my_ngx_palloc_small(size_t size, bool align);
	/*alloc new block*/
	void *my_ngx_palloc_block(size_t size);
	/*alloc large memory*/
	void *my_ngx_palloc_large(size_t size);
	/*free memory*/
	uint my_ngx_pfree(void *p);
	/*destroy pool*/
	void my_ngx_destroy_pool();
};


#endif