#ifndef MAGIQUE_BEHAVIOR_TREE_H
#define MAGIQUE_BEHAVIOR_TREE_H

#include <magique/core/Types.h>
#include <entt/entity/fwd.hpp>

//-----------------------------------------------
// BehaviorTree
//-----------------------------------------------
// .....................................................................
// Utilities to easily build complex behavior trees
// The structure of trees is not changed after it's built
// Through the template you can specify a custom data class that is passed to each condition
// The structure is simple:
// Step 1: Add any branch
// Step 2: React to the branch with onSuccess and (or) onFailure
// Step 3: Add a leaf or restart at step 1
// .....................................................................

namespace magique
{

    template <typename CustomData>
    struct BehaviorTree final
    {
        // Function that operates on component data and returns either true (success) or false (failure)
        using ConditionFunc = bool (*)(entt::registry&, entt::entity self, CustomData& cd);

        struct Branch
        {
            virtual bool condition() = 0;
            virtual void onTrue() = 0;
            virtual void onFalse() = 0;
        };

        template <class TrueFunc, class FalseFunc>
        struct BranchImpl : Branch
        {
            BranchImpl(ConditionFunc condition, TrueFunc onTrue, FalseFunc onFalse) :
                conditionFunc(condition), trueFunc(onTrue), falseFunc(onFalse)
            {

            }

            bool condition() { return conditionFunc(); }

            void onTrue() { trueFunc(); }

            void onFalse() { falseFunc(); }

        private:
            ConditionFunc conditionFunc;
            TrueFunc trueFunc;
            FalseFunc falseFunc;
        };

        void addBranch(Branch* branch)
        {

        }

        void func()
        {
            BehaviorTree tree;
           // tree.addBranch(new BranchImpl([]()));
        }
    };

} // namespace magique

//----------------- IMPLEMENTATION -----------------//


#endif //MAGIQUE_BEHAVIOR_TREE_H