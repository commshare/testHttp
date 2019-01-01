// This is the main DLL file.
//https://github.com/wfdong/captcha/tree/10f7f23145007c8b406e4074b1cf56956cfd21e5/captchaclientv2
#include "stdafx.h"
#include "captchaclientv2.h"
#include "md5.h"  
#include <string.h>





void getMD5(unsigned char* str, char* result, int resultLen) {
	MD5_CTX md5;  
    MD5Init(&md5);
	unsigned char decrypt[16];  
	//unsigned char encrypt[] ="1";
	MD5Update(&md5,str,strlen((char *)str));  
    MD5Final(&md5,decrypt);     
	int i =0;
    for(i=0;i<16;i++)  
    {  
        sprintf(result + i * 2, "%02x",decrypt[i]);  
    }  
}
int addnum(int num1, int num2) {
	/*char result[50] = "\0"; 
	unsigned char ss[] = "1";
	getMD5(ss, result, 16);
	printf("%s\n", result);
	return 0;*/
	CURL *curl;
  CURLcode res;
 
  curl = curl_easy_init();
  if(curl) {
	  char result[50] = "\0";
    curl_easy_setopt(curl, CURLOPT_URL, "http://www.qq.com");
    /* example.com is redirected, so we tell libcurl to follow redirection */ 
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	/* send all data to this function  */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
 
  		/* we pass our 'chunk' struct to the callback function */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)result);
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
	printf("done!\n");
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  return 0;
}

void retryPerform(CURL *curl) {
	while(1) {
		CURLcode res = curl_easy_perform(curl);
	}
	
}

void init() {

	//手动init
}

char* doPost(char* url, char* postFileds) {
	CURL *curl;
	CURLcode res;
 
	curl = curl_easy_init();
	if(curl) {
		char result[2001] = "\0";
		
		curl_easy_setopt(curl, CURLOPT_URL, url);
		/* example.com is redirected, so we tell libcurl to follow redirection */ 
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		//close signal to avoid mutil thread crash 多线程崩溃
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECTTIMEOUT);
		//set to 1 means use POST  设置为1使用POST
		curl_easy_setopt(curl, CURLOPT_POST, 1);  
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFileds);  
	
		/* send all data to this function  */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackBIG);
 
  		/* we pass our 'chunk' struct to the callback function */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)result);
	
		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		
		//printf("result:%s\n", result);
		
		int length = strlen(result);
		if (length == 0) {
			char* errorRet = (char*) malloc(5);
			memset(errorRet, 0, 5);
			memcpy(errorRet, "1005", 4);
			return errorRet;
		}
		char* ret = (char*) malloc(length + 1);
		memset(ret, 0, length + 1);
		memcpy(ret, result, length);
		/* Check for errors */ 
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
			char* errorRet = (char*) malloc(5);
			memset(errorRet, 0, 5);
			memcpy(errorRet, "1005", 4);
		}
		
 
		/* always cleanup */ 
		curl_easy_cleanup(curl);
		return ret;
	}
	char* errorRet = (char*) malloc(5);
	memset(errorRet, 0, 5);
	memcpy(errorRet, "1005", 4);
	return errorRet;
}

size_t
writeCallbackBIG(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realSize = size * nmemb;
	memcpy(userp, contents, min(1000, realSize));
	return realSize;
}

size_t
writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realSize = size * nmemb;
	//printf("%s\n", contents);
	memcpy(userp, contents, min(50, realSize));
	return realSize;
}

long captchaError(char* username, char* passwd, char* token, int codeID) {
	while(1) {
		long ret = docaptchaError(username, passwd, token, codeID);
		if (ret != 1005 && ret !=1012) {
			return ret;
		}
		Sleep(2000);
		printf("doSuc failed ,retry...\n");
	}
}

long docaptchaError(char* username, char* passwd, char* token, int codeID) {
	CURL *curl;
	CURLcode res;
 
	curl = curl_easy_init();
	if(curl) {
		char httpResult[50] = "\0";
		char result[5] = "\0";
		
		//curl_easy_setopt(curl, CURLOPT_URL, "http://www.baidu.com"); 
		//curl_easy_setopt(curl, CURLOPT_URL, "http://114.215.82.1:8009/mdb3");
		//curl_easy_setopt(curl, CURLOPT_URL, "http://ip.kmdama.com/mdb3");
		curl_easy_setopt(curl, CURLOPT_URL, "http://221.206.125.7:8009/mdb3");
		/* example.com is redirected, so we tell libcurl to follow redirection */ 
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		//close signal to avoid mutil thread crash
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECTTIMEOUT);
		//set to 1 means use POST
		curl_easy_setopt(curl, CURLOPT_POST, 1);  
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		char url[200] = "\0";
		_snprintf(url, 200, "username=%s&password=%s&token=%s&codeid=%d", username, passwd, token, codeID);
		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, url);
	
		/* send all data to this function  */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
 
  		/* we pass our 'chunk' struct to the callback function */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)httpResult);
	
		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);

		/*if(res != CURLE_OK){
			//fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			memset(result, 0, 5);
			memcpy(result, "1005", 4);
		}*/

		if(res != CURLE_OK){
			fprintf(stderr, "captchaSuc curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			memset(result, 0, 5);
			memcpy(result, CLIENT_ERROR, 4);
			//memset(result, 0, 50);
			//memcpy(result, curl_easy_strerror(res), 40);
		} else {
			long http_code = 0;
			curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
			if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK)
			{
				//Succeeded
				memset(result, 0, 5);
				memcpy(result, httpResult, 4);
				//memcpy(result, "0", 1);
				//printf("result:%s\n", result);
			}
			else
			{
				//Failed
				memset(result, 0, 5);
				memcpy(result, SEVRER_ERROR, 4);
			}

		}
		
		//printf("result:%s\n", result);
		long ret = atol(result);
		
		/* Check for errors */ 
		if(res != CURLE_OK)
		fprintf(stderr, "captchaSuc curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
		/* always cleanup */ 
		curl_easy_cleanup(curl);
		
		return ret;
	}
	return 1005;
}

int captcha(char* imagefile, char* userna, char* passwd, char* token, char* result)
{
	CURL *curl;
	CURLcode res;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;
	char buf[] = "Expect:";
	curl = curl_easy_init();
	int codeID = 0;
	
 
	/* Fill in the filename field */ 
	curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "username",
               CURLFORM_COPYCONTENTS, userna,
               CURLFORM_END);
	/* Fill in the submit field too, even if this is rarely needed */ 
	curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "password",
               CURLFORM_COPYCONTENTS, passwd,
               CURLFORM_END);
	curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "token",
               CURLFORM_COPYCONTENTS, token,
               CURLFORM_END);
	/* Fill in the file upload field */ 
	curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "myfile",
               CURLFORM_FILE, imagefile,
               CURLFORM_END);

	headerlist = curl_slist_append(headerlist, buf);
	if(curl) {
		char httpResult[50] = "\0";
		//curl_easy_setopt(curl, CURLOPT_URL, "http://114.215.82.1:8009/uploadv3");
		//curl_easy_setopt(curl, CURLOPT_URL, "http://0099azaz.ddns.ms:10010/dov2");
		//close signal to avoid mutil thread crash
		//curl_easy_setopt(curl, CURLOPT_URL, "http://ip.kmdama.com/uploadv3");
		curl_easy_setopt(curl, CURLOPT_URL, "http://221.206.125.7:8009/uploadv3");
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		//set to 1 means use POST  
		curl_easy_setopt(curl, CURLOPT_POST, 1);  
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECTTIMEOUT);
		
		curl_easy_setopt(curl, CURLOPT_VERBOSE, VERVOSE);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		/* send all data to this function  */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
 
  		/* we pass our 'chunk' struct to the callback function */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)httpResult);
		
		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK){
			fprintf(stderr, "captcha curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			memset(result, 0, 5);
			memcpy(result, CLIENT_ERROR, 4);
			//memset(result, 0, 50);
			//memcpy(result, curl_easy_strerror(res), 40);
		} else {
			long http_code = 0;
			curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
			if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK)
			{
				//Succeeded
				char* index = strchr(httpResult, '_');
				if(index == 0) {
					memset(result, 0, 5);
					memcpy(result, httpResult, 4);
					printf("result:%s\n", result);
				} else {
					memset(result, 0, 5);
					memcpy(result, index + 1, 4);
					char codeIDStr[10] = "\0";
					memset(codeIDStr, 0, 10);
					memcpy(codeIDStr, httpResult, index - httpResult);
					codeID = atoi(codeIDStr);
				}
			}
			else
			{
				//Failed
				memset(result, 0, 5);
				memcpy(result, SEVRER_ERROR, 4);
			}

		}
			
		/* always cleanup */ 
		curl_easy_cleanup(curl);
		/* then cleanup the formpost chain */ 
		curl_formfree(formpost);
		/* free slist */ 
		curl_slist_free_all (headerlist);
  }
  return codeID;
  //return;
}

void set_share_handle(CURL* curl_handle)  
{  
    static CURLSH* share_handle = NULL;  
    if (!share_handle)  
    {  
        share_handle = curl_share_init();  
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);  
    }  
    curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);  
    curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 5);  
}  

int captcha_stream(unsigned char* imagebuf,long bytelength, char* userna, char* passwd, char* token, char* result)
{
	CURL *curl;
	CURLcode res;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;
	char buf[] = "Expect:";
	curl = curl_easy_init();
	int codeID = 0;

	//printf("username=%s, pwd=%s, token=%s", userna, passwd, token);
	//for md5 hash
	/*char hashpwd[50] = "\0"; 
	unsigned char ss[] = "1";
	getMD5(ss, hashpwd, 16);*/
	curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "username",
               CURLFORM_COPYCONTENTS, userna,
               CURLFORM_END);
	/* Fill in the submit field too, even if this is rarely needed */ 
	curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "password",
               CURLFORM_COPYCONTENTS, passwd,
               CURLFORM_END);
	curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "token",
               CURLFORM_COPYCONTENTS, token,
               CURLFORM_END);
	/* Fill in the file upload field */ 
	curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "myfile",
               CURLFORM_PTRCONTENTS, imagebuf,
               CURLFORM_CONTENTSLENGTH, bytelength, 
               CURLFORM_END);

	headerlist = curl_slist_append(headerlist, buf);
	if(curl) {
		char httpResult[50] = "\0";
		//curl_easy_setopt(curl, CURLOPT_URL, "http://1.56.184.11:40010/dotest");
		//curl_easy_setopt(curl, CURLOPT_URL, "http://114.215.82.1:8009/uploadv3");
		//curl_easy_setopt(curl, CURLOPT_URL, "http://ip.kmdama.com/uploadv3");
		curl_easy_setopt(curl, CURLOPT_URL, "http://221.206.125.7:8009/uploadv3");
		//close signal to avoid mutil thread crash
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 0);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, CONNECTTIMEOUT);
		//set to 1 means use POST
		curl_easy_setopt(curl, CURLOPT_POST, 1);  
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		
		curl_easy_setopt(curl, CURLOPT_VERBOSE, VERVOSE);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		
		/* send all data to this function  */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
 
  		/* we pass our 'chunk' struct to the callback function */ 
  		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)httpResult);

		//set_share_handle(curl);
		//curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
		
		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK){
			fprintf(stderr, "captcha_stream curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			memset(result, 0, 5);
			memcpy(result, CLIENT_ERROR, 4);
			//memset(result, 0, 50);
			//memcpy(result, curl_easy_strerror(res), 40);
		} else {
			long http_code = 0;
			curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
			if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK)
			{
				//Succeeded
				char* index = strchr(httpResult, '_');
				if(index == 0) {
					memset(result, 0, 5);
					memcpy(result, httpResult, 4);
					printf("result:%s\n", result);
				} else {
					memset(result, 0, 5);
					memcpy(result, index + 1, 4);
					char codeIDStr[10] = "\0";
					memset(codeIDStr, 0, 10);
					memcpy(codeIDStr, httpResult, index - httpResult);
					codeID = atoi(codeIDStr);
				}
			}
			else
			{
				//Failed
				printf("%d\n", http_code);
				memset(result, 0, 5);
				memcpy(result, SEVRER_ERROR, 4);
			}
		}
		//memset(result, 0, 5);
		//memcpy(result, "2222", 4);
		/* always cleanup */ 
		curl_easy_cleanup(curl);
		/* then cleanup the formpost chain */ 
		curl_formfree(formpost);
		/* free slist */ 
		curl_slist_free_all (headerlist);
		
  }
	return codeID;
  
  //return 0;
}

//-----
//sub user operation
//-----

long addsubuser(char* token, char* subusername, char* subpasswd){
	char postfileds[POSTSIZE] = "\0";
	_snprintf(postfileds, POSTSIZE, "token=%s&subname=%s&subpasswd=%s", token, subusername, subpasswd);
	//char* ret = doPost("http://114.215.82.1:8080/addsubuser", postfileds);
	//char* ret = doPost("http://gl.kmdama.com/addsubuser", postfileds);
	char* ret = doPost("http://221.206.125.7:80/addsubuser", postfileds);
	long r = atol(ret);
	free(ret);
	return r;
}

void querysubuser(char* token, char* subusername, char* result, long length){
	char postfileds[POSTSIZE] = "\0";
	_snprintf(postfileds, POSTSIZE, "token=%s&subname=%s", token, subusername);
	//char* ret = doPost("http://114.215.82.1:8080/querysubuser", postfileds);
	//char* ret = doPost("http://gl.kmdama.com/querysubuser", postfileds);
	char* ret = doPost("http://221.206.125.7:80/querysubuser", postfileds);
	memset(result, 0, length);
	memcpy(result, ret, min(strlen(ret), length));
	free(ret);
}

long loginsubuser(char* token, char* subusername, char* subpasswd){
	char postfileds[POSTSIZE] = "\0";
	_snprintf(postfileds, POSTSIZE, "token=%s&subname=%s&subpasswd=%s", token, subusername, subpasswd);
	//char* ret = doPost("http://114.215.82.1:8080/loginsubuser", postfileds);
	//char* ret = doPost("http://gl.kmdama.com/loginsubuser", postfileds);
	char* ret = doPost("http://221.206.125.7:80/loginsubuser", postfileds);
	long r = atol(ret);
	free(ret);
	return r;
}

//author control api

long delsubuser(char* userna, char* passwd, char* subusername){
	char postfileds[POSTSIZE] = "\0";
	_snprintf(postfileds, POSTSIZE, "username=%s&password=%s&subname=%s", userna, passwd, subusername);
	//char* ret = doPost("http://114.215.82.1:8080/delsubuser", postfileds);
	//char* ret = doPost("http://gl.kmdama.com/delsubuser", postfileds);
	char* ret = doPost("http://221.206.125.7:80/delsubuser", postfileds);
	long r = atol(ret);
	free(ret);
	return r;
}

long modsubuser(char* userna, char* passwd, char* subusername, char* subpasswd, int subdeltcount){
	char postfileds[POSTSIZE] = "\0";
	_snprintf(postfileds, POSTSIZE, "username=%s&password=%s&subname=%s&subpassword=%s&subleftcount_delt=%d", userna, passwd, subusername, subpasswd, subdeltcount);
	//char* ret = doPost("http://114.215.82.1:8080/modsubuser", postfileds);
	//char* ret = doPost("http://gl.kmdama.com/modsubuser", postfileds);
	char* ret = doPost("http://221.206.125.7:80/modsubuser", postfileds);
	long r = atol(ret);
	free(ret);
	return r;
}

void queryallsubuser(char* userna, char* passwd, char* result, long length){
	char postfileds[POSTSIZE] = "\0";
	_snprintf(postfileds, POSTSIZE, "username=%s&password=%s", userna, passwd);
	//char* ret = doPost("http://114.215.82.1:8080/queryallsubuser", postfileds);
	//char* ret = doPost("http://gl.kmdama.com/queryallsubuser", postfileds);
	char* ret = doPost("http://221.206.125.7:80/queryallsubuser", postfileds);
	memset(result, 0, length);
	memcpy(result, ret, min(strlen(ret), length));
	free(ret);
}

void queryuserinfo(char* userna, char* passwd, char* result, long length){
	char postfileds[POSTSIZE] = "\0";
	_snprintf(postfileds, POSTSIZE, "username=%s&password=%s", userna, passwd);
	//char* ret = doPost("http://114.215.82.1:8080/queryuserinfo", postfileds);
	//char* ret = doPost("http://gl.kmdama.com/queryuserinfo", postfileds);
	char* ret = doPost("http://221.206.125.7:80/queryuserinfo", postfileds);
	memset(result, 0, length);
	memcpy(result, ret, min(strlen(ret), length));
	free(ret);
}


//
//all e interface
//
int __stdcall captcha_stream_e(unsigned char* imagefile, long byteLength, char* userna, char* passwd, char* token, char* result)
{
	return captcha_stream(imagefile, byteLength, userna, passwd, token, result);
}

int __stdcall captcha_e(char* imagefile, char* userna, char* passwd, char* token, char* result)
{
	return captcha(imagefile, userna, passwd, token, result);
}

long __stdcall captchaError_e(char* username, char* passwd, char* token, int codeID)
{
	return captchaError(username, passwd, token, codeID);
}

long __stdcall addsubuser_e(char* token, char* subusername, char* subpasswd)
{
	return addsubuser(token, subusername, subpasswd);
}
void __stdcall querysubuser_e(char* token, char* subusername, char* result, long length)
{
	return querysubuser(token, subusername, result, length);
}
long __stdcall loginsubuser_e(char* token, char* subusername, char* subpasswd)
{
	return loginsubuser(token,subusername, subpasswd);
}

void __stdcall test_e(){
}

long __stdcall delsubuser_e(char* userna, char* passwd, char* subusername)
{
	return delsubuser(userna, passwd, subusername);
}

long __stdcall modsubuser_e(char* userna, char* passwd, char* subusername, char* subpasswd, int subdeltcount)
{
	return modsubuser(userna, passwd, subusername, subpasswd, subdeltcount);
}

void __stdcall queryallsubuser_e(char* userna, char* passwd, char* result, long length)
{
	return queryallsubuser(userna, passwd, result, length);
}

void __stdcall queryuserinfo_e(char* userna, char* passwd, char* result, long length)
{
	return queryuserinfo(userna, passwd, result,length);
}