
#pragma once

#include <algorithm>
#include <cmath>

#include "scene_camera.h"

namespace CNode::SceneEditor
{
    inline bool ProjectPoint(
        Vec3 point,
        ImVec2 center,
        float width,
        float height,
        const Camera& camera,
        ImVec2& screenPoint)
    {
        if (width <= 1.0f || height <= 1.0f)
            return false;

        const CameraBasis basis = camera.Basis();

        const Vec3 relative = Subtract(
            point,
            basis.position
        );

        const float depth = Dot(
            relative,
            basis.forward
        );

        constexpr float nearPlane = 0.1f;

        if (!std::isfinite(depth) || depth < nearPlane)
            return false;

        const float focal =
            std::min(width, height) * 0.9f;

        const float cameraX = Dot(
            relative,
            basis.right
        );

        const float cameraY = Dot(
            relative,
            basis.up
        );

        screenPoint = ImVec2(
            center.x + cameraX * focal / depth,
            center.y - cameraY * focal / depth
        );

        return std::isfinite(screenPoint.x) &&
            std::isfinite(screenPoint.y);
    }

    inline bool ClipLineToViewport(
        ImVec2& a,
        ImVec2& b,
        ImVec2 center,
        float width,
        float height)
    {
        const float left = center.x - width * 0.5f;
        const float right = center.x + width * 0.5f;
        const float top = center.y - height * 0.5f;
        const float bottom = center.y + height * 0.5f;

        const float dx = b.x - a.x;
        const float dy = b.y - a.y;

        float tMin = 0.0f;
        float tMax = 1.0f;

        auto ClipTest = [&](float p, float q) -> bool
            {
                if (std::abs(p) < 1e-6f)
                    return q >= 0.0f;

                const float r = q / p;

                if (p < 0.0f)
                {
                    if (r > tMax)
                        return false;

                    tMin = std::max(tMin, r);
                }
                else
                {
                    if (r < tMin)
                        return false;

                    tMax = std::min(tMax, r);
                }

                return true;
            };

        if (!ClipTest(-dx, a.x - left) ||
            !ClipTest(dx, right - a.x) ||
            !ClipTest(-dy, a.y - top) ||
            !ClipTest(dy, bottom - a.y) ||
            tMin > tMax)
        {
            return false;
        }

        const ImVec2 originalA = a;

        a = ImVec2(
            originalA.x + tMin * dx,
            originalA.y + tMin * dy
        );

        b = ImVec2(
            originalA.x + tMax * dx,
            originalA.y + tMax * dy
        );

        return true;
    }

    inline void DrawLine3D(
        ImDrawList* drawList,
        Vec3 a,
        Vec3 b,
        ImVec2 center,
        float width,
        float height,
        ImU32 color,
        const Camera& camera,
        float thickness = 1.0f)
    {
        if (!drawList ||
            width <= 1.0f ||
            height <= 1.0f)
        {
            return;
        }

        const CameraBasis basis = camera.Basis();

        constexpr float nearPlane = 0.1f;

        const float focal =
            std::min(width, height) * 0.9f;

        // Transform world coordinates into camera space.
        auto ToCameraSpace = [&](Vec3 point) -> Vec3
            {
                const Vec3 relative = Subtract(
                    point,
                    basis.position
                );

                return {
                    Dot(relative, basis.right),
                    Dot(relative, basis.up),
                    Dot(relative, basis.forward)
                };
            };

        Vec3 p1 = ToCameraSpace(a);
        Vec3 p2 = ToCameraSpace(b);

        auto IsFinite = [](const Vec3& p) -> bool
            {
                return std::isfinite(p.x) &&
                    std::isfinite(p.y) &&
                    std::isfinite(p.z);
            };

        if (!IsFinite(p1) || !IsFinite(p2))
            return;

        // Reject lines entirely behind the camera.
        if (p1.z < nearPlane && p2.z < nearPlane)
            return;

        // Clip lines crossing the near plane.
        if (p1.z < nearPlane || p2.z < nearPlane)
        {
            const bool firstBehind =
                p1.z < nearPlane;

            Vec3& behind = firstBehind ? p1 : p2;
            const Vec3& front = firstBehind ? p2 : p1;

            const float denominator =
                front.z - behind.z;

            if (std::abs(denominator) < 1e-6f)
                return;

            const float t =
                (nearPlane - behind.z) / denominator;

            behind.x += (front.x - behind.x) * t;
            behind.y += (front.y - behind.y) * t;
            behind.z = nearPlane;
        }

        // Perspective projection uses camera-relative
        // coordinates. The viewport center is only the
        // projection origin, not the world-space origin.
        auto ToScreen = [&](const Vec3& p) -> ImVec2
            {
                return ImVec2(
                    center.x + focal * p.x / p.z,
                    center.y - focal * p.y / p.z
                );
            };

        ImVec2 screenA = ToScreen(p1);
        ImVec2 screenB = ToScreen(p2);

        if (!std::isfinite(screenA.x) ||
            !std::isfinite(screenA.y) ||
            !std::isfinite(screenB.x) ||
            !std::isfinite(screenB.y))
        {
            return;
        }

        // Keep screen coordinates within the viewport.
        if (!ClipLineToViewport(
            screenA,
            screenB,
            center,
            width,
            height))
        {
            return;
        }

        drawList->AddLine(
            screenA,
            screenB,
            color,
            thickness
        );
    }
}
