/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file ILight.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef ILIGHT_H
    #define ILIGHT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../objects/AObject.hpp"   // raytracer::AObject
    #include "../rays/LightRay.hpp"     // raytracer::LightRay

namespace raytracer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ILight: public raytracer::AObject {
    public:
        // ---------- Pre-Function -------- //
        virtual void init(void) = 0; // Create default light rays
        virtual void reset(void) = 0; // Reset light rays values (don't recreate it)
        virtual std::vector<raytracer::LightRay*> getRays(void) const = 0;
        virtual raytracer::Color getColor(void) const = 0;
        virtual float getIntensity(void) const = 0;
        virtual void enableGlobal(void) = 0;
        virtual bool isGlobal(void) const = 0;
        virtual float getPower(void) const = 0;

        // ------------ Operator ---------- //
        ILight& operator=(const ILight& object) = delete;
        ILight& operator=(ILight&& object) = delete;

        // ---------- Constructor --------- //
        ILight() = default;
        ILight(const ILight& object) = delete;
        ILight(ILight&& object) = delete;

        // ----------- Destructor --------- //
        virtual ~ILight() = default;
};

} // namespace end
#endif /* ILIGHT_H */
