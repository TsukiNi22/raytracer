/**************************************************************\
Edition:
##  @date 14/05/2026 by @author Tsukini

File Name:
##  @file Cone.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#include "utils/utils.hpp"
#include "raytracer/objects/Cone.hpp"
#include "raytracer/Struct.hpp"

void raytracer::Cone::parse(const libconfig::Setting& node)
{
    raytracer::ObjectDescriptor descriptor;

    // Setup the cframe
    raytracer::ObjectDescriptor::setCFrame(descriptor, node);

    // Other settings
    double scale = 1.0;
    if (node.lookupValue("scale", scale))
        descriptor.scale = scale;

    // Set the descriptor
    this->setObjectDescriptor(descriptor);

    // Load the obj sub path (to load the vertex in the future)
    descriptor.obj = "cone.obj";
}
