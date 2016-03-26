#pragma once

#include "graphics.hpp"
#include "timer.hpp"

class GameLoop;
class Chunk;
class Weed;

class Entity {
public:

	bool HasUpgrade[3];

	int Growth = 0;
	int Evolution = 0;
	float Health = 100.0f;

	bool IsEaten = false;
	bool InOil = false;
	int KilledBy = 0;
	Transformation Transform;
	vector3f Color;
	AssetHandle Sprite = 0;
	SpriteAnimation Animation;

	float Acceleration = 0.0f;
	float AccelerationRate = 0.4f;
	float AccelerationLimit = 9.0f;
	vector2f Speed = { 6.0f, 6.0f };

	void UpdateAcceleration();
	void CheckForOil(Chunk* InChunk);

	Entity();

};

class Enemy : public Entity {
public:

	Weed* SmokinDatWeed = nullptr;
	bool HuntMode = false;

	vector2f Goal;
	Timer GoalTimer;
	int Evolution = 0;
	int TimerGoal = 5000;
	Timer UpdateTimer;
	int GoalDist = 1024;
	int GoalReaction = 3000;
	Enemy* KilledByEnemy = nullptr;
	
	bool IsOutOfBounds = false;

	void FindNextGoal(Chunk* InCuhunk, vector3f Base);

	void Update(GameLoop* Loop, Chunk* InChunk, vector2f ChunkPosition);

	Enemy();

};