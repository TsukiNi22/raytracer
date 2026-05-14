/**************************************************************\
Edition:
##  @date 13/05/2026 by @author Tsukini

File Name:
##  @file Sphere.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef SPHERE_H
    #define SPHERE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Vector
    #include "utils/utils.hpp"  // utils::vector::Vector3
    #include "AObject.hpp"      // raytracer::AObject
    #include "../rays/IRay.hpp" // raytracer::IRay
    #include <memory>           // std::shared_ptr

namespace raytracer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Sphere: public raytracer::AObject {
    private:
        float _radius = 1.0f;

    public:
        // ---------- Pre-Function -------- //
        void parse(const libconfig::Setting& node) final;
        std::pair<float, const raytracer::Face*> willCollide(const raytracer::Coord& point, const raytracer::Direction& orientation) const final;
        raytracer::Direction computeHit(const raytracer::Coord& point, const raytracer::Face* face) const final;

        // ------------ Operator ---------- //
        Sphere& operator=(const Sphere& object) = delete;
        Sphere& operator=(Sphere&& object) = delete;

        // ---------- Constructor --------- //
        Sphere() = default;
        Sphere(const Sphere& object) = delete;
        Sphere(Sphere&& object) = delete;

        // ----------- Destructor --------- //
        ~Sphere() = default;
};

} // namespace end
#endif /* SPHERE_H */
