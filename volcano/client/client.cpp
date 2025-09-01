#include "client.h"

static std::string g_apiKey = ""; 

std::string GetUserWindowsName() {
    char username[255] = { 0 };
    DWORD username_len = 255;

    if (GetUserNameA(username, &username_len)) {
        return std::string(username);
    }
    return "";
}

void InitializeCurl() {
    curl_global_init(CURL_GLOBAL_ALL);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool ServerIsOn() {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string url = "https://example.com/health";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);         
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);    
    CURLcode res = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_easy_cleanup(curl);

    return (res == CURLE_OK) && (httpCode == HTTP_STATUS_OK);
}

bool SaveApiKeyToFile(const std::string& apiKey) {
    std::string filename = "apiKey.txt";
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << "API Key: " << apiKey << std::endl;
    file.close();
    return true;
}

bool SendLoginRequest(bool* privileged) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string username = GetUserWindowsName();
    std::string postData = "user_name=" + username + "&token=example";

    std::string url = "https://example.com/auth/login";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(curl, CURLOPT_POST, 1L); 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str()); 

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response); 

    CURLcode res = curl_easy_perform(curl); 

    curl_easy_cleanup(curl); 

    if (res != CURLE_OK) return false;

    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        bool success = jsonResponse.value("success", false);
        std::string status = jsonResponse.value("status", "");
        std::string apiKey = jsonResponse.value("api_key", "");

        if (success) {
            if (status == "privileged_user") {
                *privileged = true;
                MessageBoxW(nullptr, L"• Privileged user detected\n\n• Login not required, congrats", L"== Privileged User ==", MB_ICONINFORMATION | MB_OK);
                return true;
            }
            else if (status == "api_key_generated") {
                g_apiKey = apiKey;
                SaveApiKeyToFile(apiKey);
                MessageBoxW(nullptr, L"• API key generated in the current directory as 'apiKey.txt'", L"== API Key Created ==", MB_ICONINFORMATION | MB_OK);
                return true;
            }
            else if (status == "authorized") {
                g_apiKey = apiKey;
                return true;
            }
        }
        else {
            if (status == "pending_authorization") {
                MessageBoxW(nullptr, L"• Registered in database\n\n• Pending authorization", L"== Awaiting Authorization ==", MB_ICONINFORMATION | MB_OK);
                return false;
            }
            else if (status == "unauthorized_user") {
                MessageBoxW(nullptr, L"• Unauthorized user, cannot use volcanoESP\n\n• Contact flosur for authorization", L"== Access Denied ==", MB_ICONERROR | MB_OK);
                return false;
            }
            else if (status == "error") {
                MessageBoxW(nullptr, L"• Server error", L"== Error ==", MB_ICONERROR | MB_OK);
                return false;
            }
        }
    }
    catch (const nlohmann::json::exception& e) {
        std::string errorMsg = "• Failed parsing JSON: " + std::string(e.what()) + "\n• Response: " + response;
        std::wstring wErrorMsg(errorMsg.begin(), errorMsg.end());
        MessageBoxW(nullptr, wErrorMsg.c_str(), L"== JSON error ==", MB_ICONERROR | MB_OK);
        return false;
    }

    return false;
}

bool SendLogOutRequest() {

    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string username = GetUserWindowsName();
    std::string postData = "user_name=" + username + "&token=volcanoESP";

    std::string url = "https://example.com/auth/logout";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST, 1L); 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str()); 

    CURLcode res = curl_easy_perform(curl); 
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

bool SendVerifyApiKeyRequest() {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string username = GetUserWindowsName();
    std::string postData = "user_name=" + username + "&api_key=" + g_apiKey + "&token=example";

    std::string url = "https://example.com/auth/verify";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str()); 

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response); 

    CURLcode res = curl_easy_perform(curl); 

    curl_easy_cleanup(curl); 

    if (res != CURLE_OK) return false;

    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        bool success = jsonResponse.value("success", false);
        std::string status = jsonResponse.value("status", "");

        if (success && status == "valid_api_key") {
            return true;
        }
        else if (!success && status == "invalid_api_key") {
            MessageBoxW(nullptr, L"• Invalid API key, try again", L"== Invalid API Key ==", MB_ICONERROR | MB_OK);
        }
    }
    catch (const nlohmann::json::exception& e) {
        std::string errorMsg = "• Failed parsing JSON: " + std::string(e.what()) + "\n• Response: " + response;
        std::wstring wErrorMsg(errorMsg.begin(), errorMsg.end());
        MessageBoxW(nullptr, wErrorMsg.c_str(), L"== JSON error ==", MB_ICONERROR | MB_OK);
    }

    return false;
}

bool LoginUser(bool* privileged) {

    if (!SendLoginRequest(privileged)) {
        return false;
    }

    return true;
}

bool GetApiKey() {
    if (!PromptForApiKey(g_apiKey)) {
        return false;
    }
    return true;
}

bool PromptForApiKey(std::string& inputApiKey) {
    char buffer[256] = { 0 };
    HWND hwnd = NULL;

    INT_PTR result = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_APIKEY), hwnd, ApiKeyDialogProc, (LPARAM)buffer);

    if (result == IDCANCEL) {
        return false;
    }

    if (result == IDOK) {
        inputApiKey = std::string(buffer);
        g_apiKey = inputApiKey;
        return true;
    }

    return false;
}

INT_PTR CALLBACK ApiKeyDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static char* pBuffer = NULL;

    switch (message) {
    case WM_INITDIALOG: {
        pBuffer = (char*)lParam;
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            GetDlgItemTextA(hwnd, IDC_APIKEY_EDIT, pBuffer, 256);
            EndDialog(hwnd, IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }

    return FALSE;
}