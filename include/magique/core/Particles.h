#ifndef MAGIQUE_PARTICLES_H
#define MAGIQUE_PARTICLES_H

#include <initializer_list>
#include <entt/entity/fwd.hpp>
#include <magique/internal/InternalTypes.h>

//-----------------------------------------------
// Particle Module
//-----------------------------------------------
// .....................................................................
// This module is for creating particle effects. The interface is inspired by Godot4's CPUParticle2D node.
// You create a emitter first (either Entity or Screen) and then create the particle effect by calling
// the global Create() function with that emitter. An emitter can (and should) be reused as often as you like.
// Note: Changing the emitter doesnt change already spawned particles (except the tick functions)
// Uses the builder pattern for syntactic sugar.
// To begin create a ScreenEmitter emitter; and cusotmize it: emitter.setEmissionPosition(150,150).set...
// Can currently handle well up to 250'000 screen particles at the same time on modern systems
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
        // This allows you to do anything - call gameplay related code and create new particles!
        using TickFunction = std::function<void(ScreenParticle& p, float t)>;

        //----------------- EMISSION SHAPE -----------------//
        // Note: Emmision shape determines where particles can spawn
        //       Particles can spawn anywhere insdide the emission shape randomly!

        // Sets the position of the emission shape
        // Topleft for rect, middle point for circle, first point for triangle
        // Default: (0,0)
        EmitterBase& setEmissionPosition(float x, float y);

        // Sets the emission shape - ONLY RECT and CIRCLE are supported!
        // Pass the width and height of the rectangle or the radius of the circle OR (0,0,0) to reset
        // A random point inside the emission shape is chosen for each particle
        // Default: None - is directly spawned on the emission point
        EmitterBase& setEmissionShape(Shape shape, float width, float height, float radius = 0.0F);

        //----------------- PARTICLE -----------------//
        // Note: Default shape is Rect with dimensions (5,5)

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
        // Default: RED
        EmitterBase& setColor(const Color& color);

        // Sets the pool of colors from which one is randomly chosen - up to 5 colors
        // If set will override the value of setColor() - using setColor() after will override the pool!
        // Default: empty
        EmitterBase& setColorPool(const std::initializer_list<Color>& colors);

        // Sets the lifetime in game ticks - randomly chosen between the min and max
        // Default: 100
        EmitterBase& setLifetime(int minLife, int maxLife = 0);

        //----------------- ADDITIONALS -----------------//

        // Sets the gravity (pixels/s**2) in both x and y direction
        // Note: Gravity is applied every tick to the particles velocity
        // Default: (0,0)
        EmitterBase& setGravity(float gravityX, float gravityY);

        // Sets the min and max scale of the emitted particle(s) - randomly chosen when created
        // Note: Scaling is applied to the default dimensions of the particle
        // Default: 1
        EmitterBase& setScale(float minScale, float maxScale = 0);

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
        EmitterBase& setVelocity(float minVeloc, float maxVeloc = 0);

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

        // Sets a arbitrary tick function that is also called every tick for the particles lifetime
        // Default: nullptr
        EmitterBase& setTickFunction(const TickFunction& func);

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


//----------------- IMPLEMENTATION -----------------//

namespace magique
{

} // namespace magique
// namespace magique
#endif //MAGIQUE_PARTICLES_H