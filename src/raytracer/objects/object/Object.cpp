/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file Object.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#include "utils/utils.hpp"
#include "raytracer/objects/Object.hpp"
#include "raytracer/Struct.hpp"

void raytracer::Object::parse(const libconfig::Setting& node)
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

    // Get the obj path & load the shape with vertices
    if (!node.lookupValue("obj", descriptor.obj))
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The obj field isn't defined for the object");
}
