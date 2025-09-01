#pragma once
#include <Windows.h>
#include <string>
#include <map>
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "json.hpp"

using json = nlohmann::json;

void InitOffsetsLogger();

class OffsetsLoader {
private:
    const std::string OFFSETS_URL = "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/offsets.json";
    const std::string CLIENT_DLL_URL = "https://github.com/a2x/cs2-dumper/raw/refs/heads/main/output/client_dll.json";

    std::map<std::string, std::ptrdiff_t> loadedOffsets;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    std::string HttpRequest(const std::string& url);
    json DownloadJson(const std::string& url);

public:
    OffsetsLoader();
    ~OffsetsLoader();

    std::ptrdiff_t GetOffset(const std::string& name);
    bool OffsetsAreLoaded() const;
};