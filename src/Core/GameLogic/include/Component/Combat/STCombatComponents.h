#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

using EntityID = uint32_t;

// 描述当前实体是否可以攻击
struct STAttackComponent {
    float damage = 10.0f;
    float range = 2.0f;
    float cooldown = 1.0f;
    float cooldownTimer = 0.0f;
    bool canAttack = true;
};

// 描述当前实体是否可以蓄力攻击
struct STChargeAttackComponent {
    bool isCharging = false;         // 是否正在蓄力
    float chargeTime = 0.0f;         // 当前蓄力时间
    float maxChargeTime = 2.0f;      // 最大蓄力时间
    float chargeStartTime = 0.0f;    // 开始蓄力的时间（可选，用于计算）
    float baseDamage = 10.0f;        // 基础伤害
    float maxDamage = 30.0f;         // 蓄满后的伤害
    bool hasReleased = false;        // 是否已释放攻击
};

// 描述当前实体是否可以蓄力攻击
struct STSkillComponent {
    int skillID = 0;                 // 技能唯一标识
    float cooldown = 5.0f;           // 技能冷却时间
    float cooldownTimer = 0.0f;      // 当前剩余冷却
    float cost = 20.0f;              // 消耗能量 / 法力
    bool isReady = true;             // 是否可释放
    bool isActive = false;           // 是否正在释放中
};

// 描述当前实体是否有生命属性
struct STHealthComponent {
    float maxHealth = 100.0f;
    float currentHealth = 100.0f;
    bool isAlive = true;
};

// 描述当前实体的锁定目标
struct STCombatTargetComponent {
    EntityID target; 
    bool isLock = false;
};

// 描述当前实体的攻击列表
struct STAttackedListComponent {
    STAttackedListComponent() {
        targets.reserve(10);
    }
    std::vector<EntityID> targets; 
};


// 描述受击状态
struct STHitReactionComponent {
    enum class HitState : uint32_t
    {
        None = 0,           // 0000 0000
        Crit = 1 << 0,      // 0000 0001  --> 暴击
        Fire = 1 << 1,      // 0000 0010  --> 着火
        Ice = 1 << 2,       // 0000 0100  --> 冰冻
        Poison = 1 << 3,    // 0000 1000  --> 中毒
        Stun = 1 << 4,      // 0001 0000  --> 眩晕
    };

    uint32_t hitFlags = 0;
    bool isHit = false;              // 是否刚刚命中
    float hitReactTime = 0.0f;       // 受击动画播放时间
    float hitReactTimer = 0.0f;      // 当前剩余时间
    glm::vec3 knockbackDirection = glm::vec3(1.0f, 0.0f, 0.0f); // 击退方向
    float knockbackForce = 5.0f;     // 击退力度

    void AddFlag(HitState flag) {
        hitFlags |= static_cast<uint32_t>(flag);
    }

    void RemoveFlag(HitState flag) {
        hitFlags &= ~static_cast<uint32_t>(flag);
    }

    bool HasFlag(HitState flag) const {
        return (hitFlags & static_cast<uint32_t>(flag)) != 0;
    }

    void ClearFlags() {
        hitFlags = 0;
    }
};

// 描述当前的动画状态
struct STCombatStateComponent {
    enum class State {
        Idle,
        Attacking,
        Charging,
        Hit,
        Blocking,
        Dodging
    };
    State currentState = State::Idle;
};