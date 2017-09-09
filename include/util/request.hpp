#pragma once

#include <string>
#include <jansson.h>


enum RequestMethod {
	GET_METHOD,
	POST_METHOD,
	PUT_METHOD,
	DELETE_METHOD,
};

/** Must be called before using this API */
void requestInit();
void requestDestroy();

/** Requests a JSON API URL over HTTP(S), using the data as the query (GET) or the body (POST, etc) */
json_t *requestJson(RequestMethod method, std::string url, json_t *dataJ);
/** Returns the filename, blank if unsuccessful */
bool requestDownload(std::string url, std::string filename, float *progress);

