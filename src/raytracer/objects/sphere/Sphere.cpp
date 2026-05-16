/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file Sphere.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#define _Exception
#include "utils/utils.hpp"
#include "raytracer/objects/Sphere.hpp"
#include "raytracer/Struct.hpp"

cold void raytracer::Sphere::parse(const libconfig::Setting& node)
{
    raytracer::ObjectDescriptor descriptor;

    // Setup the cframe
    raytracer::ObjectDescriptor::setCFrame(descriptor, node);
    
    // Other settings
    if (!node.exists("radius"))
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The radius field isn't defined for the object");
    this->_radius = node["radius"];

    // Set the descriptor
    this->setObjectDescriptor(descriptor);
}

std::pair<float, const raytracer::Face*> raytracer::Sphere::willCollide(const raytracer::Coord& point, unused const raytracer::Direction& orientation) const
{
    const raytracer::Coord& center = this->getCFrame().position;
    float radius = this->_radius * 0.5f;
    raytracer::Direction oc = point - center;
    float a = orientation.dot(orientation);
    float b = 2.0f * oc.dot(orientation);
    float c = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) return {std::numeric_limits<float>::max(), nullptr};
    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);
    float t = std::numeric_limits<float>::max();
    if (t1 > EPSILON) t = t1;
    else if (t2 > EPSILON) t = t2;
    return {t, nullptr};
}

hot raytracer::Direction raytracer::Sphere::computeHit(const raytracer::Coord& point, unused const raytracer::Face* face) const
{
    return (point - this->getCFrame().position).normalize();
}
