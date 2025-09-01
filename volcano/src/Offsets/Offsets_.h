#pragma once
#include <cstddef>

namespace offsets {

	namespace offsets {
		constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1AF4B00;
		constexpr std::ptrdiff_t dwEntityList = 0x1CBE5B0;
		constexpr std::ptrdiff_t dwViewAngles = 0x1D2C740;
		constexpr std::ptrdiff_t dwViewMatrix = 0x1D21980;
	}

	namespace client_dll {
		constexpr std::ptrdiff_t m_iHealth = 0x34C; // int32  
		constexpr std::ptrdiff_t m_iTeamNum = 0x3E3; // uint8  
		constexpr std::ptrdiff_t m_hPawn = 0x6B4;// CHandle<C_BasePlayerPawn> 
		constexpr std::ptrdiff_t m_hPlayerPawn = 0x8FC; // CHandle<C_CSPlayerPawn>
		constexpr std::ptrdiff_t m_vOldOrigin = 0x15B0; // vector 
		constexpr std::ptrdiff_t m_iOldHealth = 0xB5C; // int32 
		constexpr std::ptrdiff_t m_iszPlayerName = 0x6E8; // char[128]
		constexpr std::ptrdiff_t m_vecViewOffset = 0xD98; // CNetworkViewOffsetVector
		constexpr std::ptrdiff_t m_pGameSceneNode = 0x330; // CGameSceneNode*
		constexpr std::ptrdiff_t m_modelState = 0x170; // CModelState
	}

	namespace engine2_dll {
		constexpr std::ptrdiff_t dwWindowHeight = 0x8AB4DC;
		constexpr std::ptrdiff_t dwWindowWidth = 0x8AB4D8;
	}

}