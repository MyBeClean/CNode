
#pragma once

#include <algorithm>
#include <cmath>
#include "imgui.h"

namespace CNode::SceneEditor
{
    struct Vec3
    {
        float x, y, z;
    };

    inline constexpr float PI = 3.14159265358979323846f;

    inline Vec3 Add(Vec3 a, Vec3 b)
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    inline Vec3 Subtract(Vec3 a, Vec3 b)
    {
        return { a.x - b.x, a.y - b.y, a.z - b.z };
    }

    inline Vec3 Multiply(Vec3 v, float s)
    {
        return { v.x * s, v.y * s, v.z * s };
    }

    inline float Dot(Vec3 a, Vec3 b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline Vec3 Cross(Vec3 a, Vec3 b)
    {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    inline Vec3 Normalize(Vec3 v)
    {
        const float length = std::sqrt(Dot(v, v));

        if (length < 0.0001f)
            return { 0.0f, 0.0f, 0.0f };

        return Multiply(v, 1.0f / length);
    }

    struct CameraBasis
    {
        Vec3 position;
        Vec3 forward;
        Vec3 right;
        Vec3 up;
    };

    struct Camera
    {
        float yaw = 0.75f;
        float pitch = 0.45f;
        float distance = 12.0f;

        Vec3 pivot{ 0.0f, 0.0f, 0.0f };
        Vec3 fpsPosition{ 0.0f, 0.0f, 0.0f };

        float orbitYaw = 0.75f;
        float orbitPitch = 0.45f;

        bool fpsMode = false;
        bool navigating = false;
        bool hasLastMouse = false;

        ImVec2 lastMouse{ 0.0f, 0.0f };

        Vec3 Forward() const
        {
            const float cp = std::cos(pitch);

            return Normalize({
                -cp * std::sin(yaw),
                -std::sin(pitch),
                -cp * std::cos(yaw)
                });
        }

        Vec3 Position() const
        {
            if (fpsMode)
                return fpsPosition;

            const float cp = std::cos(pitch);

            return Add(pivot, {
                distance * cp * std::sin(yaw),
                distance * std::sin(pitch),
                distance * cp * std::cos(yaw)
                });
        }

        CameraBasis Basis() const
        {
            CameraBasis c{};

            c.position = Position();

            c.forward = fpsMode
                ? Forward()
                : Normalize(Subtract(pivot, c.position));

            const Vec3 worldUp{ 0.0f, 1.0f, 0.0f };

            c.right = Normalize(Cross(c.forward, worldUp));

            if (Dot(c.right, c.right) < 0.0001f)
                c.right = { 1.0f, 0.0f, 0.0f };

            c.up = Normalize(Cross(c.right, c.forward));

            return c;
        }

        void Orbit(float dx, float dy)
        {
            if (fpsMode)
                return;

            yaw -= dx * 0.01f;

            pitch = std::clamp(
                pitch + dy * 0.01f,
                -1.55f,
                1.55f
            );
        }

        void Pan(float dx, float dy)
        {
            if (fpsMode)
                return;

            const CameraBasis c = Basis();
            const float scale = distance * 0.002f;

            const Vec3 movement = Multiply(
                Add(
                    Multiply(c.right, -dx),
                    Multiply(c.up, dy)
                ),
                scale
            );

            pivot = Add(pivot, movement);
        }

        void Zoom(float amount)
        {
            if (fpsMode)
                return;

            distance = std::clamp(
                distance - amount,
                1.0f,
                100.0f
            );
        }

        void BeginFPS()
        {
            if (fpsMode)
                return;

            orbitYaw = yaw;
            orbitPitch = pitch;

            fpsPosition = Position();
            fpsMode = true;
        }

        void EndFPS()
        {
            if (!fpsMode)
                return;

            const Vec3 forward = Forward();

            pivot = Add(
                fpsPosition,
                Multiply(forward, distance)
            );

            fpsMode = false;
        }

        void LookFromDirection(Vec3 direction)
        {
            direction = Normalize(direction);

            if (Dot(direction, direction) < 0.5f)
                return;

            yaw = std::atan2(direction.x, direction.z);

            pitch = std::asin(
                std::clamp(direction.y, -1.0f, 1.0f)
            );

            pitch = std::clamp(pitch, -1.55f, 1.55f);
        }

        void RotateFPS(float dx, float dy)
        {
            if (!fpsMode)
                return;

            constexpr float sensitivity = 0.003f;

            yaw -= dx * sensitivity;
            pitch += dy * sensitivity;

            pitch = std::clamp(
                pitch,
                -1.55f,
                1.55f
            );
        }

        void MoveFPS(
            float forwardAmount,
            float rightAmount,
            float upAmount,
            float dt)
        {
            if (!fpsMode || dt <= 0.0f)
                return;

            const CameraBasis c = Basis();

            const bool fast =
                ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
                ImGui::IsKeyDown(ImGuiKey_RightShift);

            const float speed = (fast ? 18.0f : 5.0f) * dt;

            Vec3 movement = Add(
                Add(
                    Multiply(c.forward, forwardAmount),
                    Multiply(c.right, rightAmount)
                ),
                Multiply(c.up, upAmount)
            );

            const float movementLength =
                std::sqrt(Dot(movement, movement));

            if (movementLength > 1.0f)
            {
                movement = Multiply(
                    movement,
                    1.0f / movementLength
                );
            }

            fpsPosition = Add(
                fpsPosition,
                Multiply(movement, speed)
            );
        }
    };
}
