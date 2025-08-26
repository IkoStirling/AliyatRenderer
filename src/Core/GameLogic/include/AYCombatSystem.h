#pragma once
#include "AYECSEngine.h"
#include "Mod_CombatSystem.h"

class AYCombatSystem : public Mod_CombatSystem
{
public:
	AYCombatSystem();
	/*
	 * 1、基础攻击 STAttackComponent
	 * 2、蓄力攻击 STChargeAttackComponent
	 * 3、技能攻击 STSkillComponent
	 * 4、受击状态 STHitReactionComponent
	 * 5、行为状态 STCombatStateComponent
	 * 
	*/
	void update(float delta_time) override;
	void init() override {}
	void shutdown() override {}
};

REGISTER_MODULE_CLASS("CombatSystem", AYCombatSystem);