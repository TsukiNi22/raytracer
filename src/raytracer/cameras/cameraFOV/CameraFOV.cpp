/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file CameraFOV.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include "utils/utils.hpp"
#include "raytracer/cameras/CameraFOV.hpp"
#include "raytracer/special/Utils.hpp"
#include "raytracer/Struct.hpp"
#include "raytracer/Define.hpp"
#include "raytracer/rays/Ray.hpp"
#include <limits>
#include <cmath>

void raytracer::CameraFOV::parse(const libconfig::Setting& node)
{
    raytracer::ObjectDescriptor descriptor;

    // Setup the cframe
    raytracer::ObjectDescriptor::setCFrame(descriptor, node);

    // Other settings
    if (!node.exists("resolution"))
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The resolution field isn't defined for the camera");
    const libconfig::Setting& res = node["resolution"];
    this->setResolution({(int)res[0], (int)res[1]});

    double fieldOfView = 70.0f;
    if (node.lookupValue("fieldOfView", fieldOfView))
        this->_fieldOfView = fieldOfView;
    
    double renderDistance = RENDER_DISTANCE;
    if (node.lookupValue("renderDistance", renderDistance))
        this->_renderDistance = renderDistance;

    // Set the descriptor
    this->setObjectDescriptor(descriptor);
}

void raytracer::CameraFOV::init(void)
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

void raytracer::CameraFOV::reset(void)
{   
    utils::vector::OVector2<int> resolution = this->getResolution();
    raytracer::CFrame cframe = this->getCFrame();
    raytracer::Coord position = cframe.position;
    raytracer::Direction orientation = cframe.orientation;
    raytracer::Angle angleX = 0.0, angleY = 0.0;
    raytracer::Coord2D rotated;

    // For each rays set default light value
    for (raytracer::Ray* ray: this->_rays)
        ray->reset();

    // Pre compute values
    float fieldOfViewW = this->_fieldOfView * .5;
    float fieldOfViewH = (resolution.y / static_cast<float>(resolution.x)) * this->_fieldOfView * .5;

    // Set rays init position & orientation
    for (int y = 0; y < resolution.y; ++y) {
        angleX = ((y / (resolution.y - 1.0f)) * 2.0f - 1.0f) * fieldOfViewH;
        for (int x = 0; x < resolution.x; ++x) {
            angleY = ((x / (resolution.x - 1.0f)) * 2.0f - 1.0f) * fieldOfViewW;
            // Orientation
            cframe.orientation = orientation;
            cframe.orientation.x += angleX;
            cframe.orientation.y += angleY;
            // Look (orientation modified)
            cframe.look = raytracer::toLook(cframe.orientation);
            // Apply (roll)
            rotated = raytracer::rotatePoint2D({0.0, 0.0}, {cframe.look.x, cframe.look.y}, cframe.orientation.z);
            cframe.look.x = rotated.x;
            cframe.look.y = rotated.y;
            // Position (FOV = same as the origin)
            cframe.position = position;
            this->_rays[y * resolution.x + x]->setCFrame(cframe);
        }
    }
}
