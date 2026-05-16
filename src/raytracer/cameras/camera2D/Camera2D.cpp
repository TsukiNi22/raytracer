/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file Camera2D.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include "utils/utils.hpp"
#include "raytracer/cameras/Camera2D.hpp"
#include "raytracer/special/Utils.hpp"
#include "raytracer/Struct.hpp"
#include "raytracer/Define.hpp"
#include "raytracer/rays/Ray.hpp"
#include <limits>
#include <cmath>

void raytracer::Camera2D::parse(const libconfig::Setting& node)
{
    raytracer::ObjectDescriptor descriptor;

    // Setup the cframe
    raytracer::ObjectDescriptor::setCFrame(descriptor, node);

    // Other settings
    if (!node.exists("resolution"))
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The resolution field isn't defined for the camera");
    const libconfig::Setting& res = node["resolution"];
    this->setResolution({(int)res[0], (int)res[1]});

    double renderDistance = RENDER_DISTANCE;
    if (node.lookupValue("renderDistance", renderDistance))
        this->_renderDistance = renderDistance;

    // Set the descriptor
    this->setObjectDescriptor(descriptor);
}

void raytracer::Camera2D::init(void)
{
    std::size_t size = static_cast<std::size_t>(this->getResolution().x) * this->getResolution().y;

    // Clear old data
    this->_screen.clear();
    for (std::size_t i = 0; i < this->_rays.size(); ++i)
        delete this->_rays[i];
    this->_rays.clear();

    // Resize screen size
    this->_screen.reserve(size);
    this->_rays.reserve(size);
    this->_screen.resize(size, DEFAULT_COLOR);
    this->_rays.resize(size, nullptr);
    for (std::size_t i = 0; i < size; ++i)
        this->_rays[i] = new raytracer::Ray();
}

void raytracer::Camera2D::reset(void)
{
    utils::vector::OVector2<int> resolution = this->getResolution();
    raytracer::CFrame cframe = this->getCFrame();
    raytracer::Coord position = cframe.position;
    raytracer::Direction orientation = cframe.orientation;
    raytracer::Coord2D rotated;

    // For each rays set default light value
    for (raytracer::Ray* ray: this->_rays)
        ray->reset();

    // Set rays init position & orientation
    utils::vector::OVector2<float> resolution2 = resolution / 2;
    for (int y = 0; y < resolution.y; ++y) {
        for (int x = 0; x < resolution.x; ++x) {
            // Orientation (2D = same as the origin)
            cframe.orientation = orientation;
            // Look (2D = same as the origin)
            cframe.look = cframe.look;
           // Position
            cframe.position = position;
            cframe.position.x += (resolution2.x - x);
            cframe.position.y += (resolution2.y - y);
            // Apply rotation (roll)
            rotated = raytracer::rotatePoint2D({position.x, position.y}, {cframe.position.x, cframe.position.y}, cframe.orientation.z);
            cframe.position.x = rotated.x;
            cframe.position.y = rotated.y;
            // Apply rotation (pitch + yaw)
            cframe.position = raytracer::rotatePoint3D(position, cframe.position, cframe.look);
            this->_rays[y * resolution.x + x]->setCFrame(cframe);
        }
    }
}
