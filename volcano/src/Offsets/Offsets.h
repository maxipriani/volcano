#pragma once
#include <cstddef>
#include "OffsetsLoader.h"

extern OffsetsLoader* g_pOffsetsLoader;

namespace offsets {

    namespace offsets {

        inline std::ptrdiff_t dwEntityList() {
            return g_pOffsetsLoader->GetOffset("dwEntityList");
        }

        inline std::ptrdiff_t dwLocalPlayerPawn() {
            return g_pOffsetsLoader->GetOffset("dwLocalPlayerPawn");
        }

        inline std::ptrdiff_t dwViewAngles() {
            return g_pOffsetsLoader->GetOffset("dwViewAngles");
        }

        inline std::ptrdiff_t dwViewMatrix() {
            return g_pOffsetsLoader->GetOffset("dwViewMatrix");
        }
    }

    namespace client_dll {
        inline std::ptrdiff_t m_iHealth() {
            return g_pOffsetsLoader->GetOffset("m_iHealth");
        }

        inline std::ptrdiff_t m_iTeamNum() {
            return g_pOffsetsLoader->GetOffset("m_iTeamNum");
        }

        inline std::ptrdiff_t m_hPawn() {
            return g_pOffsetsLoader->GetOffset("m_hPawn");
        }

        inline std::ptrdiff_t m_hPlayerPawn() {
            return g_pOffsetsLoader->GetOffset("m_hPlayerPawn");
        }

        inline std::ptrdiff_t m_vOldOrigin() {
            return g_pOffsetsLoader->GetOffset("m_vOldOrigin");
        }

        inline std::ptrdiff_t m_iOldHealth() {
            return g_pOffsetsLoader->GetOffset("m_iOldHealth");
        }

        inline std::ptrdiff_t m_iszPlayerName() {
            return g_pOffsetsLoader->GetOffset("m_iszPlayerName");
        }

        inline std::ptrdiff_t m_vecViewOffset() {
            return g_pOffsetsLoader->GetOffset("m_vecViewOffset");
        }

        inline std::ptrdiff_t m_pGameSceneNode() {
            return g_pOffsetsLoader->GetOffset("m_pGameSceneNode");
        }

        inline std::ptrdiff_t m_modelState() {
            return g_pOffsetsLoader->GetOffset("m_modelState");
        }

    }

    namespace engine2_dll {
        inline std::ptrdiff_t dwWindowHeight() {
            return g_pOffsetsLoader->GetOffset("dwWindowHeight");
        }

        inline std::ptrdiff_t dwWindowWidth() {
            return g_pOffsetsLoader->GetOffset("dwWindowWidth");
        }
    }
}