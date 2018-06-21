#include "LinuxFun"
#include "STL"
#include "sockpair.h"

int main()
{
	int sockfd = sockfd_init();

	Client p;
	p.cliRun();
	return 0;
}