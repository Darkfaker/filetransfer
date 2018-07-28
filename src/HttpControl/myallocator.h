#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include "../NginxPool/Nginx.h"

/*Define the default memory management method*/
template<typename T, typename Ngx = NgxMemPool>
class myallocator{
public:
	typedef unsigned long long UN_LONG;
private:
	Ngx *_Ngx;
public:
	myallocator(UN_LONG size){
		_Ngx = new NgxMemPool(size);
	}
	/*delete*/
	~myallocator(){
		delete _Ngx;
		_Ngx.~NgxMemPool();
		_Ngx = NULL;
	}
	void construct(void *ptr, const T &val){
		new (ptr)T(val);
	}

	void construct(void *ptr){
		new (ptr)T();
	}
	void destroy(T *ptr){
		ptr->~T();
	}

	void* allocate(size_t size){
		return _Ngx->my_ngx_palloc(size);
	}
	/*reset memory pool*/
	void deallocate(void *ptr){
		_Ngx->my_ngx_reset_pool();
	}
};

#endif 
