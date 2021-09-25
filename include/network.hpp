#pragma once
#include <map>

#include <jansson.h>

#include <common.hpp>


namespace rack {
/** Networking functions for HTTP requests, downloads, and URLs */
namespace network {


typedef std::map<std::string, std::string> CookieMap;

enum Method {
	METHOD_GET,
	METHOD_POST,
	METHOD_PUT,
	METHOD_DELETE,
};

void init();
/** Requests a JSON API URL over HTTP(S), using the data as the query (GET) or the body (POST, etc)
Caller must json_decref() if return value is non-NULL.
*/
json_t* requestJson(Method method, const std::string& url, json_t* dataJ = NULL, const CookieMap& cookies = {});
/** Returns true if downloaded successfully.
If `progress` is non-NULL, the value is updated from 0 to 1 while downloading.
*/
bool requestDownload(const std::string& url, const std::string& filename, float* progress, const CookieMap& cookies = {});
/** URL-encodes a string. */
std::string encodeUrl(const std::string& s);
/** Returns the path portion of the URL.
Example:

	urlPath("https://example.com/foo/index.html") // Returns "/foo/index.html"
*/
std::string urlPath(const std::string& url);


} // namespace network
} // namespace rack
