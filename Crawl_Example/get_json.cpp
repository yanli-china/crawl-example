#define CURL_STATICLIB

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <json.hpp>

// 用于接收 curl 返回的数据
size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl\n";
        return 1;
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.chucknorris.io/jokes/random");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // 可选：设置 User-Agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.88.1");

    //// POST JSON 数据
    //std::string json_data = R"({"username":"test","password":"123"})";
    //curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/login");
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

    //// 设置 Content-Type 头
    //struct curl_slist* headers = nullptr;
    //headers = curl_slist_append(headers, "Content-Type: application/json");
    //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return 1;
    }

    // 解析 JSON
    try {
        auto json = nlohmann::json::parse(response);
        std::cout << "Joke: " << json["value"] << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }

    curl_easy_cleanup(curl);
    return 0;
}
