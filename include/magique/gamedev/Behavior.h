#ifndef MAGIQUE_NPCBEHAVIOR_H
#define MAGIQUE_NPCBEHAVIOR_H

#include <magique/core/Types.h>
#include <entt/entity/fwd.hpp>

// Utilities to easily build complex behavoir trees
// The structure of trees is not changed after its built
// Through the template you can specify a custom data class that is passed to each condition
// The structure is simple:
// Step 1: Add any branch
// Step 2: React to the branch with onSuccess and (or) onFailure
// Step 3: Add a leaf or restart at step 1

namespace magique
{

    template <typename CustomData = void>
    struct TreeBuilder final
    {
        // Returns the built behavoir tree
        BehavoirTree<CustomData> build();

        // Function that operates on component data and returns either true or false
        using ConditionFunc = bool (*)(entt::registry&, entt::entity self, CustomData& cd);

        // Branches

        // Adds a if-branch to the tree
        // Next: Either "onSuccess" or "onFailure"
        TreeBuilder& ifBranch(ConditionFunc func);

        // Adds a anyOf-branch (true if any of the conditions are true) to this tree
        // Checks them in the given order and early returns when possible (same as &&)
        TreeBuilder& anyOfBranch(ConditionFunc... functions);


        // Leaves

        TreeBuilder& thenDo();


        // Add the next
        TreeBuilder& onSuccess();
        TreeBuilder& onFailure();
    };

} // namespace magique


#endif //MAGIQUE_NPCBEHAVIOR_H