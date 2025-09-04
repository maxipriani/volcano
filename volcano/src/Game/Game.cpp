#include "Game.h"

ViewMatrix GameState::GetViewMatrix() {
    return memory.Read<ViewMatrix>(clientDLL + offsets::offsets::dwViewMatrix());
}

uintptr_t GameState::GetLocalPlayer() {
    return memory.Read<uintptr_t>(clientDLL + offsets::offsets::dwLocalPlayerPawn());
}

uintptr_t GameState::GetEntityList() {
   return memory.Read<uintptr_t>(clientDLL + offsets::offsets::dwEntityList());
}

int GameState::GetLocalTeam() {
    return memory.Read<int>(localPlayer + offsets::client_dll::m_iTeamNum());
}

void GameState::Update() {
    viewMatrix = GetViewMatrix();
    localPlayer = GetLocalPlayer();
    entityList = GetEntityList();
    localTeam = GetLocalTeam();
}