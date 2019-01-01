#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <vector>
#include <map>

#define _HTTP_CLIENT_TIMEOUT_	1000			//ms
static int HTTP_CLIENT_OK		= 0;
static int HTTP_CLIENT_START	= -58000;
static int HTTP_CLIENT_ARGS		= -58001;
static int HTTP_CLIENT_CONNECT	= -58002;
static int HTTP_CLIENT_MEMORY	= -58003;
static int HTTP_CLIENT_DATA_ADD	= -58004;
static int HTTP_CLIENT_TIMEOUT	= -58010;

#pragma pack(push, 1)
typedef struct _CurlFileData_
{
	char	fileName[260];
	int		fileLen;
	void *	fileData;
}CurlFileData;
#pragma pack(pop) // #pragma pack(push, 1)

#pragma pack(push, 1)
typedef struct _CurlResult_
{
	int			 errCode;
	int			 resultLen;
	void		*resultData;
}CurlResult;
#pragma pack(pop) // #pragma pack(push, 1)


class CHttpClient
{
public:
	CHttpClient(void);
	~CHttpClient(void);

	int PostRequest(const std::string &url, 
		const std::string &vcData, 
		std::string &vcResponse, 
		const std::vector<std::string> &head = std::vector<std::string>());

	int PostFileRequest(const std::string & url, 
		const char *buff, 
		const int size, 
		std::string &strResponse);

	int Post2FileRequest(const std::string & url, 
		const char *buff1, 
		const int size1, 
		const char *buff2, 
		const int size2, 
		std::string &strResponse);
	int PostMultiFileRequest(const std::string & url, const CurlFileData *files, const int fileLen, std::string &strResponse);


	int GetRequest(const std::string &url, 
		std::string &vcResponse, 
		const std::vector<std::string> &head = std::vector<std::string>());

	void SetTimeout(int _timeout, int _connecttimeout);

private:
	static size_t WriteData(void *buff, size_t size, size_t nmemb, void *userp);
private:
	int m_timeout;
};

#endif