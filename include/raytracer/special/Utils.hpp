/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file Utils.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef UTILS_H
    #define UTILS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Attribute
    #include "utils/utils.hpp"  // nodiscard
    #include "../Struct.hpp"    // raytracer::* (types)
    #include <cstdint>          // std::uint16_t

namespace raytracer { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* color */
void noise(const raytracer::Coord& p, raytracer::FColor& v, float strength = NOISE_STRENGTH, float size = NOISE_SIZE);
inline nodiscard raytracer::Color moyColor(raytracer::HugeColor color1, raytracer::HugeColor color2, float intensity = 1.0)
{return (color1 + color2 * intensity) / (1 + intensity);};
inline nodiscard raytracer::Color mergeLight(raytracer::FColor color, raytracer::FColor light, float coef = 1.0)
{
    if (color.x < 1e-8) color.x = NO_LIGHT_DEFAULT;
    if (color.y < 1e-8) color.y = NO_LIGHT_DEFAULT;
    if (color.z < 1e-8) color.z = NO_LIGHT_DEFAULT;
    color *= light / (coef * 255.0f);
    return {std::min(color.x, 255.0f), std::min(color.y, 255.0f), std::min(color.z, 255.0f)};
};
inline nodiscard raytracer::Color mergeColor(raytracer::HugeColor color1, raytracer::HugeColor color2, float intensity = 1.0)
{
    /*
    if (color1 == 0 && color2 != 0) return color2;
    else if (color2 == 0 && color1 != 0) return color1;
    else if (color1 == 0 && color2 == 0) return {0, 0, 0};
    else return color1 * color2 * intensity / 255;
    */
    //return (color1 + color2 * intensity / 255;
    //return (color1 + color2 * intensity) / (1 + intensity);
    raytracer::HugeColor newColor = color1 + (color2 * intensity);
    return {
        std::min(newColor.x, (std::uint16_t)255),
        std::min(newColor.y, (std::uint16_t)255),
        std::min(newColor.z, (std::uint16_t)255)
    };
};

/* computing */
inline nodiscard raytracer::Type degToRad(raytracer::Angle deg)
{return deg * M_PI / 180.0;};
inline nodiscard raytracer::Angle radToDeg(raytracer::Type rad)
{return rad / M_PI * 180.0;};
raytracer::Coord2D rotatePoint2D(const raytracer::Coord2D& origin, const raytracer::Coord2D& point, raytracer::Angle angle, const bool rad = false);
raytracer::Coord rotatePoint3D(const raytracer::Coord& origin, const raytracer::Coord& point, const raytracer::Direction& orientation, const bool rad = false);
raytracer::Direction toLook(const raytracer::Direction& orientation);

} // namespace end
#endif /* UTILS_H */
