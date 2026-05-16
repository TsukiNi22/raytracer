/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file Point.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#include "utils/utils.hpp"
#include "raytracer/lights/Point.hpp"
#include "raytracer/special/Utils.hpp"
#include "raytracer/Struct.hpp"
#include <cstdint>
#include <cstdlib>
#include <cmath>

void raytracer::Point::parse(const libconfig::Setting& node)
{
    raytracer::ObjectDescriptor descriptor;

    // Setup the cframe
    raytracer::ObjectDescriptor::setCFrame(descriptor, node);

    // Other settings
    double intensity = 1.0;
    if (node.lookupValue("intensity", intensity))
        this->_intensity = intensity;

    double lumen = 1.0;
    if (node.lookupValue("lumen", lumen))
        this->_lumen = lumen;

    if (node.exists("color")) {
        const libconfig::Setting& color = node["color"];
        this->_color = {
            (int)color[0],
            (int)color[1],
            (int)color[2]
        };
    }

    // Set the descriptor
    this->setObjectDescriptor(descriptor);
}

void raytracer::Point::init(void)
{
    std::size_t size = MAX_LIGHT_RAY;

    // Clear old data
    this->_rays.clear();

    // Resize screen size
    this->_rays.reserve(size);
    this->_rays.resize(size, nullptr);
    for (std::size_t i = 0; i < size; ++i)
        this->_rays[i] = new raytracer::LightRay();
}

void raytracer::Point::reset(void)
{
    utils::vector::OVector2<int> resolution = {std::sqrt(MAX_LIGHT_RAY), std::sqrt(MAX_LIGHT_RAY)};
    raytracer::CFrame cframe = this->getCFrame();
    raytracer::Coord position = cframe.position;
    raytracer::Direction orientation = cframe.orientation;
    raytracer::Angle angleX = 0.0, angleY = 0.0;
    raytracer::Coord2D rotated;

    // For each rays set default light value
    for (raytracer::LightRay* ray: this->_rays) {
        ray->reset();
        ray->setColor(this->_color);
        ray->setIntensity(this->_intensity);
        ray->setPower(this->_lumen);
    }

    // Pre compute values (360 = spherical)
    float fieldOfLightW = 360.0 * .5;
    float fieldOfLightH = (resolution.y / static_cast<float>(resolution.x)) * 360.0 * .5;

    // Set rays init position & orientation
    for (int y = 0; y < resolution.y; ++y) {
        angleX = ((y / (resolution.y - 1.0f)) * 2.0f - 1.0f) * fieldOfLightH;
        for (int x = 0; x < resolution.x; ++x) {
            angleY = ((x / (resolution.x - 1.0f)) * 2.0f - 1.0f) * fieldOfLightW;
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
