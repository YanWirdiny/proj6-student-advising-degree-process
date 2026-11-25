#include "fetcher.h"
#include <curl/curl.h>
#include <iostream>
#include <string>
using namespace std ; 

static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    if (!userdata || !ptr) {
        return 0;
    }
    size_t total_size = size * nmemb;
    string* out = static_cast<string*>(userdata);
    out->append(ptr, total_size);
    return total_size;
}

Fetcher::Fetcher(int delayMs) : politeness_(delayMs) {}

string Fetcher::fetch(const string& url) {
    politeness_.wait();   // pause before every request so we don't hammer the server

    string response;

    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "fetch: curl_easy_init failed" << endl;
        return "";
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "StudentAdvisingCrawler/v1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "fetch: " << url << " failed: " << curl_easy_strerror(res) << endl;
        response.clear();
    } else {
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (status >= 400) {
            cerr << "fetch: " << url << " returned HTTP " << status << endl;
            response.clear();
        }
    }

    curl_easy_cleanup(curl);
    return response;
}
