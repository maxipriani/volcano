#pragma once
#include <imgui.h>

struct Config {
    bool enableHealthBased = true;
    bool enableESP = true;
    bool enableBoxes = false;
    bool enableBoneESP = true;
    bool enableDistance = true;
    bool showLocalTeam = false;
    bool showHealth = true;
    bool showPlayerNames = true;
    float maxEnemyDistance = 6000.0f;
    float jointRadius = 2.0f;
    float boxThickness = 3.0f;
    float boneThickness = 3.0f;
    float boxRoundness = 0.0f;
    float healthTextSize = 15.0f;
    float distanceTextSize = 15.0f;
    float playerNamesTextSize = 20.0f;
    ImColor healthColor;
    ImColor enemyColor = ImColor(255, 0, 0, 255);
    ImColor teamColor = ImColor(0, 0, 255, 255);
    ImColor distanceTextColor = ImColor(255, 255, 255, 255);
    ImColor playerNamesTextColor = ImColor(255, 0, 0, 255);
};