#include "util/request.hpp"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>


static size_t writeStringCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	std::string *str = (std::string*) userdata;
	size_t len = size * nmemb;
	str->append(ptr, len);
	return len;
}


json_t *requestJson(RequestMethod method, std::string url, json_t *dataJ) {
	CURL *curl = curl_easy_init();
	if (!curl)
		return NULL;

	assert(dataJ);
	char *reqStr;
	if (method != GET_METHOD) {
		reqStr = json_dumps(dataJ, 0);
	}

	// Set URL
	if (method == GET_METHOD) {
		if (dataJ) {
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
	}
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	// Set HTTP method
	switch (method) {
		case GET_METHOD:
			// This is default
			break;
		case POST_METHOD:
			curl_easy_setopt(curl, CURLOPT_POST, true);
			break;
		case PUT_METHOD:
			curl_easy_setopt(curl, CURLOPT_PUT, true);
			break;
		case DELETE_METHOD:
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
			break;
	}

	// Set headers
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	// Body callbacks
	if (method != GET_METHOD)
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reqStr);

	std::string resText;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeStringCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resText);

	// Perform request
	printf("Requesting %s\n", url.c_str());
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	CURLcode res = curl_easy_perform(curl);
	printf("%d\n", res);

	// Cleanup
	if (method != GET_METHOD)
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


static size_t writeFileCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	FILE *file = (FILE*) userdata;
	return fwrite(ptr, size, nmemb, file);
}

static int xferInfoCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	float *progress = (float*) clientp;
	if (progress) {
		if (dltotal <= 0)
			*progress = 1.0;
		else
			*progress = (float)dlnow / dltotal;
	}
	return 0;
}

bool requestDownload(std::string url, std::string filename, float *progress) {
	CURL *curl = curl_easy_init();
	if (!curl)
		return false;

	FILE *file = fopen(filename.c_str(), "w");

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, false);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferInfoCallback);
	curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progress);

	printf("Downloading %s\n", url.c_str());
	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	fclose(file);

	if (res != CURLE_OK)
		remove(filename.c_str());

	return res == CURLE_OK;
}
