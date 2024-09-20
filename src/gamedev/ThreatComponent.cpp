#include <cfloat>
#include <magique/gamedev/components/ThreatComponent.h>

namespace magique
{

    ThreatC::ThreatC(int aggroRange, int chaseRange) {}

    void ThreatC::addThreat(const entt::entity target, const float threat)
    {
        if (threat == 0.0F) // No change occurs
            return;

        if (threat > 0.0F)
        {
            for (auto& entry : table)
            {
                if (entry.target == target)
                {
                    entry.threat += threat;
                    return;
                }
                if (entry.threat == FLT_MIN)
                {
                    entry.target = target;
                    entry.threat = threat;
                    ++targets;
                    return;
                }
            }
        }
        else
        {
            for (auto& entry : table)
            {
                if (entry.target == target)
                {
                    entry.threat += threat;
                    if (entry.threat < 0.0F)
                    {
                        entry.threat = FLT_MIN;
                        MAGIQUE_ASSERT(targets >= 1, "Target count mismatch");
                        --targets;
                    }
                    return;
                }
            }
        }
    }
    bool ThreatC::getIsTableFull() const { return targets == THREAT_TABLE_SIZE; }
} // namespace magique