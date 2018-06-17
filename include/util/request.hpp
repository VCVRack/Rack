#pragma once
#include <string>
#include <jansson.h>


namespace rack {


enum RequestMethod {
	METHOD_GET,
	METHOD_POST,
	METHOD_PUT,
	METHOD_DELETE,
};

/** Requests a JSON API URL over HTTP(S), using the data as the query (GET) or the body (POST, etc)
Caller must json_decref().
*/
json_t *requestJson(RequestMethod method, std::string url, json_t *dataJ);
/** Returns true if downloaded successfully */
bool requestDownload(std::string url, std::string filename, float *progress);
/** URL-encodes `s` */
std::string requestEscape(std::string s);
/** Computes the SHA256 of the file at `filename` */
std::string requestSHA256File(std::string filename);


} // namespace rack
