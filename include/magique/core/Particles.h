// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PARTICLES_H
#define MAGIQUE_PARTICLES_H

#include <initializer_list>
#include <entt/entity/fwd.hpp>
#include <magique/internal/InternalTypes.h>
#include <functional>

//===============================================
// Particle Module
//===============================================
// .....................................................................
// This module is for creating particle effects. The interface is inspired by Godot4's CPUParticle2D node.
// You create an emitter first (either Entity or Screen) and then create the particle effect by calling
// the global Create() function with that emitter. An emitter can (and should) be reused as often as you like.
// Note: Changing the emitter doesn't change already spawned particles (except the tick functions)
// Uses the builder pattern for syntactic sugar.
// To begin create a ScreenEmitter emitter; and customize it: emitter.setEmissionPosition(150,150).set...
// Can currently handle well up to 250'000 screen particles at the same time on modern systems
// .....................................................................

namespace magique
{
    // Renders all active particles
    // Note: Needs to be called manually - so you can control at which layer particles are rendered
    void ParticlesDraw();

    // Allows to set a global modifier to the amount of particles created
    // Useful cause of single point of truth - modifies all calls
    // Default: 1.0
    float ParticlesGetAmountScale();
    void ParticlesSetAmountScale(float amount);

    //================= CREATE =================//

    // Creates new particle(s) from the given emitter - evokes the emitter "amount" many times
    // IMPORTANT: Passed emitter reference has to outlive all particles created by it! (don't pass stack values)
    // Sets the position of the emission shape - top left for rect, middle point for circle
    void CreateScreenParticle(const ScreenEmitter& emitter, const Point& position = {0, 0}, int amount = 1);

    // Adds an entity particle to the ECS
    // IMPORTANT: Passed emitter reference has to outlive all particles created by it! (don't pass stack values)
    // Sets the position of the emission shape - top left for rect, middle point for circle
    entt::entity CreateEntityParticle(const EntityEmitter& emitter, const Point& position = {0, 0}, int amount = 1);

    //================= EMITTERS =================//

    struct EmitterBase
    {
        //================= FUNCTIONS =================//
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

        //================= EMISSION SHAPE =================//
        // Note: Emission shape determines where particles can spawn
        //       Particles can spawn anywhere inside the emission shape randomly!

        // Sets the emission shape - ONLY RECT and CIRCLE are supported!
        // Pass the width and height of the rectangle or the radius of the circle OR (0,0,0) to reset
        // A random point inside the emission shape is chosen for each particle
        // Default: None - is directly spawned on the emission point
        EmitterBase& setEmissionShape(Shape shape, float width, float height, float radius = 0.0F);

        // Sets the angle of rotation - rotates around the anchor point
        // Note: only works for RECT - angle starts at 12 and clockwise until 360 degrees
        // Default: 0
        EmitterBase& setEmissionRotation(int angle);

        // Sets the anchor point around which the rotation occurs
        // Default: (0,0)
        EmitterBase& setEmissionRotationAnchor(const Point& anchor);

        // How much of the collision shape will be treated as spawnable area - 0 => only the outlines 1.0 full body
        // Grows from the outside inwards - for the other way around just make the shape smaller
        // Default: 1.0
        EmitterBase& setEmissionShapeVolume(float percent);

        //================= PARTICLE =================//
        // Note: Default shape is Rect with dimensions (5,5)

        // Sets the emission shape to be a rect
        EmitterBase& setParticleShapeRect(float width, float height);

        // Sets the emission shape to be a circle
        EmitterBase& setParticleShapeCircle(float radius);

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

        //================= ADDITIONALS =================//

        // Sets an angular (to the circle center inwards) gravity - replaces normal gravity
        // in pixels per second**2
        // Note: if set will switch particle to angular mode
        EmitterBase& setAngularGravity(float gravity);

        // Sets an angular velocity (speed on the circle outline) - replaces normal velocity
        // X is the speed along the circle outline - positive is right-around
        // Y is the speed component away or towards the center - positive is towards center
        // Note: Uses the base velocity for scaling - both in pixels per second
        // Note: if set will switch particle to angular mode
        EmitterBase& setAngularVelocity(const Point& velocity);

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
        // Note: The (absolute) values have to add up to 1 (or 0)!
        // Default: (0,-1)
        EmitterBase& setDirection(const Point& direction);

        // Sets the angle for spread cone (centered on the direction)
        // A new direction is chosen randomly within the spread angle
        // Default: 0
        EmitterBase& setSpread(float val);

        // Sets the min and max initial velocity in pixels per second - randomly chosen when created
        // Default: 1
        EmitterBase& setVelocityRange(float minVeloc, float maxVeloc = 0);

        // True: Scales the base dimensions with the resolution (Base resolution: 1920x1080)
        // Default: True
        EmitterBase& setResolutionScaling(bool val);

        //================= FUNCTIONS =================//

        // Sets the coloring function to dynamically set the particles color depending on the time
        // Default: nullptr
        EmitterBase& setColorFunction(ColorFunction func);

        // Sets the function that determines the scale across its lifetime
        // Default: nullptr
        EmitterBase& setScaleFunction(ScaleFunction func);

        // Sets an arbitrary tick function that is also called every tick for the particles lifetime
        // Default: nullptr
        EmitterBase& setTickFunction(const TickFunction& func);

        //================= HELPERS =================//

        // Returns a smooth step scale function
        static ScaleFunction GetSmoothStep();

        // Returns
        const internal::EmitterData& getData() const;

    private:
        internal::EmitterData data{};
        friend struct ParticleData;
        friend void CreateScreenParticle(const ScreenEmitter&, const Point&, int);
        friend TickFunction;
    };


    // A simple and faster particle that doesn't interact with anything
    struct ScreenEmitter final : EmitterBase
    {
    };

    // A more complex but slower particle that lives inside the ECS
    // Can be made to react to collisions or physics with other gameobjects
    struct EntityEmitter final : EmitterBase
    {
    };

} // namespace magique


//================= IMPLEMENTATION =================//

namespace magique
{

} // namespace magique
// namespace magique
#endif //MAGIQUE_PARTICLES_H