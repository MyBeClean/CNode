
#pragma once

#include "scene_projection.h"

#include <algorithm>
#include <cmath>

namespace CNode::SceneEditor
{
    inline void DrawGrid(
        ImDrawList* drawList,
        ImVec2 center,
        float width,
        float height,
        bool showAxes,
        const Camera& camera)
    {
        if (!drawList || width <= 1.0f || height <= 1.0f)
            return;

        const CameraBasis basis = camera.Basis();

        // Jarak antargaris selalu tetap dalam world space.
        // Kamera tidak mengubah koordinat dasar grid.
        constexpr float spacing = 1.0f;
        constexpr float gridY = 0.0f;

        // Luas grid yang digambar di sekitar posisi kamera.
        // Ini hanya menentukan area gambar, bukan ukuran grid.
        constexpr float range = 100.0f;
        constexpr int maxLines = 201;

        // Batas grid selalu disejajarkan ke kelipatan spacing.
        // Dengan demikian, garis tetap berada pada koordinat dunia
        // yang konsisten ketika kamera bergerak.
        const float startX =
            std::floor((basis.position.x - range) / spacing) * spacing;

        const float endX =
            std::ceil((basis.position.x + range) / spacing) * spacing;

        const float startZ =
            std::floor((basis.position.z - range) / spacing) * spacing;

        const float endZ =
            std::ceil((basis.position.z + range) / spacing) * spacing;

        const int countX = std::clamp(
            static_cast<int>(std::round((endX - startX) / spacing)),
            1,
            maxLines - 1
        );

        const int countZ = std::clamp(
            static_cast<int>(std::round((endZ - startZ) / spacing)),
            1,
            maxLines - 1
        );

        const float actualEndX = startX + countX * spacing;
        const float actualEndZ = startZ + countZ * spacing;

        // Garis utama setiap 5 unit.
        // Indeks integer lebih stabil daripada membandingkan
        // angka floating-point dengan remainder.
        for (int i = 0; i <= countX; ++i)
        {
            const float x = startX + i * spacing;
            const long long gridIndex =
                static_cast<long long>(std::llround(x / spacing));

            const bool major = (gridIndex % 5 == 0);

            const ImU32 color = major
                ? IM_COL32(110, 120, 138, 145)
                : IM_COL32(90, 98, 112, 85);

            DrawLine3D(
                drawList,
                { x, gridY, startZ },
                { x, gridY, actualEndZ },
                center,
                width,
                height,
                color,
                camera,
                1.0f
            );
        }

        for (int i = 0; i <= countZ; ++i)
        {
            const float z = startZ + i * spacing;
            const long long gridIndex =
                static_cast<long long>(std::llround(z / spacing));

            const bool major = (gridIndex % 5 == 0);

            const ImU32 color = major
                ? IM_COL32(110, 120, 138, 145)
                : IM_COL32(90, 98, 112, 85);

            DrawLine3D(
                drawList,
                { startX, gridY, z },
                { actualEndX, gridY, z },
                center,
                width,
                height,
                color,
                camera,
                1.0f
            );
        }

        // Sumbu dunia X, Y, dan Z.
        if (showAxes)
        {
            constexpr float axisRange = 10000.0f;

            // X: merah
            DrawLine3D(
                drawList,
                { -axisRange, 0.0f, 0.0f },
                { axisRange, 0.0f, 0.0f },
                center,
                width,
                height,
                IM_COL32(245, 75, 75, 255),
                camera,
                2.0f
            );

            // Z: biru
            DrawLine3D(
                drawList,
                { 0.0f, 0.0f, -axisRange },
                { 0.0f, 0.0f,  axisRange },
                center,
                width,
                height,
                IM_COL32(80, 165, 255, 255),
                camera,
                2.0f
            );

            // Y: hijau
            DrawLine3D(
                drawList,
                { 0.0f, -axisRange, 0.0f },
                { 0.0f,  axisRange, 0.0f },
                center,
                width,
                height,
                IM_COL32(80, 225, 120, 255),
                camera,
                2.0f
            );
        }
    }
}
