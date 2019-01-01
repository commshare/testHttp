// captchaclientv2.h


#include <windows.h>

#define DLLEXPORT __declspec(dllexport)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define HOST "http://0099azaz.ddns.ms:10010"

//return value space
#define RETSIZE 50

#define BUFFERSIZE 200

#define POSTSIZE 200

#define VERVOSE 0

#define CONNECTTIMEOUT 100

#define SEVRER_ERROR "1012"

#define CLIENT_ERROR "1005"

extern "C" __declspec(dllexport) int addnum(int num1, int num2);

extern "C" __declspec(dllexport) long captchaError(char* userna, char* passwd, char* token, int codeID);

long docaptchaError(char* userna, char* passwd, char* token, int codeID);

extern "C" __declspec(dllexport) int captcha(char* imagefile, char* userna, char* passwd, char* token, char* result);

extern "C" __declspec(dllexport) int captcha_stream(unsigned char* imagefile,long bytelength, char* userna, char* passwd, char* token, char* result);

size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
size_t writeCallbackBIG(void *contents, size_t size, size_t nmemb, void *userp);

char* doPost(char* url, char* postFileds);

extern "C" __declspec(dllexport) long addsubuser(char* token, char* subusername, char* subpasswd);
extern "C" __declspec(dllexport) void querysubuser(char* token, char* subusername, char* result, long length);
extern "C" __declspec(dllexport) long loginsubuser(char* token, char* subusername, char* subpasswd);

//author control
extern "C" __declspec(dllexport) long delsubuser(char* userna, char* passwd, char* subusername);
extern "C" __declspec(dllexport) long modsubuser(char* userna, char* passwd, char* subusername, char* subpasswd, int subdeltcount);
extern "C" __declspec(dllexport) void queryallsubuser(char* userna, char* passwd, char* result, long length);
extern "C" __declspec(dllexport) void queryuserinfo(char* userna, char* passwd, char* result, long length);

int __stdcall captcha_stream_e(unsigned char* imagefile, long byteLength, char* userna, char* passwd, char* token, char* result);

int __stdcall captcha_e(char* imagefile, char* userna, char* passwd, char* token, char* result);

long __stdcall captchaError_e(char* username, char* passwd, char* token, int codeID);

long __stdcall addsubuser_e(char* token, char* subusername, char* subpasswd);

void __stdcall querysubuser_e(char* token, char* subusername, char* result, long length);

long __stdcall loginsubuser_e(char* token, char* subusername, char* subpasswd);

void __stdcall test_e();

long __stdcall delsubuser_e(char* userna, char* passwd, char* subusername);

long __stdcall modsubuser_e(char* userna, char* passwd, char* subusername, char* subpasswd, int subdeltcount);

void __stdcall queryallsubuser_e(char* userna, char* passwd, char* result, long length);

void __stdcall queryuserinfo_e(char* userna, char* passwd, char* result, long length);