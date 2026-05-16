/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file ARay.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef ARAY_H
    #define ARAY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Attribute
    #include "utils/utils.hpp"          // nodiscard
    #include "../objects/AObject.hpp"   // raytracer::AObject
    #include "../Define.hpp"            // values
    #include "IRay.hpp"                 // raytracer::IRay

namespace raytracer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ARay: public raytracer::IRay {
    protected:
        bool _alive = true;
        std::vector<std::tuple<raytracer::IObject*, raytracer::Type, const raytracer::Face*>> _hits;
        raytracer::Type _distance; // Distance traveled

    public:
        // ---------- Pre-Function -------- //
        void parse(const libconfig::Setting& node) final;
        void computeObjects(raytracer::Type renderDistance, const std::vector<raytracer::IObject*>& objects, const std::unordered_map<raytracer::Chunk, std::vector<raytracer::IObject*>, raytracer::ChunkHash>& objectsChunks); // Call on init & each direction changement

        // ------------ Function ---------- //
        nodiscard const std::vector<std::tuple<raytracer::IObject*, raytracer::Type, const raytracer::Face*>>& getHits(void) const final {return this->_hits;};
        void kill(void) final {this->_alive = false;};
        nodiscard bool isAlive(void) const final {return this->_alive;};
        void addDistance(raytracer::Type distance) final {this->_distance += distance;};
        nodiscard raytracer::Type getDistance(void) const final {return this->_distance;};

        // ------------ Operator ---------- //
        ARay& operator=(const ARay& object) = delete;
        ARay& operator=(ARay&& object) = delete;

        // ---------- Constructor --------- //
        ARay() = default;
        ARay(const ARay& object) = delete;
        ARay(ARay&& object) = delete;

        // ----------- Destructor --------- //
        ~ARay() = default;
};

} // namespace end
#endif /* ARAY_H */
