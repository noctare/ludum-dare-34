#pragma once

#include "asset.hpp"
#include "graphics.hpp"
#include "entity.hpp"
#include "timer.hpp"

#define EVOLUTION_GROWTH_1		100
#define EVOLUTION_GROWTH_2		1000
#define EVOLUTION_GROWTH_3		2000
#define EVOLUTION_GROWTH_4		4000

class Player : public Entity {
public:

	AssetHandle EvolutionSFX = 0;
	AssetHandle EvolutionSprite[6];

	Timer EatTimer;

	Player();

	void Update();
	void Grow();
	int GrowCost();
	bool CanGrow();
	void EquipUpgrade(int Upgrade);

};