#ifndef PARTICLES_H
#define PARTICLES_H

#include <entt/entity/fwd.hpp>
#include <magique/internal/InternalTypes.h>
#include <magique/core/Types.h>

//-----------------------------------------------
// Particle Module
//-----------------------------------------------
// .....................................................................
// This module is for creating particle effects. Its interface is inspired by Godot4
// You create a emitter first (either Entity or Screen) and then create the particle effect by calling
// the global Create__ function with that emitter. A emitter can (and should) be reused as often as you like.
// Uses the builder pattner for coincise syntax.
// To begin create a ScreenEmitter emitter; and cusotmize it: emitter.setEmissionPosition(150,150).set...
// .....................................................................

namespace magique
{
    // Renders all active particles
    // Note: Needs to be called manually - so you can control at which layer particles are renderd
    void DrawParticles();

    //----------------- CREATE -----------------//

    // Creates new particle(s) from the given emitter - evokes the emitter "amount" many times
    // IMPORTANT: Passed emitter reference has to outlive all particles created by it! (dont pass stack values)
    void CreateScreenParticle(const ScreenEmitter& emitter, int amount = 1);

    // Adds a entity particle to the ECS
    // IMPORTANT: Passed emitter reference has to outlive all particles created by it! (dont pass stack values)
    entt::entity CreateEntityParticle(const EntityEmitter& emitter, int amount = 1);

    //----------------- EMITTERS -----------------//

    struct EmitterBase
    {
        //----------------- FUNCTIONS -----------------//
        // Note: They are called each tick for each particle!

        // Takes the current scale and the normalized time (ticksAlive/totalLifetime / 0.0 - 1.0)
        // Returns: the new scale of the particle
        using ScaleFunction = float (*)(float scale, float t);

        // Takes the current color and the normalized time
        // Returns: the new color of the particle
        using ColorFunction = Color (*)(const Color& color, float t);

        // Takes the particles itself and the normalized time
        // This allows you to do anything - even call gameplay related code!
        using TickFunction = void (*)(ScreenParticle&, float t);

        //----------------- EMISSION SHAPE -----------------//
        // Note: Emmision shape determines where particles can spawn
        //       Particles can spawn anywhere insdide the emission shape randomly!

        // Sets the position of the emission shape
        // Topleft for rect, middle point for circle, first point for triangle
        // Default: (0,0)
        EmitterBase& setEmissionPosition(float x, float y);

        // Sets the emission shape - ONLY RECT and CIRCLE are supported!
        // Pass the width and height of the rectangle or the radius of the circle OR (0,0,0) to reset
        // Default: None - is directly spawn on the emission point
        EmitterBase& setEmissionShape(Shape shape, float width, float height, float radius = 0.0F);

        //----------------- PARTICLE -----------------//

        // Sets the emission shape to be a rect
        // Pass the width and height of the rectangle
        EmitterBase& setParticleShapeRect(float width, float height);

        // ets the emission shape to be a rect
        // Pass the height and the radius of the capsule
        EmitterBase& setParticleShapeCircle(float radius);

        // Makes the entity collidable with others
        // Pass the offsets for the two remaining points in counter clockwise order - first one is (pos.x, pos.y)
        EmitterBase& setParticleShapeTri(Point p2, Point p3);

        // Sets the color of emitted particles
        // Default: BLACK
        EmitterBase& setColor(const Color& color);

        // Sets the lifetime in millis
        // Default: 1000
        EmitterBase& setLifetime(int val);

        //----------------- ADDITIONALS -----------------//

        // Sets the gravity (pixels/s**2) in both x and y direction
        // Note: Gravity is applied every tick to the particles velocity
        // Default: (0,0)
        EmitterBase& setGravity(float gravityX, float gravityY);

        // Sets the min and max scale of the emitted particle(s) - randomly chosen when created
        // Note: Scaling is applied to the default dimensions of the particle
        // Default: 1
        EmitterBase& setScaling(float minScale, float maxScale);

        // Sets the initial direction vector - uses raylibs coordinate system
        //  - - - - - - -
        // |(0,0)   (1,0)|
        // |             |
        // |      X      |
        // |             |
        // |(0,1)   (1,1)|
        //  - - - - - - -
        // => Straight up (0,-1) => Top Right (0.5,-0.5) => Top Left (-0.5,-0.5) => Bottom Left (-0.5,0.5)
        // Note: The values have to add up to 1!
        // Default: (0,-1)
        EmitterBase& setDirection(float dx, float dy);

        // Sets the angle for spread cone (centered on the direction)
        // A new direction is chosen randomly within the spread angle
        // Default: 0
        EmitterBase& setSpread(float val);

        // Sets the min and max initial velocity in pixels per second - randomly chosen when created
        // Default: 1
        EmitterBase& setInitialVelocity(float minVeloc, float maxVeloc);

        // True: Scales the base dimensions with the resolution (Base resolution: 1920x1080)
        // Default: True
        EmitterBase& setResolutionScaling(bool val);

        //----------------- FUNCTIONS -----------------//

        // Sets the coloring function to dynamically set the particles color depending on the time
        // Default: nullptr
        EmitterBase& setColorFunction(ColorFunction func);

        // Sets the function that determines the scale across its lifetime
        // Default: nullptr
        EmitterBase& setScaleFunction(ScaleFunction func);

        //----------------- HELPERS -----------------//

        // Returns a smoothstep scale function
        static ScaleFunction GetSmoothStep();

    private:
        EmitterData data{};
        friend struct ParticleData;
        friend void CreateScreenParticle(const ScreenEmitter&, int);
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