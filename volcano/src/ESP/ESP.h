#pragma once
#include "../Vector/Vec3.h"
#include "../Offsets/Offsets.h"
//#include "../Offsets/Offsets_.h"
#include "../Memory/Memory.h"
#include "../Render/Render.h"
#include "../Config/Config.h"
#include "../../utils/imguiDx.h"
#include "../Player/Player.h"
#include "../Game/Game.h"
#include <imgui.h>
#include <string>

class ESP {
private:
    Memory& memory;
    Render* render;
    Config* config;
    GameState gameState;
    Player player;
    bool uiActivated = true;
    WorldToScreen* wts;
    void HandleUIToggle();
    ImU32 PlayerColor(Player& player);
    bool GetBonePosition(uintptr_t entityPawn, int boneIndex, Vector3& bonePosition);
    void DrawBoneESP(Player& player);

public:
    void RenderPlayerESP(Player& player);
    int resX;
    int resY;

    ESP(Memory& mem, Render* render, Config* config);
    ~ESP();
    int GetResX();
    int GetResY();
    void RenderESP();
    void DrawESPBox(const ImVec2& playerScreenPos, const ImVec2& playerScreenHead, ImU32 color);
    void DrawHealthText(const ImVec2& playerScreenPos, int health, float healthTextSize);
    void DrawPlayerDistance(const ImVec2& playerScreenPos, float playerDistance);
    void DrawPlayerNames(ImVec2 playerScreenHead, int playerHealth, const char playerName[], float playerNamesTextSize);
};