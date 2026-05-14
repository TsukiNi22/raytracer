/**************************************************************\
Edition:
##  @date 14/05/2026 by @author Tsukini

File Name:
##  @file Struct.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef STRUCT_H
    #define STRUCT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Vector
    #define _Exception
    #define _Attribute
    #include "utils/utils.hpp"  // utils::exception::*, utils::vector::*, hot, nodiscard
    #include "Define.hpp"       // SPACE_CHUNK_SIZE, COLOR_CHUNK_SIZE
    #include <libconfig.h++>    // libconfig::Setting
    #include <cstddef>          // std::size_t
    #include <cstdint>          // std::uint8_t, std::uint16_t, std::int32_t
    #include <vector>           // std::vector
    #include <tuple>            // std::tuple
    #include <bit>              // std::bit_cast

namespace raytracer { // namespace start
//----------------------------------------------------------------//
/* CLASS & TYPEDEF */

using Type = double; // Type used everywhere for coord, angle, direction, computing, ...
using V2Type = utils::vector::OVector2<raytracer::Type>;
using V3Type = utils::vector::OVector3<raytracer::Type>;

using Coord2D = utils::vector::OVector2<raytracer::Type>;
using Coord = utils::vector::OVector3<raytracer::Type>;
using Direction = utils::vector::OVector3<raytracer::Type>; // Generaly normalized
using Angle = raytracer::Type;

using Chunk = utils::vector::OVector3<std::int32_t>;

using Resolution = utils::vector::OVector2<std::uint16_t>;

using Color = utils::vector::OVector3<std::uint8_t>;
using HugeColor = utils::vector::OVector3<std::uint16_t>;
using FColor = utils::vector::OVector3<float>;

using Vertice = utils::vector::OVector3<raytracer::Type>;
using Face = std::vector<Vertice>;

struct CFrame {
    raytracer::Coord position = {0.0, 0.0, 0.0};
    raytracer::Direction orientation = {0.0, 0.0, 0.0};
    raytracer::Angle rotation = 0.0;
    inline hot nodiscard bool operator==(const CFrame& other) const noexcept {
        return (
            position.x == other.position.x &&
            position.y == other.position.y &&
            position.z == other.position.z &&
            orientation.x == other.orientation.x &&
            orientation.y == other.orientation.y &&
            orientation.z == other.orientation.z &&
            rotation == other.rotation
        );
    }
};

struct CFrameHash {
    inline hot nodiscard std::size_t operator()(const raytracer::CFrame& cframe) const noexcept {
        auto bits = [](double v) -> std::uint64_t {
            return std::bit_cast<std::uint64_t>(v);
        };
        std::size_t h = bits(cframe.position.x) * 73856093ull;
        h ^= bits(cframe.position.y) * 19349663ull;
        h ^= bits(cframe.position.z) * 83492791ull;
        h ^= bits(cframe.orientation.x) * 2654435761ull;
        h ^= bits(cframe.orientation.y) * 2246822519ull;
        h ^= bits(cframe.orientation.z) * 3266489917ull;
        return h ^ (bits(cframe.rotation) * 668265263ull);
    }
};

enum class Shape {
    None,
    Point,
    Plane,
    Sphere,
    Cylinder,
    Cone,
};

inline hot nodiscard raytracer::Chunk getSpaceChunk(const raytracer::Coord& point)
{return {point.x / SPACE_CHUNK_SIZE, point.y / SPACE_CHUNK_SIZE, point.z / SPACE_CHUNK_SIZE};}
inline hot nodiscard raytracer::Chunk getColorChunk(const raytracer::Coord& point)
{return {point.x / COLOR_CHUNK_SIZE, point.y / COLOR_CHUNK_SIZE, point.z / COLOR_CHUNK_SIZE};}

struct ChunkHash {
    inline hot nodiscard std::size_t operator()(const raytracer::Chunk& chunk) const noexcept {
        std::size_t h = chunk.x * 73856093u;
        h ^= chunk.y * 19349663u + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h ^ (chunk.z * 83492791u + 0x9e3779b9 + (h << 6) + (h >> 2));
    }
};

struct ChunkLightData {
    raytracer::Coord position;
    raytracer::Color color;
    float intensity;
};

class IMaterial;

struct ObjectDescriptor {
    raytracer::Shape id = raytracer::Shape::None;

    /* all */
    raytracer::CFrame cframe;
    raytracer::CFrame cframeOrigin;
    raytracer::IMaterial* material = nullptr;

    /* obj */
    std::string obj;
    std::vector<raytracer::Chunk> chunks;
    std::vector<raytracer::Face> faces;
    float scale = 1.0f;

    // ---------- Constructor --------- //
    ObjectDescriptor() = default;
    ObjectDescriptor(const raytracer::CFrame& cframe): cframe{cframe}, cframeOrigin{cframe} {};

    // -------- Static-Function ------- //
    static void setCFrame(raytracer::ObjectDescriptor& descriptor, const libconfig::Setting& node)
    {
        // Check existantce
        if (!node.exists("cframe"))
            throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The CFrame field isn't defined");
        const libconfig::Setting& cframe = node["cframe"];
        if (!cframe.exists("position"))
            throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The position field isn't defined for the CFrame");
        else if (!cframe.exists("orientation"))
            throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The orientation field isn't defined for the CFrame");

        // Set values
        const libconfig::Setting& pos = cframe["position"];
        descriptor.cframe.position.x = pos[0];
        descriptor.cframe.position.y = pos[1];
        descriptor.cframe.position.z = pos[2];
        const libconfig::Setting& rot = cframe["orientation"];
        descriptor.cframe.orientation.x = rot[0];
        descriptor.cframe.orientation.y = rot[1];
        descriptor.cframe.orientation.z = rot[2];
        double rotation = 0.0;
        if (cframe.lookupValue("rotation", rotation))
            descriptor.cframe.rotation = rotation;

        // Normalize orientation (only if not null)
        if (descriptor.cframe.orientation.x >= 1e-8 || descriptor.cframe.orientation.y >= 1e-8 || descriptor.cframe.orientation.z >= 1e-8
            || descriptor.cframe.orientation.x <= -1e-8 || descriptor.cframe.orientation.y <= -1e-8 || descriptor.cframe.orientation.z <= -1e-8)
            descriptor.cframe.orientation = descriptor.cframe.orientation.normalize();

        descriptor.cframeOrigin = descriptor.cframe;
    }
};

} // namespace end
#endif /* STRUCT_H */
