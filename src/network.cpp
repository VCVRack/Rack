#include <network.hpp>
#define CURL_STATICLIB
#include <curl/curl.h>


namespace rack {
namespace network {


static size_t writeStringCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	std::string *str = (std::string*) userdata;
	size_t len = size * nmemb;
	str->append(ptr, len);
	return len;
}


json_t *requestJson(Method method, std::string url, json_t *dataJ) {
	CURL *curl = curl_easy_init();
	assert(curl);

	char *reqStr = NULL;

	// Process data
	if (dataJ) {
		if (method == METHOD_GET) {
			// Append ?key=value&... to url
			url += "?";
			bool isFirst = true;
			const char *key;
			json_t *value;
			json_object_foreach(dataJ, key, value) {
				if (json_is_string(value)) {
					if (!isFirst)
						url += "&";
					url += key;
					url += "=";
					const char *str = json_string_value(value);
					size_t len = json_string_length(value);
					char *escapedStr = curl_easy_escape(curl, str, len);
					url += escapedStr;
					curl_free(escapedStr);
					isFirst = false;
				}
			}
		}
		else {
			reqStr = json_dumps(dataJ, 0);
		}
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	// Set HTTP method
	switch (method) {
		case METHOD_GET:
			// This is CURL's default
			break;
		case METHOD_POST:
			curl_easy_setopt(curl, CURLOPT_POST, true);
			break;
		case METHOD_PUT:
			curl_easy_setopt(curl, CURLOPT_PUT, true);
			break;
		case METHOD_DELETE:
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
			break;
	}

	// Set headers
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	// Body callbacks
	if (reqStr)
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reqStr);

	std::string resText;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeStringCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resText);

	// Perform request
	// DEBUG("Requesting %s", url.c_str());
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	CURLcode res = curl_easy_perform(curl);

	// Cleanup
	if (reqStr)
		free(reqStr);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	if (res == CURLE_OK) {
		// Parse JSON response
		json_error_t error;
		json_t *rootJ = json_loads(resText.c_str(), 0, &error);
		return rootJ;
	}
	return NULL;
}


static int xferInfoCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	float *progress = (float*) clientp;
	if (progress) {
		if (dltotal <= 0)
			*progress = 0.f;
		else
			*progress = (float)dlnow / dltotal;
	}
	return 0;
}

bool requestDownload(std::string url, const std::string &filename, float *progress) {
	CURL *curl = curl_easy_init();
	if (!curl)
		return false;

	FILE *file = fopen(filename.c_str(), "wb");
	if (!file)
		return false;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, false);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferInfoCallback);
	curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progress);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	// Fail on 4xx and 5xx HTTP codes
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	fclose(file);

	if (res != CURLE_OK)
		remove(filename.c_str());

	return res == CURLE_OK;
}

std::string encodeUrl(const std::string &s) {
	CURL *curl = curl_easy_init();
	assert(curl);
	char *escaped = curl_easy_escape(curl, s.c_str(), s.size());
	std::string ret = escaped;
	curl_free(escaped);
	curl_easy_cleanup(curl);
	return ret;
}

std::string urlPath(const std::string &url) {
	CURLU *curl = curl_url();
	DEFER({
		curl_url_cleanup(curl);
	});
	if (curl_url_set(curl, CURLUPART_URL, url.c_str(), 0))
		return "";
	char *buf;
	if (curl_url_get(curl, CURLUPART_PATH, &buf, 0))
		return "";
	std::string ret = buf;
	curl_free(buf);
	return ret;
}


} // namespace network
} // namespace rack
