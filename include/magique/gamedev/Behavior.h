#ifndef MAGIQUE_NPCBEHAVIOR_H
#define MAGIQUE_NPCBEHAVIOR_H

#include <magique/core/Types.h>
#include <entt/entity/fwd.hpp>

//-----------------------------------------------
// Behavior (NPC/AI)
//-----------------------------------------------
// .....................................................................
// Utilities to easily build complex behavior trees
// The structure of trees is not changed after its built
// Through the template you can specify a custom data class that is passed to each condition
// The structure is simple:
// Step 1: Add any branch
// Step 2: React to the branch with onSuccess and (or) onFailure
// Step 3: Add a leaf or restart at step 1
// .....................................................................

namespace magique
{
    struct Leaf final
    {
    };

    template <typename CustomData>
    struct TreeBuilder final
    {
        // Function that operates on component data and returns either true (success) or false (failure)
        using ConditionFunc = bool (*)(entt::registry&, entt::entity self, CustomData& cd);

        template <class... Funcs>
        class Condition final
        {
            int n = 0;
            //            Funcs functions[sizeof...(Funcs)]{};
        };

        struct Branch final
        {
            //explicit Branch(const Condition& condition);

            Branch& onSuccess(Branch& branch);
            Branch& onSuccess(Leaf& branch);

            Branch& onFailure(Branch& branch);
            Branch& onFailure(Leaf& branch);
        };

        //----------------- BRANCH -----------------//

        template <typename T>
        Branch& branch(const Condition<T>& branch);


        //----------------- CONDITIONS -----------------//

        // Adds a if-branch to the tree
        template <typename T>
        Condition<T> If(ConditionFunc func);

        // Adds a anyOf-branch (true if any of the conditions are true) to this tree
        // Checks them in the given order and early returns when possible (same as && operator)
        // Condition< AnyOf(ConditionFunc... functions);

        // Condition NOf(int n, ConditionFunc... functions);


        //----------------- BUILD -----------------//

        void setRoot(const Branch& branch);

        // Returns the built behavior tree
        BehaviorTree<CustomData> build();
    };

} // namespace magique


//----------------- IMPLEMENTATION -----------------//


#endif //MAGIQUE_NPCBEHAVIOR_H