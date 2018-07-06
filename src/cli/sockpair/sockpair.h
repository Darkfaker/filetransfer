
#ifndef _DPOPEN_H
#define _DPOPEN_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

	FILE *dpopen(const char *command);
	int dpclose(FILE *stream);
	int dphalfclose(FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* _DPOPEN_H */
