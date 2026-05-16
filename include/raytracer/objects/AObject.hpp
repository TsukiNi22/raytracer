/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file AObject.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef AOBJECT_H
    #define AOBJECT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Vector
    #define _Attribute
    #include "utils/utils.hpp"  // utils::vector::Vector3, nodiscard
    #include "../Struct.hpp"    // raytracer::* (types)
    #include "IObject.hpp"      // raytracer::IObject
    #include <unordered_map>    // std::unordered_map
    #include <vector>           // std::vector
    #include <mutex>            // std::mutex

namespace raytracer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class AObject: public raytracer::IObject {
    private:
        std::mutex _lock;
        raytracer::IObject* _immunity = nullptr; // Dosen't comput SDF with this object
        std::unordered_map<raytracer::Chunk, std::vector<raytracer::ChunkLightData>, raytracer::ChunkHash> _lightData;

    protected:
        raytracer::ObjectDescriptor _descriptor;

    public:
        // ---------- Pre-Function -------- //
        /* loading */
        void loadObj(const std::string& path) final {this->loadObj(path, this->_descriptor);};
        void loadObj(const std::string& path, raytracer::ObjectDescriptor& descriptor) final;

        /* 3D logic */
        void reflectRay(raytracer::IRay* ray, const raytracer::Face* face) const final;
        std::pair<float, const raytracer::Face*> willCollide(const raytracer::Coord& point, const raytracer::Direction& orientation) const override;
        raytracer::Direction computeHit(const raytracer::Coord& point, const raytracer::Face* face) const override;

        /* color handling */
        std::pair<raytracer::Color, bool> getPointColor(const raytracer::Coord& point) const final;
        void addLightData(raytracer::Coord position, raytracer::Color color, float intensity) final;

        // ------------ Function ---------- //
        /* 3D logic */
        hot inline void setImmunity(raytracer::IObject* object) final {this->_immunity = object;};
        hot inline nodiscard raytracer::IObject* getImmunity(void) const final {return this->_immunity;};

        /* movement */
        hot inline void translate(const raytracer::Coord& v) final {this->_descriptor.cframe.position += v;};
        inline void rotate(const raytracer::Direction& v) final {this->_descriptor.cframe.orientation += v;};

        /* color handling */
        inline void clearLightData(void) final {this->_lightData.clear();};

        /* getter & setter */
        inline void setObjectDescriptor(const raytracer::ObjectDescriptor& descriptor) final {this->_descriptor = descriptor;};
        inline void setCFrame(const raytracer::CFrame& cframe, bool origin = true) final {this->_descriptor.cframe = cframe; if (origin) this->_descriptor.cframeOrigin = cframe;};
        hot inline nodiscard raytracer::ObjectDescriptor& getObjectDescriptor(void) final {return this->_descriptor;};
        hot inline nodiscard const raytracer::ObjectDescriptor& getObjectDescriptor(void) const final {return this->_descriptor;};
        hot inline nodiscard raytracer::CFrame getCFrameOrigin(void) const final {return this->_descriptor.cframeOrigin;};
        hot inline nodiscard raytracer::CFrame getCFrame(void) const final {return this->_descriptor.cframe;};

        // ------------ Operator ---------- //
        AObject& operator=(const AObject& object) = delete;
        AObject& operator=(AObject&& object) = delete;

        // ---------- Constructor --------- //
        AObject() {this->_lightData.max_load_factor(LOAD_FACTOR);};
        AObject(const AObject& object) = delete;
        AObject(AObject&& object) = delete;

        // ----------- Destructor --------- //
        ~AObject() = default;
};

} // namespace end
#endif /* AOBJECT_H */
