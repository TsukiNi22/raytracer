/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file Utils.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "raytracer/special/Utils.hpp"
#include <cstdint>
#include <cstddef>
#include <cmath>

nodiscard raytracer::Coord2D raytracer::rotatePoint2D(const raytracer::Coord2D& origin, const raytracer::Coord2D& point, raytracer::Angle angle, const bool rad)
{
    raytracer::Type radian = rad ? angle : raytracer::degToRad(angle);

    // Remove the origin
    raytracer::Type x = point.x - origin.x;
    raytracer::Type y = point.y - origin.y;

    // Rotation
    raytracer::Type cosR = std::cos(radian);
    raytracer::Type sinR = std::sin(radian);
    raytracer::Type xr = x * cosR - y * sinR;
    raytracer::Type yr = x * sinR + y * cosR;

    // Re apply the origin
    return {xr + origin.x, yr + origin.y};
}

/*
 * pitch - x
 * yaw   - y
 * roll  - z
*/
nodiscard raytracer::Coord raytracer::rotatePoint3D(const raytracer::Coord& origin, const raytracer::Coord& point, const raytracer::Direction& orientation, const bool rad)
/*{
    // Convert to rad
    raytracer::Type radX = raytracer::degToRad(angleX);
    raytracer::Type radY = raytracer::degToRad(angleY);

    // Remove the origin
    raytracer::Type x = point.x - origin.x;
    raytracer::Type y = point.y - origin.y;
    raytracer::Type z = point.z - origin.z;

    // Rotation x
    raytracer::Type cosX = std::cos(radX);
    raytracer::Type sinX = std::sin(radX);
    raytracer::Type y1 = y * cosX - z * sinX;
    raytracer::Type z1 = y * sinX + z * cosX;
    raytracer::Type x1 = x;

    // Rotation y
    raytracer::Type cosY = std::cos(radY);
    raytracer::Type sinY = std::sin(radY);
    raytracer::Type x2 = x1 * cosY - z1 * sinY;
    raytracer::Type z2 = x1 * sinY + z1 * cosY;
    raytracer::Type y2 = y1;

    // Re apply the origin
    return {x2 + origin.x, y2 + origin.y, z2 + origin.z};
}*/
{
    // Pre compute
    raytracer::Coord p = point - origin;
    raytracer::Type pitch = rad ? orientation.x : raytracer::degToRad(orientation.x);
    raytracer::Type yaw =   rad ? orientation.y : raytracer::degToRad(orientation.y);
    raytracer::Type roll =  rad ? orientation.z : raytracer::degToRad(orientation.z);

    // Rotation value
    raytracer::Type cosa = std::cos(yaw);
    raytracer::Type sina = std::sin(yaw);

    raytracer::Type cosb = std::cos(pitch);
    raytracer::Type sinb = std::sin(pitch);
    
    raytracer::Type cosc = std::cos(roll);
    raytracer::Type sinc = std::sin(roll);

    // Create the matrix
    raytracer::Type Axx = cosa * cosb;
    raytracer::Type Axy = cosa * sinb * sinc - sina * cosc;
    raytracer::Type Axz = cosa * sinb * cosc + sina * sinc;

    raytracer::Type Ayx = sina * cosb;
    raytracer::Type Ayy = sina * sinb * sinc + cosa * cosc;
    raytracer::Type Ayz = sina * sinb * cosc - cosa * sinc;

    raytracer::Type Azx = -sinb;
    raytracer::Type Azy = cosb * sinc;
    raytracer::Type Azz = cosb * cosc;

    // Apply the matrix
    raytracer::Type px = p.x;
    raytracer::Type py = p.y;
    raytracer::Type pz = p.z;
    p.x = Axx * px + Axy * py + Axz * pz;
    p.y = Ayx * px + Ayy * py + Ayz * pz;
    p.z = Azx * px + Azy * py + Azz * pz;
    return p + origin;
}

nodiscard raytracer::Direction raytracer::toLook(const raytracer::Direction& orientation)
{
    raytracer::Type pitch = raytracer::degToRad(orientation.x);
    raytracer::Type yaw = raytracer::degToRad(orientation.y);
    raytracer::Direction look;
    look.x = std::cos(pitch) * std::sin(yaw);
    look.y = std::sin(pitch);
    look.z = std::cos(pitch) * std::cos(yaw);
    return look.normalize();
}

static std::uint32_t hash_u32(std::uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

static float hash(float x, float y, float z)
{
    int xi = int(std::floor(x));
    int yi = int(std::floor(y));
    int zi = int(std::floor(z));

    uint32_t h = hash_u32(xi ^ hash_u32(yi) ^ hash_u32(zi));
    return (h & 0xFFFFFF) / float(0xFFFFFF);
}

static inline float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static float smoothNoise(const raytracer::Coord& p)
{
    int x = std::floor(p.x);
    int y = std::floor(p.y);
    int z = std::floor(p.z);

    float fx = p.x - x;
    float fy = p.y - y;
    float fz = p.z - z;

    float c000 = hash(x, y, z);
    float c100 = hash(x + 1, y, z);
    float c010 = hash(x, y + 1, z);
    float c110 = hash(x + 1, y + 1, z);

    float c001 = hash(x, y, z + 1);
    float c101 = hash(x + 1, y, z + 1);
    float c011 = hash(x, y + 1, z + 1);
    float c111 = hash(x + 1, y + 1, z + 1);

    float x00 = lerp(c000, c100, fx);
    float x10 = lerp(c010, c110, fx);
    float x01 = lerp(c001, c101, fx);
    float x11 = lerp(c011, c111, fx);

    float y0 = lerp(x00, x10, fy);
    float y1 = lerp(x01, x11, fy);

    return lerp(y0, y1, fz);
}

static float fbm(const raytracer::Coord& p)
{
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    for (std::size_t i = 0; i < 4; ++i) {
        total += smoothNoise(p * frequency) * amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    return total;
}

void raytracer::noise(const raytracer::Coord& p, raytracer::FColor& v, float strength, float size)
{
    v *= (1.0f + (fbm((p) * size) - 0.5f) * strength);
}
