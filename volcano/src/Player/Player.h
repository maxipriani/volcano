#pragma once
#include "../Memory/Memory.h"
#include "../Game/Game.h"
#include "../Config/Config.h"
#include "../Vector/Vec3.h"

class Player {
public:
    uintptr_t pawn;
    int team;
    int health;
    Vector3 worldPos;
    Vector3 worldHead;
    float distance;
    std::string name;
	bool isLocalPlayer;

public:
    Player GetPlayerInfo(int index, const GameState& gameState, Memory& memory);
    Player() = default;
    Player(uintptr_t pawn, int team, int health, Vector3 worldPos, Vector3 worldHead, float distance, std::string name, bool isLocalPlayer);
    bool ShouldSkipPlayer(Player& player, const GameState& gameState, Config* config);
};
