#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#pragma comment(lib, "wininet.lib")

// http --> string
std::string FetchHttpContent(const std::wstring& url) {
    HINTERNET hInternet = InternetOpen(L"HTTP Example", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        std::cerr << "[ERROR] InternetOpen failed" << std::endl;
        return "";
    }

    HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (!hConnect) {
        std::cerr << "[ERROR] InternetOpenUrl failed" << std::endl;
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[4096];
    DWORD bytesRead;
    std::string response;

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        response.append(buffer, bytesRead);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return response;
}


std::string ExtractVersion(const std::string& json) {
    std::string versionKey = "\"clientVersionUpload\":\"version-";
    size_t startPos = json.find(versionKey);
    if (startPos == std::string::npos) return "";

    startPos += versionKey.length();
    size_t endPos = json.find('"', startPos);
    if (endPos == std::string::npos) return "";

    return json.substr(startPos, endPos - startPos);
}


bool DownloadFile(const std::wstring& url, const std::wstring& fileName) {
    std::wcout << L"[INFO] Downloading from URL: " << url << std::endl;

    HINTERNET hInternet = InternetOpen(L"HTTP Example", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        std::cerr << "[ERROR] InternetOpen failed" << std::endl;
        return false;
    }

    HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (!hConnect) {
        std::cerr << "[ERROR] InternetOpenUrl failed" << std::endl;
        InternetCloseHandle(hInternet);
        return false;
    }

    HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERROR] CreateFile failed" << std::endl;
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        DWORD bytesWritten;
        if (!WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL) || bytesWritten != bytesRead) {
            std::cerr << "[ERROR] WriteFile failed" << std::endl;
            CloseHandle(hFile);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
    }

    CloseHandle(hFile);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return true;
}

void ShowLoadingAnimation(const std::string& message) {
    const std::string spinner[] = { "|", "/", "-", "\\" };
    for (int i = 0; i < 20; ++i) {
        std::cout << "\r" << message << " " << spinner[i % 4] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << std::endl;
}


void DisplayStatus(const std::string& message) {
    std::cout << "[STATUS] " << message << std::endl;
}

int main() {
    std::cout << "Welcome to the Roblox Updater!" << std::endl;
    std::cout << "--------------------------------" << std::endl;

    DisplayStatus("Checking for downgrade version...");


    std::wstring jsonUrl = L"https://pastebin.com/raw/jqBdZ1cG";
    std::string jsonResponse = FetchHttpContent(jsonUrl);
    if (jsonResponse.empty()) {
        std::cerr << "[ERROR] Failed to fetch JSON response" << std::endl;
        return 1;
    }


    std::cout << "[INFO] Received JSON response:\n" << jsonResponse << std::endl;

    std::string version = ExtractVersion(jsonResponse);
    if (version.empty()) {
        std::cerr << "[ERROR] Failed to extract version" << std::endl;
        return 1;
    }
    std::cout << "[INFO] Extracted version: " << version << std::endl;


    std::string downloadUrlStr = "https://roblox-setup.cachefly.net/version-" + version + "-RobloxApp.zip";
    std::wstring downloadUrl(downloadUrlStr.begin(), downloadUrlStr.end());

    std::wcout << L"[INFO] Constructed download URL: " << downloadUrl << std::endl;


    std::cout << "Starting download..." << std::endl;
    ShowLoadingAnimation("Downloading");


    std::wstring fileName = L"RobloxApp.zip";
    if (DownloadFile(downloadUrl, fileName)) {
        std::cout << "[SUCCESS] File downloaded successfully as " << std::string(fileName.begin(), fileName.end()) << std::endl;
        std::cout << "[INFO] Extract the WinRAR and Place the RobloxPlayerBeta.exe and DLL in the recent version roblox files" << std::endl;
        system("pause");
    }
    else {
        std::cerr << "[ERROR] Failed to download file" << std::endl;
        return 1;
    }

    return 0;
}
