#include "cloud.h"

//https://github.com/ghaucience/algo/blob/fbb00e6abefdf65316060bbe8a68db9f998488f1/lib/cloud.cpp 
// reply of the requery  
static size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream) {  

	stCloudRes_t *cr = (stCloudRes_t*)stream;

	int		len		= size * nmemb;
	char *data	= (char *)ptr;

	/*
	int i = 0;
	printf("---------------------------------start\n");
	for (i = 0; i < len; i++) {
		printf("[%02X] ", data[i]&0xff);
	}
	printf("\n");
	printf("---------------------------------end\n");
	*/
	
	if (cr->startflag == 0) {
		if (len >= 2) {
			memcpy(cr->dada + cr->i, data + len - 2, 2);
			cr->i = (cr->i + 2) % 4;

			if (memcmp(cr->dada, "\x0d\x0a\x0d\x0a", 4) == 0 && len == 2) {
				cr->startflag = 1;
				len = 0;
			} else {
					/*
				printf("%02x%02x%02x%02x\n", 
					cr->dada[0]&0xff, 
					cr->dada[1]&0xff, 
					cr->dada[2]&0xff, 
					cr->dada[3]&0xff);
					*/
			}
		}
	} else if (cr->startflag)  {

		memcpy(cr->data + cr->len, data, len);

		cr->len += len;

		cr->data[cr->len] = 0;
		//printf("data is %s\n", cr->data);
	}


	return size*nmemb;
} 

int cloud_http_pst(const char *url, const char *postParams, stCloudRes_t *response) {
	CURLcode res = CURL_LAST;

	CURL *curl = curl_easy_init();  
	if (curl)   {  
		curl_easy_setopt(curl, CURLOPT_POST, 1); // post req  
		curl_easy_setopt(curl, CURLOPT_URL, url); // url  
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams); // params  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // if want to use https  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); // set peer and host verify false  
		curl_easy_setopt(curl, CURLOPT_VERBOSE, CLOUD_ENABLE_DEBUG);  
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);  
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);  
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);  
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);  

		res = curl_easy_perform(curl);  

		curl_easy_cleanup(curl);  
	}  

	if (res != CURLE_OK) {
		return -1;
	}

	return 0;
}
int cloud_http_get(const char *url, stCloudRes_t *response) {
	CURLcode res = CURL_LAST;

	CURL *curl = curl_easy_init();  
	if (curl)  {  
		curl_easy_setopt(curl, CURLOPT_URL, url); // url  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // if want to use https  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); // set peer and host verify false  
		curl_easy_setopt(curl, CURLOPT_VERBOSE, CLOUD_ENABLE_DEBUG);  
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);  
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);  
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);  
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3); // set transport and time out time  
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);  

		res = curl_easy_perform(curl);  

		curl_easy_cleanup(curl);  
	}  

	if (res != CURLE_OK) {
		return -1;
	}

	return 0;
}


