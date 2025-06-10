#define CURL_STATICLIB
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <curl/curl.h>

std::mutex coutMutex;
std::mutex semaphoreMutex;
std::condition_variable semaphoreCond;
int activeThreads = 0;
const int MAX_THREADS = 2;


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}


std::string fetchURL(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "Fetch failed: " << url << " - " << curl_easy_strerror(res) << '\n';
        }

        curl_easy_cleanup(curl);
    }
    return response;
}

void extractLinks(const std::string& html, const std::string& url) {

    std::regex linkRegex(R"(<a\s+(?:[^>]*?\s+)?href=\"([^"]*)\")");

    auto begin = std::sregex_iterator(html.begin(), html.end(), linkRegex);
    auto end = std::sregex_iterator();

    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "Links from: " << url << '\n';
    for (auto it = begin; it != end; ++it) {
        std::cout << "  -> " << (*it)[1].str() << '\n';
    }
}

void fetchAndExtractLimited(const std::string& url) {

    {
        std::unique_lock<std::mutex> lock(semaphoreMutex);

        semaphoreCond.wait(lock, [] { return activeThreads < MAX_THREADS; });  // 如果 activeThreads >= MAX_THREADS，则挂起当前线程，等待资源释放。
        
        ++activeThreads;
    }

    std::string html = fetchURL(url);
    
    if (!html.empty()) {
        extractLinks(html, url);
    }

    {
        std::lock_guard<std::mutex> lock(semaphoreMutex);
        --activeThreads;
    }
    semaphoreCond.notify_all();  // 唤醒等待的线程
}

// 用于构造合法的文件名（将 https://github.com 变成 github_com.html）
std::string urlToFilename(const std::string& url) {
    std::string filename = url;
    std::regex removeScheme(R"(https?://)");
    filename = std::regex_replace(filename, removeScheme, "");
    std::replace(filename.begin(), filename.end(), '/', '_');
    std::replace(filename.begin(), filename.end(), '.', '_');
    return filename + ".html";
}

static size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    FILE* fp = (FILE*)userp;
    return fwrite(contents, size, nmemb, fp);
}

void downloadToFile(const std::string& url) {
    {
        std::unique_lock<std::mutex> lock(semaphoreMutex);
        semaphoreCond.wait(lock, [] { return activeThreads < MAX_THREADS; });
        ++activeThreads;
    }

    CURL* curl = curl_easy_init();

    if (curl) {
        std::string filename = urlToFilename(url);
        FILE* fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "Failed to open file for writing: " << filename << '\n';
        }
        else {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

            CURLcode res = curl_easy_perform(curl);
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                if (res != CURLE_OK) {
                    std::cerr << "Failed to download " << url << ": " << curl_easy_strerror(res) << '\n';
                }
                else {
                    std::cout << "Downloaded: " << url << " -> " << filename << '\n';
                }
            }

            fclose(fp);
        }
        curl_easy_cleanup(curl);
    }

    {
        std::lock_guard<std::mutex> lock(semaphoreMutex);
        --activeThreads;
    }
    semaphoreCond.notify_all();
}


int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    std::vector<std::string> urls = {
        "https://www.github.com",
        "https://www.wikipedia.org",
        "https://www.stackoverflow.com",
        "https://www.reddit.com",
        "https://www.cnn.com"
    };

    std::vector<std::thread> threads;

    for (const auto& url : urls) {
        threads.emplace_back(fetchAndExtractLimited, url);
    }

    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }

    std::vector<std::thread> threads2;
    for (const auto& url : urls) {
        threads2.emplace_back(downloadToFile, url);
    }

    for (auto& t : threads2) {
        if (t.joinable())
            t.join();
    }

    curl_global_cleanup();
    return 0;
}
