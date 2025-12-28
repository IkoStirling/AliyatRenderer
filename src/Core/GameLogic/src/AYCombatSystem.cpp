#include "Component/Combat/STCombatComponents.h"
#include "AYCombatSystem.h"

namespace ayt::engine::game
{
    using namespace ::ayt::engine::ecs;

    AYCombatSystem::AYCombatSystem()
    {
    }

    void AYCombatSystem::update(float delta_time)
    {
        auto ecs = GET_CAST_MODULE(ECS, "ECSEngine");

        // ======================
        // 1. 更新基础攻击冷却（STAttackComponent）
        // ======================
        for (auto [entity, attackComp] : ecs->getView<STAttackComponent>()) {
            if (attackComp.cooldownTimer > 0.0f) {
                attackComp.cooldownTimer -= delta_time;
                if (attackComp.cooldownTimer < 0.0f)
                {
                    attackComp.cooldownTimer = 0.0f;
                    attackComp.canAttack = true;
                }
            }
        }

        // ======================
        // 2. 更新蓄力攻击（STChargeAttackComponent）
        // ======================
        for (auto [entity, charge] : ecs->getView<STChargeAttackComponent>()) {
            if (charge.isCharging && !charge.hasReleased) {
                charge.chargeTime += delta_time;

                // 限制最大蓄力时间
                if (charge.chargeTime > charge.maxChargeTime) {
                    charge.chargeTime = charge.maxChargeTime;
                }
            }
        }

        // ======================
        // 3. 更新技能冷却（STSkillComponent）
        // ======================
        for (auto [entity, skill] : ecs->getView<STSkillComponent>()) {
            if (!skill.isReady) {
                skill.cooldownTimer -= delta_time;
                if (skill.cooldownTimer <= 0.0f) {
                    skill.cooldownTimer = 0.0f;
                    skill.isReady = true;
                    std::cout << "[CombatSystem] Skill " << skill.skillID << " is ready!" << std::endl;
                }
            }
        }

        // ======================
        // 4. 更新受击反应（STHitReactionComponent）
        // ======================
        for (auto [entity, hit] : ecs->getView<STHitReactionComponent>()) {
            if (hit.hitReactTimer > 0.0f) {
                hit.hitReactTimer -= delta_time;
                if (hit.hitReactTimer <= 0.0f) {
                    hit.hitReactTimer = 0.0f;
                    hit.isHit = false;  // 可选：受击状态结束
                    hit.ClearFlags();   // 清除所有 bitmask 状态（如暴击、燃烧等）
                    std::cout << "[CombatSystem] Hit reaction finished for entity " << entity << std::endl;
                }
            }
        }
    }
}