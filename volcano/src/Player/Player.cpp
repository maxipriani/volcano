#include "Player.h"

Player::Player(uintptr_t pawn, int team, int health,
    Vector3 worldPos, Vector3 worldHead, float distance,
    std::string name, bool isLocalPlayer)
    : pawn(pawn), team(team), health(health),
    worldPos(worldPos), worldHead(worldHead), distance(distance), name(name), isLocalPlayer(isLocalPlayer) {
}

Player Player::GetPlayerInfo(int index, const GameState& gameState, Memory& memory) {

    uintptr_t controllerListEntry = memory.Read<uintptr_t>(gameState.entityList + (8 * ((index & 0x7FFF) >> 9) + 16));
    if (!controllerListEntry) return Player();

    uintptr_t playerController = memory.Read<uintptr_t>(controllerListEntry + 0x78 * (index & 0x1FF));
    if (!playerController) return Player();

    char playerName[128] = {};
    memory.ReadArray(playerController + offsets::client_dll::m_iszPlayerName(), playerName, sizeof(playerName) / sizeof(playerName[0]));
    //memory.ReadArray(playerController + offsets::client_dll::m_iszPlayerName, playerName, sizeof(playerName) / sizeof(playerName[0]));
    std::string name = std::string(playerName);

    uintptr_t playerPawnHandle = memory.Read<uintptr_t>(playerController + offsets::client_dll::m_hPawn());
    //uintptr_t playerPawnHandle = memory.Read<uintptr_t>(playerController + offsets::client_dll::m_hPawn);
    if (!playerPawnHandle) return Player();

    uintptr_t pawnListEntry = memory.Read<uintptr_t>(gameState.entityList + (8 * ((playerPawnHandle & 0x7FFF) >> 9) + 16));
    if (!pawnListEntry) return Player();

    uintptr_t pawn = memory.Read<uintptr_t>(pawnListEntry + 0x78 * (playerPawnHandle & 0x1FF));
    if (!pawn) return Player();

    int health = memory.Read<int>(pawn + offsets::client_dll::m_iHealth());
    int team = memory.Read<uint8_t>(pawn + offsets::client_dll::m_iTeamNum());
    Vector3 worldPos = memory.Read<Vector3>(pawn + offsets::client_dll::m_vOldOrigin());

    //int health = memory.Read<int>(pawn + offsets::client_dll::m_iHealth);
    //int team = memory.Read<uint8_t>(pawn + offsets::client_dll::m_iTeamNum);
    //Vector3 worldPos = memory.Read<Vector3>(pawn + offsets::client_dll::m_vOldOrigin);

    Vector3 worldHead = worldPos;
    worldHead.z += 80.f;
	bool isLocalPlayer = (gameState.localPlayer == pawn);
    Vector3 localPlayerWorldPos = memory.Read<Vector3>(gameState.localPlayer + offsets::client_dll::m_vOldOrigin());
    /*Vector3 localPlayerWorldPos = memory.Read<Vector3>(gameState.localPlayer + offsets::client_dll::m_vOldOrigin);*/
    float distance = localPlayerWorldPos.distance(worldPos);

    return Player(pawn, team, health, worldPos, worldHead, distance, name, isLocalPlayer);
}

bool Player::ShouldSkipPlayer(Player& player, const GameState& gameState, Config* config) {
    if (player.pawn == 0) return true;
    if (player.health <= 0 || player.health > 100) return true;
    if (player.isLocalPlayer) return true;
    if (player.team == gameState.localTeam && !config->showLocalTeam) return true;
    return false;
}


