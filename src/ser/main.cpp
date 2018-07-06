#include "FTP.h"
#include "sockpair.h"
#include "STL"
#include "Dir.h"

int main()
{
	int sockfd = sockfd_init();

	processpool<CProecss>  *p1 = processpool<CProecss>::create(sockfd);
	p1->run();

	return 0;
}