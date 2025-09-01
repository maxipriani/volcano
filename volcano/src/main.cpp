#include <chrono>
#include <thread>
#include "ESP/ESP.h"
#include "Memory/Memory.h"
#include "Render/Render.h"
#include "Config/Config.h"
#include "../client/client.h"
#include "Offsets/OffsetsLoader.h"

OffsetsLoader* g_pOffsetsLoader = nullptr;
INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
    spdlog::set_level(spdlog::level::debug);
    InitializeCurl();

    if (!ServerIsOn()) {
        MessageBoxW(nullptr, L"• Could not connect to the server", L"== Server OFF ==", MB_ICONERROR | MB_OK);
        return 1;
    }

    g_pOffsetsLoader = new OffsetsLoader();

    Memory memory(L"cs2.exe");
    if (!memory.IsProcessFound()) {
        MessageBoxW(nullptr, L"• cs2.exe is not running", L"== Error ==", MB_ICONERROR | MB_OK);
        return 1;
    }

    bool privileged = false;
    if (!LoginUser(&privileged)) {
        return 1;
    }

    if (!privileged) {
        bool apiKeyValid = false;
        while (!apiKeyValid) {
            if (!GetApiKey()) return 1;
            if (SendVerifyApiKeyRequest()) apiKeyValid = true;
        }
    }

    Config config;
    Render render(config);
    ESP esp(memory, &render, &config);

    render.InitializeUI(instance, esp);
    ShowWindow(render.GetOverlayWindow(), cmd_show);
    UpdateWindow(render.GetOverlayWindow());

    bool running = true;
    int FPS = 144;
    std::chrono::milliseconds frameDuration(1000 / FPS);

    while (running) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }

            if (!memory.IsProcessFound()) {
                running = false;
                break;
            }

        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        esp.RenderESP();
        ImGui::Render();
        render.Present();

        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsedTime = frameEnd - frameStart;

        if (elapsedTime < frameDuration) {
            std::this_thread::sleep_for(frameDuration - elapsedTime);
        }

    }

    SendLogOutRequest();
    delete g_pOffsetsLoader;
    g_pOffsetsLoader = nullptr;
    curl_global_cleanup();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    return 0;
}