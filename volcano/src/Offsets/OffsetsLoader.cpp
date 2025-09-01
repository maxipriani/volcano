#include <iostream>
#include <chrono>
#include <thread>
#include "OffsetsLoader.h"

void InitOffsetsLogger() {
    try {
        auto logger = spdlog::basic_logger_mt("offsets", "offsetsLoader.log", true);
        logger->set_pattern("[%d-%m-%Y %H:%M:%S] [%l] %v");
        logger->flush_on(spdlog::level::info);

        logger->info("===============================================");
        logger->info("             Volcano ESP by flosur");
        logger->info("===============================================");

        spdlog::register_logger(logger);
    }
    catch (const spdlog::spdlog_ex& e) {
        std::cout << "Offsets log init failed: " << e.what() << std::endl;
    }
}

size_t OffsetsLoader::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

OffsetsLoader::OffsetsLoader() {
    if (!spdlog::get("offsets")) {
        InitOffsetsLogger();
    }

    spdlog::get("offsets")->info("Loading offsets from GitHub");

    json offsetsJson = DownloadJson(OFFSETS_URL);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    json clientJson = DownloadJson(CLIENT_DLL_URL);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // offsets.json
    if (!offsetsJson.empty() && offsetsJson.contains("client.dll") && offsetsJson.contains("engine2.dll")) {

        // client.dll
        if (offsetsJson["client.dll"].contains("dwEntityList")) {
            loadedOffsets["dwEntityList"] = offsetsJson["client.dll"]["dwEntityList"];
            spdlog::get("offsets")->debug("Loaded offset: dwEntityList = 0x{:X}",
                offsetsJson["client.dll"]["dwEntityList"].get<std::ptrdiff_t>());
        }

        if (offsetsJson["client.dll"].contains("dwLocalPlayerPawn")) {
            loadedOffsets["dwLocalPlayerPawn"] = offsetsJson["client.dll"]["dwLocalPlayerPawn"];
            spdlog::get("offsets")->debug("Loaded offset: dwLocalPlayerPawn = 0x{:X}",
                offsetsJson["client.dll"]["dwLocalPlayerPawn"].get<std::ptrdiff_t>());
        }

        if (offsetsJson["client.dll"].contains("dwViewAngles")) {
            loadedOffsets["dwViewAngles"] = offsetsJson["client.dll"]["dwViewAngles"];
            spdlog::get("offsets")->debug("Loaded offset: dwViewAngles = 0x{:X}",
                offsetsJson["client.dll"]["dwViewAngles"].get<std::ptrdiff_t>());
        }
        if (offsetsJson["client.dll"].contains("dwViewMatrix")) {
            loadedOffsets["dwViewMatrix"] = offsetsJson["client.dll"]["dwViewMatrix"];
            spdlog::get("offsets")->debug("Loaded offset: dwViewMatrix = 0x{:X}",
                offsetsJson["client.dll"]["dwViewMatrix"].get<std::ptrdiff_t>());
        }

        // engine2.dll
        if (offsetsJson["engine2.dll"].contains("dwWindowHeight")) {
            loadedOffsets["dwWindowHeight"] = offsetsJson["engine2.dll"]["dwWindowHeight"];
            spdlog::get("offsets")->debug("Loaded offset: dwWindowHeight = 0x{:X}",
                offsetsJson["engine2.dll"]["dwWindowHeight"].get<std::ptrdiff_t>());
        }
        if (offsetsJson["engine2.dll"].contains("dwWindowWidth")) {
            loadedOffsets["dwWindowWidth"] = offsetsJson["engine2.dll"]["dwWindowWidth"];
            spdlog::get("offsets")->debug("Loaded offset: dwWindowWidth = 0x{:X}",
                offsetsJson["engine2.dll"]["dwWindowWidth"].get<std::ptrdiff_t>());
        }

    }

    // client_dll.json
    if (!clientJson.empty() && clientJson.contains("client.dll") && clientJson["client.dll"].contains("classes")) {

        // C_BaseEntity
        if (clientJson["client.dll"]["classes"].contains("C_BaseEntity") && clientJson["client.dll"]["classes"]["C_BaseEntity"].contains("fields")) {

            auto& CBaseEntity = clientJson["client.dll"]["classes"]["C_BaseEntity"]["fields"];

            if (CBaseEntity.contains("m_iHealth")) {
                loadedOffsets["m_iHealth"] = CBaseEntity["m_iHealth"];
                spdlog::get("offsets")->debug("Loaded offset: m_iHealth = 0x{:X}",
                    CBaseEntity["m_iHealth"].get<std::ptrdiff_t>());
            }
            if (CBaseEntity.contains("m_iTeamNum")) {
                loadedOffsets["m_iTeamNum"] = CBaseEntity["m_iTeamNum"];
                spdlog::get("offsets")->debug("Loaded offset: m_iTeamNum = 0x{:X}",
                    CBaseEntity["m_iTeamNum"].get<std::ptrdiff_t>());
            }
            if (CBaseEntity.contains("m_pGameSceneNode")) {
                loadedOffsets["m_pGameSceneNode"] = CBaseEntity["m_pGameSceneNode"];
                spdlog::get("offsets")->debug("Loaded offset: m_pGameSceneNode = 0x{:X}",
                    CBaseEntity["m_pGameSceneNode"].get<std::ptrdiff_t>());
            }
        }

        // CBasePlayerController
        if (clientJson["client.dll"]["classes"].contains("CBasePlayerController") && clientJson["client.dll"]["classes"]["CBasePlayerController"].contains("fields")) {

            auto& CBasePlayerController = clientJson["client.dll"]["classes"]["CBasePlayerController"]["fields"];

            if (CBasePlayerController.contains("m_hPawn")) {
                loadedOffsets["m_hPawn"] = CBasePlayerController["m_hPawn"];
                spdlog::get("offsets")->debug("Loaded offset: m_hPawn = 0x{:X}",
                    CBasePlayerController["m_hPawn"].get<std::ptrdiff_t>());
            }
            if (CBasePlayerController.contains("m_iszPlayerName")) {
                loadedOffsets["m_iszPlayerName"] = CBasePlayerController["m_iszPlayerName"];
                spdlog::get("offsets")->debug("Loaded offset: m_iszPlayerName = 0x{:X}",
                    CBasePlayerController["m_iszPlayerName"].get<std::ptrdiff_t>());
            }
        }

        //CCSPlayerController
        if (clientJson["client.dll"]["classes"].contains("CCSPlayerController") && clientJson["client.dll"]["classes"]["CCSPlayerController"].contains("fields")) {

            auto& CSPlayerController = clientJson["client.dll"]["classes"]["CCSPlayerController"]["fields"];

            if (CSPlayerController.contains("m_hPlayerPawn")) {
                loadedOffsets["m_hPlayerPawn"] = CSPlayerController["m_hPlayerPawn"];
                spdlog::get("offsets")->debug("Loaded offset: m_hPlayerPawn = 0x{:X}",
                    CSPlayerController["m_hPlayerPawn"].get<std::ptrdiff_t>());
            }

        }

        // C_BasePlayerPawn
        if (clientJson["client.dll"]["classes"].contains("C_BasePlayerPawn") && clientJson["client.dll"]["classes"]["C_BasePlayerPawn"].contains("fields")) {

            auto& CBasePlayerPawn = clientJson["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"];

            if (CBasePlayerPawn.contains("m_vOldOrigin")) {
                loadedOffsets["m_vOldOrigin"] = CBasePlayerPawn["m_vOldOrigin"];
                spdlog::get("offsets")->debug("Loaded offset: m_vOldOrigin = 0x{:X}",
                    CBasePlayerPawn["m_vOldOrigin"].get<std::ptrdiff_t>());
            }
        }

        // C_BaseModelEntity
        if (clientJson["client.dll"]["classes"].contains("C_BaseModelEntity") && clientJson["client.dll"]["classes"]["C_BaseModelEntity"].contains("fields")) {

            auto& CBaseModelEntity = clientJson["client.dll"]["classes"]["C_BaseModelEntity"]["fields"];

            if (CBaseModelEntity.contains("m_iOldHealth")) {
                loadedOffsets["m_iOldHealth"] = CBaseModelEntity["m_iOldHealth"];
                spdlog::get("offsets")->debug("Loaded offset: m_iOldHealth = 0x{:X}",
                    CBaseModelEntity["m_iOldHealth"].get<std::ptrdiff_t>());
            }

            if (CBaseModelEntity.contains("m_vecViewOffset")) {
                loadedOffsets["m_vecViewOffset"] = CBaseModelEntity["m_vecViewOffset"];
                spdlog::get("offsets")->debug("Loaded offset: m_vecViewOffset = 0x{:X}",
                    CBaseModelEntity["m_vecViewOffset"].get<std::ptrdiff_t>());
            }
        }

        // CSkeletonInstance
        if (clientJson["client.dll"]["classes"].contains("CSkeletonInstance") && clientJson["client.dll"]["classes"]["CSkeletonInstance"].contains("fields")) {

            auto& CSkeletonInstance = clientJson["client.dll"]["classes"]["CSkeletonInstance"]["fields"];

            if (CSkeletonInstance.contains("m_modelState")) {
                loadedOffsets["m_modelState"] = CSkeletonInstance["m_modelState"];
                spdlog::get("offsets")->debug("Loaded offset: m_modelState = 0x{:X}",
                    CSkeletonInstance["m_modelState"].get<std::ptrdiff_t>());
            }
        }

    }

    spdlog::get("offsets")->info("Loaded {} offsets successfully", loadedOffsets.size());
}

OffsetsLoader::~OffsetsLoader() {
    spdlog::get("offsets")->info("OffsetsLoader destroyed");
}

std::string OffsetsLoader::HttpRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            spdlog::get("offsets")->error("HTTP request failed: {}", curl_easy_strerror(res));
            response.clear();
        }

        curl_easy_cleanup(curl);
    }

    return response;
}


json OffsetsLoader::DownloadJson(const std::string& url) {
    spdlog::get("offsets")->info("Downloading: {}", url);

    std::string response = HttpRequest(url);
    if (response.empty()) {
        spdlog::get("offsets")->error("Failed to download: {}", url);
        return json();
    }

    try {
        json result = json::parse(response);
        spdlog::get("offsets")->info("Successfully parsed JSON from {}", url);
        return result;
    }
    catch (const std::exception& e) {
        spdlog::get("offsets")->error("JSON parsing error: {}", e.what());
        return json();
    }
}

std::ptrdiff_t OffsetsLoader::GetOffset(const std::string& name) {
    auto it = loadedOffsets.find(name);
    if (it != loadedOffsets.end()) {
        return it->second;
    }

    spdlog::get("offsets")->warn("Offset not found: {}", name);
    return 0;
}

bool OffsetsLoader::OffsetsAreLoaded() const {
    return !loadedOffsets.empty();
}