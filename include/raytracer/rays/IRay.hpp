/**************************************************************\
Edition:
##  @date 13/05/2026 by @author Tsukini

File Name:
##  @file IRay.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef IRAY_H
    #define IRAY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../objects/AObject.hpp"   // raytracer::AObject
    #include "../Define.hpp"            // values

namespace raytracer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class IRay: public raytracer::AObject {
    public:
        // ---------- Pre-Function -------- //
        virtual void* clone(void) = 0;
        virtual void reset(void) = 0; // Reset ray values
        virtual void computeObjects(raytracer::Type renderDistance, const std::vector<raytracer::IObject*>& objects, const std::unordered_map<raytracer::Chunk, std::vector<raytracer::IObject*>, raytracer::ChunkHash>& objectsChunks) = 0;
        virtual const std::vector<std::tuple<raytracer::IObject*, raytracer::Type, const raytracer::Face*>>& getHits(void) const = 0;
        virtual void kill(void) = 0;
        virtual bool isAlive(void) const = 0;
        virtual void addDistance(raytracer::Type distance) = 0;
        virtual raytracer::Type getDistance(void) const = 0;

        // ------------ Operator ---------- //
        IRay& operator=(const IRay& object) = delete;
        IRay& operator=(IRay&& object) = delete;

        // ---------- Constructor --------- //
        IRay() = default;
        IRay(const IRay& object) = delete;
        IRay(IRay&& object) = delete;

        // ----------- Destructor --------- //
        virtual ~IRay() = default;
};

} // namespace end
#endif /* IRAY_H */
