#include "ESP.h"

ESP::ESP(Memory& mem, Render* render, Config* config) :
    memory(mem),
    render(render),
    config(config),
    gameState(mem),
    wts(nullptr)
{   
    resX = memory.Read<int>(gameState.engineDLL + offsets::engine2_dll::dwWindowWidth());;
    resY = memory.Read<int>(gameState.engineDLL + offsets::engine2_dll::dwWindowHeight());
    wts = new WorldToScreen(gameState.viewMatrix, resX, resY);
}

ESP::~ESP() {

    if (wts) {
        delete wts;
    }
}

int ESP::GetResX() {
    return resX;
}

int ESP::GetResY() {
    return resY;
}

void ESP::RenderESP() {
    HandleUIToggle();

    if (uiActivated) render->RenderUI();
    if (!config->enableESP) return;

    gameState.Update();

    if (wts) {
        wts->updateMatrix(gameState.viewMatrix);
    }

    for (int i = 1; i < 64; i++) {
        Player currentPlayer = player.GetPlayerInfo(i, gameState, memory);
        if (player.ShouldSkipPlayer(currentPlayer, gameState, config)) continue;
        RenderPlayerESP(currentPlayer);
    }
}

ImU32 HPBasedColor(int health) {

    if (health > 80) {
        return IM_COL32(0, 255, 0, 255);
    }
    else if (health > 50) {
        return IM_COL32(255, 255, 0, 255);
    }
    else if (health > 20) {
        return IM_COL32(255, 165, 0, 255);
    }
    else {
        return IM_COL32(255, 0, 0, 255);
    }
}

ImU32 ESP::PlayerColor(Player& player) {

    ImU32 color;
    if (config->enableHealthBased) {
        color = HPBasedColor(player.health);
    }
    else if (player.team == gameState.localTeam) {
        color = ImGui::ColorConvertFloat4ToU32(config->teamColor);
    }
    else {
        color = ImGui::ColorConvertFloat4ToU32(config->enemyColor);
    }

    return color;
}

void ESP::DrawHealthText(const ImVec2& playerScreenPos, int playerHealth, float healthTextSize) {
    if (config->showHealth) {
        std::string healthText = std::to_string(playerHealth) + "HP";
        config->healthColor = HPBasedColor(playerHealth);

        ImGui::GetBackgroundDrawList()->AddText(
            ImGui::GetFont(),
            config->healthTextSize,
            ImVec2(playerScreenPos.x - (healthTextSize * healthText.length() / 4), playerScreenPos.y),
            config->healthColor,
            healthText.c_str()
        );
    }
}

// gets bone coordinates in world space (3D)
bool ESP::GetBonePosition(uintptr_t pawn, int boneIndex, Vector3& bonePosition) {

    if (!pawn) return false;

    uintptr_t gameSceneNode = memory.Read<uintptr_t>(pawn + offsets::client_dll::m_pGameSceneNode());
    if (!gameSceneNode) return false;

    uintptr_t boneArray = memory.Read<uintptr_t>(gameSceneNode + offsets::client_dll::m_modelState() + 0x80);
    if (!boneArray) return false;

    bonePosition = memory.Read<Vector3>(boneArray + (boneIndex * 0x20));

    return true;
}

void ESP::DrawBoneESP(Player& player) {

    if (!config->enableBoneESP) return;
    if (!wts) return;

    ImU32 color = PlayerColor(player);

    // each pair represents a connection between two bones
    const std::pair<int, int> boneConnections[] = {
        {6, 5},                  // head to neck 
        {5, 4},                  // neck to spine 
        {4, 0},                  // spine to hip
        {5, 8},                  // neck to left_shoulder
        {8, 9},                 // left_shoulder to left_arm
        {9, 11},               // left_arm to left_hand
        {5, 13},               // neck to right_shoulder
        {13, 14},             // right_shoulder to right_arm
        {14, 16},             // right_arm to right_hand
        {0, 2},               // spine to spine_1
        {0, 22},              // hip to left_hip
        {22, 23},             // left_hip to left_knee
        {23, 24},            // left_knee to left_foot
        {0, 25},             // hip to right_hip
        {25, 26},            // right_hip to right_knee
        {26, 27}             // right_knee to right_foot
    };

    // draw each bone connection
    for (const std::pair<int, int>& connection : boneConnections) {

        Vector3 worldBoneStart, worldBoneEnd;
        if (GetBonePosition(player.pawn, connection.first, worldBoneStart) &&
            GetBonePosition(player.pawn, connection.second, worldBoneEnd)) {

            float maxValidBoneDistance = 25.0f;
            if (worldBoneStart.distance(worldBoneEnd) > maxValidBoneDistance) {
                continue;
            }

            // transform world bone coordinates (3D) to screen bone coordinates (2D)
            // use WorldToScreen instance
            ImVec2 screenBoneStart, screenBoneEnd;
            if (wts->transform(worldBoneStart, screenBoneStart) &&
                wts->transform(worldBoneEnd, screenBoneEnd)) {

                ImGui::GetBackgroundDrawList()->AddLine(
                    screenBoneStart,
                    screenBoneEnd,
                    color,
                    config->boneThickness
                );

                float jointRadius = config->jointRadius;
                ImGui::GetBackgroundDrawList()->AddCircleFilled(
                    screenBoneStart,
                    jointRadius,
                    color
                );

                ImGui::GetBackgroundDrawList()->AddCircleFilled(
                    screenBoneEnd,
                    jointRadius,
                    color
                );

            }
        }
    }
}

void ESP::DrawESPBox(const ImVec2& playerScreenPos, const ImVec2& playerScreenHead, ImU32 color) {
    float boxHeight = playerScreenPos.y - playerScreenHead.y;
    float boxWidth = boxHeight / 2.4f;

    if (config->enableBoxes) {
        ImGui::GetBackgroundDrawList()->AddRect(
            ImVec2(playerScreenHead.x - boxWidth / 2, playerScreenHead.y),
            ImVec2(playerScreenHead.x + boxWidth / 2, playerScreenPos.y),
            color,
            config->boxRoundness,
            0,
            config->boxThickness);
    }
}

void ESP::DrawPlayerDistance(const ImVec2& playerScreenPos, float playerDistance) {
    if (config->enableDistance) {
        std::string enemyDistanceText = std::to_string(static_cast<int>(playerDistance));

        ImGui::GetBackgroundDrawList()->AddText(
            ImGui::GetFont(),
            config->distanceTextSize,
            ImVec2(playerScreenPos.x - (config->distanceTextSize * enemyDistanceText.length() / 4),
                playerScreenPos.y + config->distanceTextSize),
            config->distanceTextColor,
            enemyDistanceText.c_str()
        );
    }
}

void ESP::DrawPlayerNames(ImVec2 playerScreenHead, int playerHealth, const char playerName[], float playerNamesTextSize) {
    if (config->showPlayerNames) {
        if (config->enableHealthBased) {
            config->playerNamesTextColor = HPBasedColor(playerHealth);
        }

        ImGui::GetBackgroundDrawList()->AddText(
            ImGui::GetFont(),
            playerNamesTextSize,
            ImVec2(playerScreenHead.x - (playerNamesTextSize * strlen(playerName) / 4),
                playerScreenHead.y - playerNamesTextSize),
            config->playerNamesTextColor,
            playerName
        );
    }
}

void ESP::RenderPlayerESP(Player& player) {

    if (!wts) return;

    ImVec2 playerScreenPos, playerScreenHead;

    bool playerVisible = wts->transform(player.worldPos, playerScreenPos) && wts->transform(player.worldHead, playerScreenHead);

    if (playerVisible) {

        ImU32 color = PlayerColor(player);
        DrawESPBox(playerScreenPos, playerScreenHead, color);
        DrawHealthText(playerScreenPos, player.health, config->healthTextSize);
        DrawPlayerDistance(playerScreenPos, player.distance);
        DrawPlayerNames(playerScreenHead, player.health, player.name.c_str(), config->playerNamesTextSize);
        DrawBoneESP(player);

    }
}

void ESP::HandleUIToggle() {
    static bool previousState = false;

    bool currentState = (GetAsyncKeyState(VK_F2) & 1);

    if (currentState && !previousState) {
        config->enableESP = !config->enableESP;
    }

    previousState = currentState;

    if (GetAsyncKeyState(VK_F1) & 1) {
        uiActivated = !uiActivated;
        LONG windowStyle = GetWindowLong(render->GetOverlayWindow(), GWL_EXSTYLE);
        if (uiActivated) {
            windowStyle &= ~WS_EX_TRANSPARENT;
        }
        else {
            windowStyle |= WS_EX_TRANSPARENT;
        }
        SetWindowLong(render->GetOverlayWindow(), GWL_EXSTYLE, windowStyle);
    }
}