/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file ARay.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include "utils/utils.hpp"
#include "raytracer/rays/ARay.hpp"
#include <unordered_set>

void raytracer::ARay::parse(unused const libconfig::Setting& node)
{
    throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::NotSupportedFunction, "Rays does not support: parse");
}

void raytracer::ARay::computeObjects(raytracer::Type renderDistance, const std::vector<raytracer::IObject*>& objects, const std::unordered_map<raytracer::Chunk, std::vector<raytracer::IObject*>, raytracer::ChunkHash>& objectsChunks)
{
    raytracer::Type renderDistanceSquared = renderDistance * renderDistance;
    raytracer::CFrame cframe = this->getCFrame();
    raytracer::Coord positionOrigin = cframe.position;
    raytracer::Direction direction = cframe.look * (SPACE_CHUNK_SIZE / cframe.look.length());
    raytracer::Chunk chunk;
    std::unordered_set<IObject*> seen;
    seen.reserve(objects.size());
    std::pair<float, const raytracer::Face*> hit = {std::numeric_limits<float>::max(), nullptr};

    // Reset
    this->_hits.clear();
    this->_hits.reserve(objects.size());
    if (objects.size() == 0) return;

    // Get every object other than *.obj
    for (raytracer::IObject* object: objects) {
        if (object->getObjectDescriptor().faces.size() == 0) {
            hit = object->willCollide(cframe.position, cframe.look);
            this->_hits.push_back({object, hit.first, hit.second});
            seen.insert(object);
        }
    }
    if (seen.size() == objects.size()) return;

    // Get the ray chunks (While not out of bounds)
    for (; (cframe.position - positionOrigin).lengthSquared() < renderDistanceSquared; cframe.position += direction) {
        chunk = raytracer::getSpaceChunk(cframe.position);
        // Get the shape that will intersect with the chunk
        auto it = objectsChunks.find(chunk);
        if (it != objectsChunks.end()) {
            for (raytracer::IObject* object: it->second) {
                if (!seen.insert(object).second) continue;
                hit = object->willCollide(cframe.position, cframe.look);
                if (!hit.second) continue; // Check if it will collide in the future at least once
                this->_hits.push_back({object, hit.first, hit.second});
            }
        }
        if (seen.size() == objects.size()) return;
    }
}
