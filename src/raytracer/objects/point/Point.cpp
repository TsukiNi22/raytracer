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
#define _Exception
#include "utils/utils.hpp"
#include "raytracer/objects/Point.hpp"
#include "raytracer/Struct.hpp"

cold void raytracer::Point::parse(const libconfig::Setting& node)
{
    raytracer::ObjectDescriptor descriptor;

    // Setup the cframe
    raytracer::ObjectDescriptor::setCFrame(descriptor, node);

    // Set the descriptor
    this->setObjectDescriptor(descriptor);
}

std::pair<float, const raytracer::Face*> raytracer::Point::willCollide(unused const raytracer::Coord& point, unused const raytracer::Direction& orientation) const
{
    return {std::numeric_limits<float>::max(), nullptr};
    //return {(point - this->getCFrame().position).length(), nullptr};
}

hot raytracer::Direction raytracer::Point::computeHit(unused const raytracer::Coord& point, unused const raytracer::Face* face) const
{
    return {0.0, 0.0, 0.0}; // Nop
}
