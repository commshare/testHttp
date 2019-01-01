/*
 * WebBrowser.cpp
 *
 *  Created on: Jan 6, 2014
 *      Author: denia
 */

#include <iostream>
#include <sstream>
#include <cstring>
#include <curl/curl.h>

#include "LogDestination.h"
#include "WebBrowser.h"

namespace domoaster {

WebBrowser::WebBrowser(std::string url)
{
	this->url = url;
}

WebBrowser::~WebBrowser()
{
}

Json::Value WebBrowser::doGet(std::string url, std::string parameters)
{
	CURL *curl;
	CURLcode res;

	Json::Reader reader;
	Json::Value response;

	curl = curl_easy_init();
	if(curl) {
		// Build get request
		std::stringstream request;
		std::string req_param;

		request << "http://" << this->url << url;
		DOMOASTER_DEBUG << "Full uri: " << request.str();
		req_param = request.str();

		curl_easy_setopt(curl, CURLOPT_URL, req_param.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// Define the callback function and the data pointer
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &domoaster::WebBrowser::responseToJsonObjectCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);

		if(res != CURLE_OK){
			DOMOASTER_WARN << "curl_easy_perform() failed: " << curl_easy_strerror(res);
		}
		curl_easy_cleanup(curl);
	}

	return response;
}

Json::Value WebBrowser::doPost(std::string url, Json::Value *post_data)
{
	CURL *curl;
	CURLcode res;
	char *error_buf;
	Json::Reader reader;
	Json::Value response;
	std::stringstream data;
	std::string spdata;

	error_buf = (char*)malloc(sizeof(char) * CURL_ERROR_SIZE * 2);
	if(error_buf == NULL){
		DOMOASTER_FATAL << "Unable to allocate memory";
		return 0;
	}
	curl = curl_easy_init();
	if(curl) {
		// Build get request
		std::stringstream request;
		std::string req_param;
		request << "http://" << this->url << url;
		DOMOASTER_DEBUG << "Full uri: " << request.str();
		req_param = request.str();
		try{
			data << "json=" << post_data->toStyledString();
			spdata = data.str();
		}catch(std::exception & ex){
			DOMOASTER_WARN << "Exception :" << ex.what();
		}

		DOMOASTER_DEBUG << "POST Param " << spdata;
		curl_easy_setopt(curl, CURLOPT_URL, req_param.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, spdata.c_str());
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		// Define the callback function and the data pointer
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &domoaster::WebBrowser::responseToJsonObjectCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);

		if(res != CURLE_OK){
			DOMOASTER_WARN << "curl_easy_perform() failed: " << curl_easy_strerror(res);
			DOMOASTER_WARN << "Error is :" << error_buf;
		}
		curl_easy_cleanup(curl);
		free(error_buf);
	}

	return response;
}

Json::Value WebBrowser::doPost(std::string url, std::string  post_data)
{
	CURL *curl;
	CURLcode res;
	char *error_buf;
	Json::Reader reader;
	Json::Value response;
	std::stringstream data;
	std::string spdata;

	error_buf = (char*)malloc(sizeof(char) * CURL_ERROR_SIZE * 2);
	if(error_buf == NULL){
		DOMOASTER_FATAL << "Unable to allocate memory";
		return 0;
	}
	curl = curl_easy_init();
	if(curl) {
		// Build get request
		std::stringstream request;
		std::string req_param;
		request << "http://" << this->url << url;
		DOMOASTER_DEBUG << "Full uri: " << request.str() << " POST Param : " << post_data;

		req_param = request.str();

		curl_easy_setopt(curl, CURLOPT_URL, req_param.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		// Define the callback function and the data pointer
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &domoaster::WebBrowser::responseToJsonObjectCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);

		if(res != CURLE_OK){
			DOMOASTER_WARN << "curl_easy_perform() failed: " << curl_easy_strerror(res);
			DOMOASTER_WARN << "Error is :" << error_buf;
		}
		curl_easy_cleanup(curl);
		free(error_buf);
	}

	return response;
}

size_t WebBrowser::responseToJsonObjectCallback( char *response, size_t size, size_t nmemb, void *userdata)
{
	size_t realsize = size * nmemb;

	Json::Value *result = (Json::Value *)userdata;

	Json::Reader reader;
	bool parsingSuccessful = reader.parse(response, *result);
	if ( !parsingSuccessful )
	{
	    // report to the user the failure and their locations in the document.
	    DOMOASTER_WARN  << "Failed to parse configuration\n"
	               << reader.getFormatedErrorMessages();
	    DOMOASTER_DEBUG << response;
	}

	return realsize;
}

} ; // namespace domoaster