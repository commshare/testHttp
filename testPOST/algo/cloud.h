#ifndef __CLOUD_H_
#define __CLOUD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "curl.h"

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct stCloudRes {
	char	dada[4];
	int		i;
	int   startflag;
	int		len;
	char	data[1024];
}stCloudRes_t;

int cloud_http_pst(const char *url, const char *postParams, stCloudRes_t *response);
int cloud_http_get(const char *url, stCloudRes_t *response); 


#define CLOUD_ENABLE_DEBUG 0

#ifdef __cplusplus
}
#endif






#endif
