/**************************************************************\
Edition:
##  @date 14/05/2026 by @author Tsukini

File Name:
##  @file AObject.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#define _Exception
#include "utils/utils.hpp"
#include "raytracer/special/Utils.hpp"
#include "raytracer/objects/AObject.hpp"
#include "raytracer/rays/IRay.hpp"
#include "raytracer/Raytracer.hpp"
#include "raytracer/Struct.hpp"
#include "raytracer/Define.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <algorithm>
#include <exception>
#include <iostream>
#include <optional>
#include <cmath>

static void processChunk(const std::vector<raytracer::ChunkLightData>& data,
    const raytracer::Coord& point, raytracer::FColor& lightColor,
    float& count)
{
    static raytracer::Type limit = LIGHT_COLOR_LIMIT * LIGHT_COLOR_LIMIT;
    const raytracer::Type px = point.x;
    const raytracer::Type py = point.y;
    const raytracer::Type pz = point.z;

    for (const raytracer::ChunkLightData& s: data) {
        // Check if the point is near the light ray
        const raytracer::Type dx = px - s.position.x;
        const raytracer::Type dy = py - s.position.y;
        const raytracer::Type dz = pz - s.position.z;
        const raytracer::Type d2 = dx * dx + dy * dy + dz * dz;
        if (d2 > limit) continue;

        // Compute the coef
        //float proximityCoef = 1.0f - (d2 / limit);

        // Fuse the colors
        lightColor.x += s.color.x * s.intensity;// * proximityCoef;
        lightColor.y += s.color.y * s.intensity;// * proximityCoef;
        lightColor.z += s.color.z * s.intensity;// * proximityCoef;
        ++count;
    }
}

hot std::pair<raytracer::Color, bool> raytracer::AObject::getPointColor(const raytracer::Coord& point) const
{
    raytracer::FColor lightColor = DEFAULT_COLOR;
    raytracer::Chunk chunk;
    float count = 0;

    // Get the min & max chunk
    raytracer::Coord minPoint = {
        point.x - LIGHT_COLOR_LIMIT,
        point.y - LIGHT_COLOR_LIMIT,
        point.z - LIGHT_COLOR_LIMIT
    };
    raytracer::Coord maxPoint = {
        point.x + LIGHT_COLOR_LIMIT,
        point.y + LIGHT_COLOR_LIMIT,
        point.z + LIGHT_COLOR_LIMIT
    };
    raytracer::Chunk minChunk = raytracer::getColorChunk(minPoint);
    raytracer::Chunk maxChunk = raytracer::getColorChunk(maxPoint);

    // For each chunk around the chunk point
    for (int z = minChunk.z; z <= maxChunk.z; ++z)
    for (int y = minChunk.y; y <= maxChunk.y; ++y)
    for (int x = minChunk.x; x <= maxChunk.x; ++x) {
        chunk = {x, y, z};
        auto it = this->_lightData.find(chunk);
        if (it == this->_lightData.end()) continue;
        processChunk(it->second, point, lightColor, count);
    }

    // No light ray on this hit
    if (!DEFAULT_LIGHT && !count)
        return {DEFAULT_COLOR, false};

    // Apply the light modifier
    return {raytracer::mergeLight(this->getObjectDescriptor().material->getColor(), lightColor, count), true};
}

hot void raytracer::AObject::addLightData(raytracer::Coord position, raytracer::Color color, float intensity)
{
    raytracer::Chunk chunk = raytracer::getColorChunk(position);
    std::lock_guard<std::mutex> lock(this->_lock);
    this->_lightData[chunk].push_back({position, color, intensity});
}

cold void raytracer::AObject::loadObj(const std::string& path, raytracer::ObjectDescriptor& descriptor)
{
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig config;
    raytracer::Chunk chunkMin, chunkMax;
    raytracer::Chunk chunk;
    raytracer::Vertice verticeMin, verticeMax;
    raytracer::Vertice vertice;
    config.triangulate = true;

    // Check error & warning
    if (!reader.ParseFromFile(path, config)) {
        std::string err = reader.Error();
        err.pop_back();
        utils::exception::CustomException e(utils::exception::Error, utils::exception::Code::Parser, err);
        std::cout << e.formated() << std::endl;
    }
    if (!reader.Warning().empty()) {
        utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::Parser, reader.Warning());
        std::cout << e.formated() << std::endl;
    }

    // Compute world rotation
    raytracer::Coord orientation = descriptor.cframe.orientation;
    raytracer::Type len = orientation.dot(orientation);
    if (len < 1e-12) orientation = {0, 0, 1}; // Fallback orientation
    raytracer::Coord forward = orientation;
    raytracer::Coord worldUp = {0, 1, 0};
    if (std::abs(forward.dot(worldUp)) > 0.999) worldUp = {1, 0, 0}; // Edge case, parrallel
    raytracer::Coord right = (worldUp.cross(forward)).normalize();
    raytracer::Coord up = forward.cross(right).normalize();

    // Get the file content
    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    // For each shape get the different vertex
    for (const auto& shape: shapes) {
        // For each face get the vertices (only accept face of 3 or less vertices)
        for (std::size_t f = 0, indexOffset = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            // Check the number of vertices
            std::size_t verticesCount = shape.mesh.num_face_vertices[f];
            if (verticesCount < MAX_VERTICES)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, std::string("Invalid vertices number, sould be inferior or equal to '") + std::to_string(MAX_VERTICES) + "', but got: " + std::to_string(f));

            // Setup the vertex
            raytracer::Face face;
            for (std::size_t v = 0; v < verticesCount; ++v) {
                const tinyobj::index_t& idx = shape.mesh.indices[indexOffset + v];

                // Get the vertice
                vertice.x = attrib.vertices[3 * idx.vertex_index + 0];
                vertice.y = attrib.vertices[3 * idx.vertex_index + 1];
                vertice.z = attrib.vertices[3 * idx.vertex_index + 2];

                // Apply rotation & offset
                raytracer::Coord rotated = right * vertice.x + up * vertice.y + forward * vertice.z;
                rotated *= descriptor.scale;
                vertice.x = rotated.x + descriptor.cframe.position.x;
                vertice.y = rotated.y + descriptor.cframe.position.y;
                vertice.z = rotated.z + descriptor.cframe.position.z;
                vertice.x += descriptor.cframe.position.x;
                vertice.y += descriptor.cframe.position.y;
                vertice.z += descriptor.cframe.position.z;

                // Min & Max
                if (v == 0) {
                    verticeMin = vertice;
                    verticeMax = vertice;
                } else {
                    verticeMin.x = std::min(verticeMin.x, vertice.x);
                    verticeMin.y = std::min(verticeMin.y, vertice.y);
                    verticeMin.z = std::min(verticeMin.z, vertice.z);
                    verticeMax.x = std::max(verticeMax.x, vertice.x);
                    verticeMax.y = std::max(verticeMax.y, vertice.y);
                    verticeMax.z = std::max(verticeMax.z, vertice.z);
                }

                // Store the vertice
                face.push_back(vertice);
            }

            // Store the face
            chunkMin = raytracer::getSpaceChunk(verticeMin);
            chunkMax = raytracer::getSpaceChunk(verticeMax);
            for (int z = chunkMin.z; z <= chunkMax.z; ++z)
            for (int y = chunkMin.y; y <= chunkMax.y; ++y)
            for (int x = chunkMin.x; x <= chunkMax.x; ++x) {
                chunk = {x, y, z};
                descriptor.chunks.push_back(chunk);
            }
            descriptor.faces.push_back(face);
            indexOffset += verticesCount;
        }
    }
}

hot void raytracer::AObject::reflectRay(raytracer::IRay* ray, const raytracer::Face* face) const
{
    raytracer::CFrame cframe = ray->getCFrame();
    raytracer::Coord orientation = cframe.orientation;
    raytracer::Coord hit = this->computeHit(cframe.position, face).normalize();
    if (orientation.dot(hit) > 0) hit = -hit;
    orientation = orientation - hit * (2.0 * orientation.dot(hit));
    cframe.orientation = orientation;
    ray->setCFrame(cframe, false);
}

hot static nodiscard std::optional<float> segmentCollide(const raytracer::Coord& point, const raytracer::Direction& orientation, const raytracer::Vertice& a, const raytracer::Vertice& b)
{
    raytracer::Direction ab = b - a;
    raytracer::Direction ap = a - point;
    float ab2 = ab.lengthSquared();
    float denom = orientation.lengthSquared() * ab2 - std::pow(orientation.dot(ab), 2);
    if (denom < EPSILON) return std::nullopt;
    float t = (ap.dot(orientation) * ab2 - ap.dot(ab) * orientation.dot(ab)) / denom;
    float u = (ap.dot(orientation) * orientation.dot(ab) - ap.dot(ab) * orientation.lengthSquared()) / denom;
    if (t >= 0.0f && u >= 0.0f && u <= 1.0f) return t;
    return std::nullopt;
}

hot static nodiscard std::optional<float> triangleCollide(const raytracer::Coord& point, const raytracer::Direction& orientation, const raytracer::Vertice& a, const raytracer::Vertice& b, const raytracer::Vertice& c)
/*{
    raytracer::Direction edge1 = b - a;
    raytracer::Direction edge2 = c - a;
    raytracer::Direction h = orientation.cross(edge2);
    float det = edge1.dot(h);
    if (det > -EPSILON && det < EPSILON) return std::nullopt;
    float invDet = 1.0f / det;
    raytracer::Direction s = point - a;
    float u = invDet * s.dot(h);
    if (u < 0.0f || u > 1.0f) return std::nullopt;
    raytracer::Direction q = s.cross(edge1);
    float v = invDet * orientation.dot(q);
    if (v < 0.0f || (u + v) > 1.0f) return std::nullopt;
    float t = invDet * edge2.dot(q);
    if (t > EPSILON) return t;
    return std::nullopt;
}*/
{
    raytracer::Direction edge1 = b - a;
    raytracer::Direction edge2 = c - a;
    //const raytracer::Direction normal = edge1.cross(edge2);
	//if (normal.dot(orientation) > 0) return std::nullopt;
    raytracer::Direction ray_cross_e2 = orientation.cross(edge2);
    float det = edge1.dot(ray_cross_e2);
    if (std::abs(det) < EPSILON) return std::nullopt;
    float inv_det = 1.0 / det;
    raytracer::Direction s = point - a;
    float u = inv_det * s.dot(ray_cross_e2);
    if (u < -EPSILON || u - 1 > EPSILON) return std::nullopt;
    raytracer::Direction s_cross_e1 = s.cross(edge1);
    float v = inv_det * orientation.dot(s_cross_e1);
    if (v < -EPSILON || u + v - 1 > EPSILON) return std::nullopt;
    float t = inv_det * edge2.dot(s_cross_e1);
    if (t > EPSILON) return t;
    return std::nullopt;
}

hot nodiscard std::pair<float, const raytracer::Face*> raytracer::AObject::willCollide(const raytracer::Coord& point, const raytracer::Direction& orientation) const
{
    float t = std::numeric_limits<float>::max();
    const raytracer::Face* tface = nullptr;

    // For each face
    for (const raytracer::Face& face: this->getObjectDescriptor().faces) {
        std::optional<float> dist;
        // Dispatch the computing
        switch (face.size()) {
            case 1: dist = std::nullopt; break;
            case 2: dist = segmentCollide(point, orientation, face[0], face[1]); break;
            case 3: dist = triangleCollide(point, orientation, face[0], face[1], face[2]); break;
            default:
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "Invalid number of vertices for a face on the object to render");
        }
        if (dist && *dist < t) {
            t = *dist;
            tface = &face;
        }
    }

    return {t, tface};
}

hot static nodiscard raytracer::Coord segmentHit(const raytracer::Coord& point, const raytracer::Vertice& a, const raytracer::Vertice& b)
{
    raytracer::Coord ab = b - a;
    raytracer::Coord ap = point - a;
    raytracer::Coord proj = a + ab * (ap.dot(ab) / ab.lengthSquared());
    return (point - proj).normalize();
}

hot static nodiscard raytracer::Coord triangleHit(const raytracer::Vertice& a, const raytracer::Vertice& b, const raytracer::Vertice& c)
{
    raytracer::Coord n = (b - a).cross(c - a);
    return n.normalize();
}

hot nodiscard raytracer::Direction raytracer::AObject::computeHit(const raytracer::Coord& point, const raytracer::Face* facePtr) const
{
    // Check if the t was already computed
    if (!facePtr) unlikely {
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::InvalidAction, "Can't compute the perpendicular vector for the hit point before the t");
    }

    // Dispatch the computing
    const raytracer::Face face = *(facePtr);
    switch (face.size()) {
        case 1: return (point - face[0]).normalize();
        case 2: return segmentHit(point, face[0], face[1]);
        case 3: return triangleHit(face[0], face[1], face[2]);
        default:
            throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "Invalid number of vertices for a face on the object to render");
    }
}
