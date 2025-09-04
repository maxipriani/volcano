#pragma once
#include "../Memory/Memory.h"
#include "../Offsets/Offsets.h"
#include "../Vector/Vec3.h"

class GameState {
public:
	uintptr_t clientDLL = 0;
	uintptr_t engineDLL = 0;
	Memory& memory;
	ViewMatrix viewMatrix;
	uintptr_t localPlayer;
	uintptr_t entityList;
	int localTeam;

public:
	GameState(Memory& mem) : memory(mem) {
		clientDLL = memory.GetModuleAddress(L"client.dll");
		engineDLL = memory.GetModuleAddress(L"engine2.dll");
	}

	ViewMatrix GetViewMatrix();	
	uintptr_t GetLocalPlayer();
	uintptr_t GetEntityList();
	int GetLocalTeam();
	void Update();
};

