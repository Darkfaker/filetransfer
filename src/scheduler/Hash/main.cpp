//#include <STL>
#include "hash.h"
using namespace std;

string func(const string &str, int n){
	char buff[100] = "";
	sprintf(buff, "%s#%d", str.c_str(), n);
	return string(buff);
}

int main(){
	vector<string> vs;
	vs.push_back("192.168.1.34:8000");
	vs.push_back("192.168.1.35:8000");
	vs.push_back("192.168.1.35:8001");
	vs.push_back("192.168.1.36:8003");
	vs.push_back("192.168.1.36:8004");
	vs.push_back("127.0.0.1:8000");
	CNodeCtl<string>::Set_Fun(func);
	//CHash<string>* hash = CHash<string>::Get_ip_map(6, vs);
	CHash<string> hash(6, vs);
	hash.Show_ser();

	string str;
	cout << "Msg:";
	bool isruning = true;
	int count = 3;
	while (isruning){
		cin >> str;
		cout << "GetSer: " << hash.Find_ser(str) << endl;
		hash.Add_ser(str);
		if (!--count)
			isruning = false;
		if (count != 1)
			cout << "Msg: ";
		else {
			string s;
			cout << "del :";
			cin >> s;
			try{
				hash.Del_ser(s);
			}
			catch (string str){
				cout << str << endl;
			}
		}
	 
	}

	return 0;
}
