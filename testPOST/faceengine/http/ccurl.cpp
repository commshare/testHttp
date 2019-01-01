#include "ccurl.h"

#define DEFAULT_CURL_TIMEOUT			3		// s
#define DEFAULT_IMAGE_MAX_SIZE			(30 * 1024 * 1024) // 3M
static size_t write_data(void *buff, size_t size, size_t nmemb, void *userdata)
{
//	assert(userdata != NULL);
	if (!userdata)
	{
		return 0;
	}
	
	std::string& str = *(std::string*)userdata;
	if(str.size() >= DEFAULT_IMAGE_MAX_SIZE)
		return 0;

	str.append((char*)buff, size * nmemb);

	return size * nmemb;
}

bool CCurl::DownloadImage(const std::string &url, std::string& data)
{
	data.clear();
	CURLcode res = CURLE_FAILED_INIT;

	do{
		if(!curl)
		{
			curl = curl_easy_init();
			if(curl)
			{
				curl_slist *http_headers = NULL;
				http_headers = curl_slist_append(http_headers, "Connection: keep-alive");
				http_headers = curl_slist_append(http_headers, "Cache-Control: no-cache");
				http_headers = curl_slist_append(http_headers, "Pragma: no-cache");
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);

				curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
				curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);
				curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
			}
		}
		if (!curl) 	break;

		res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());                  
		if (res != CURLE_OK) break;

		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);  
		if (res != CURLE_OK) break;

		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
		if (res != CURLE_OK) break;

		res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, DEFAULT_CURL_TIMEOUT);
		if (res != CURLE_OK) break;

		res = curl_easy_perform(curl);

	}while(0);

	return res == CURLE_OK ? true : false;
}

CCurl::CCurl()
{
	curl = NULL;
}

CCurl::~CCurl()
{
	if(!curl)
		curl_easy_cleanup(curl);

	curl = NULL;
}