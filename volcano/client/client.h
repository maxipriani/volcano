#pragma once
#ifdef _MSC_VER
    #pragma comment(lib, "Advapi32.lib")
    #pragma comment(lib, "wbemuuid.lib")
#endif
#include <Windows.h>
#include <comdef.h>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <wbemidl.h>
#include "json.hpp" 
#include "../../volcano/utils/Resource.h"

#define HTTP_STATUS_OK 200
#define HTTP_STATUS_UNAUTHORIZED 401
#define HTTP_STATUS_CONFLICT 409

std::string GetUserWindowsName();
void InitializeCurl();
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
bool ServerIsOn();
bool SaveApiKeyToFile(const std::string &apiKey);

bool LoginUser(bool* privileged);
bool SendVerifyApiKeyRequest();
bool SendLogOutRequest();

bool GetApiKey();
bool PromptForApiKey(std::string &apiKey);
INT_PTR CALLBACK ApiKeyDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);