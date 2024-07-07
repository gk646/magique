#ifndef PARTICLES_H
#define PARTICLES_H

#include <entt/entity/fwd.hpp>
#include <magique/internal/Types.h>

//-----------------------------------------------
// Particle Module
//-----------------------------------------------
// .....................................................................
// This module is for creating particle effects.
// You create a emitter first (either Entity or Screen) and then create the particle effect by calling
// the global Create__ function with that emitter.
// A emitter can be reused as often as you like
// .....................................................................

namespace magique
{
    //----------------- CREATE -----------------//

    // Adds a particle
    void CreateScreenParticle(const ScreenEmitter& emitter);

    // Adds a entity particle to the ECS
    entt::entity CreateEntityParticle(const EntityEmitter& emitter);

    //----------------- EMITTERS -----------------//

    struct EmitterBase
    {
        // Takes the current scale and the normalized time (0-1)
        // Returns: the new scale of the particle
        using ScaleFunction = float (*)(float scale, float t);

        // Takes the current color and the normalized time
        // Returns: the new color of the particle
        using ColorFunction = float (*)(Color& color, float t);

        //----------------- SETTERS -----------------//

        // Sets the position of the emission shape
        // Default: (0,0)
        EmitterBase& setEmissionPosition(float x, float y);

        // Sets the amount of particles to emit
        // Default: 1
        EmitterBase& setAmount(int amount);

        // Sets the color of emitted particles
        // Default: BLACK
        EmitterBase& setColor(const Color& color);

        // Sets the coloring function to dynamically set the particles color depending on the time
        // Default: nullptr
        EmitterBase& setColorFunction(ColorFunction func);

        // Sets the shape of the emitted particles
        // Default: Shape::RECT
        EmitterBase& setShape(Shape shape);

        // Sets the base dimensions of the emitted particles
        // Default: (10,10)
        EmitterBase& setDimensions(float w, float h);

        // Sets the lifetime in millis
        // Default: 1000
        EmitterBase& setLifetime(int val);

        // Sets the scale of the emitted particle(s)
        // Default: 1
        EmitterBase& setMinScale(float val);

        // Sets the maximum scale of the emitted particle(s)
        // Value is randomly picked between min and max
        // Default: 1
        EmitterBase& setMaxScale(float val);

        // Sets the function that determines the scale across its lifetime
        // Default: nullptr
        EmitterBase& setScaleFunction(ScaleFunction func);

        // Sets the direction vector - uses raylibs coordinate system
        //  - - - - - - -
        // |(0,0)  (1,0)|
        // |            |
        // |            |
        // |            |
        // |(0,1)  (1,1)|
        //  - - - - - - -
        // => Straight up (0,-1) - => Top right (0.5,-0.5)
        // Default: (0,0)
        EmitterBase& setDirection(float x, float y);

        // Sets the angle for spread cone (centered on the direction)
        // A new direction is chosen randomly within the spread angle
        // Default: 0
        EmitterBase& setSpread(float val);

        // Sets the minimal initial velocity in pixels per second
        // Default: 1
        EmitterBase& setMinInitialVelocity(float val);

        // Sets the maximal initial velocity in pixels per second
        // Value is randomly picked between min and max
        // Default: 1
        EmitterBase& setMaxInitialVelocity(float val);

        // Sets the shape of the emission position
        // Default: CIRCLE
        EmitterBase& setEmissionShape(Shape val);

        // Sets the dimensions of the emission shape
        // Particle position is randomly picked from insdie the emission shape
        // Default: (1,1)
        EmitterBase& setEmissionShapeDimensions(float w, float h);

        // True: Scales the base dimensions with the resolution (Base resolution: 1920x1080)
        // Default: True
        EmitterBase& setResolutionScaling(bool val);

        //----------------- HELPERS -----------------//

        // Returns a smoothstep scale function
        static ScaleFunction GetSmoothStep();

    private:
        EmitterData data{};
    };

    // A simple and faster particle that doesnt interact with anything
    struct ScreenEmitter final : EmitterBase
    {
    };

    // A more complex but slower particle that lives inside the ECS
    // Can be made to react to collisions or physics with other gameobjects
    struct EntityEmitter final : EmitterBase
    {
    };

} // namespace magique
#endif //PARTICLES_H