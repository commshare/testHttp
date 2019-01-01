#ifndef _CCURL_H_
#define _CCURL_H_
#include "curl/curl.h"
#include <string>
using namespace std;
class CCurl
{
public:
	CCurl();
	~CCurl();
	bool DownloadImage(const std::string &url, std::string& data);
private:
	CURL *curl;
};

#endif
