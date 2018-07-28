#include "Nginx.h"
#include "http_control.h"
#include "myallocator.h"
#include <STL>
#include <iostream>
using namespace std;

int main(){
	try{
		Control_Http http(string("HTTP/ 200 OK\r\ntype zh-cn\r\n"), 0);
		http.show();
	}
	catch (const char* str){
		cout << str << endl;
	};
	

	system("pause");

	return 0;
}