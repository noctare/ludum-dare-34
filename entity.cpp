#include "entity.hpp"
#include "game.hpp"

Entity::Entity() {
	HasUpgrade[UPGRADE_CHEW] = false;
	HasUpgrade[UPGRADE_SHELL] = false;
	HasUpgrade[UPGRADE_PLANE] = false;
}

void Entity::UpdateAcceleration() {
	Acceleration -= AccelerationRate / 2.0f;
	if (Acceleration > AccelerationLimit) {
		Acceleration = AccelerationLimit;
		if (HasUpgrade[UPGRADE_SHELL]) {
			Acceleration *= 0.75f;
		}
	}
	if (Acceleration < 0.0f) {
		Acceleration = 0.0f;
	}
	Transform.Position.x += std::cos(Transform.Rotation.z) * Acceleration;
	Transform.Position.y -= std::sin(Transform.Rotation.z) * Acceleration;
	if (InOil) {
		Acceleration = 0.0f;
	}
	Animation.FPS = Max(Acceleration * 2.0f, 4.0f);
}

void Entity::CheckForOil(Chunk* InChunk) {
	for (auto& Oil : InChunk->OilPatches) {
		if (IsTransformOver(&Oil.Transform, Transform.Position.xy, Transform.Scale.xy)) {
			InOil = true;
			Health -= 0.05f;
			break;
		}
	}
}

Enemy::Enemy() {
	UpdateTimer.Start();
}

void Enemy::FindNextGoal(Chunk* InChunk, vector3f Base) {

	HuntMode = true;

	if (Evolution == 0) {
		if (SmokinDatWeed) {
			if (rand() % 100 > 10) {
				return; // Keep smokin' dat weed!
			}
		}
		for (unsigned int i = 0; i < InChunk->Weeds.size(); i++) {
			float Dist = DistanceToPoint(InChunk->Weeds[i].Position.x, InChunk->Weeds[i].Position.y, Transform.Position.x, Transform.Position.y);
			if (Dist < 128.0f) {
				SmokinDatWeed = &InChunk->Weeds[i];
				Goal = InChunk->Weeds[i].Position;
				GoalTimer.Start();
				TimerGoal = 5000 + rand() % GoalReaction;
				return;
			}
		}
	} else if (Evolution > 0) {
		for (unsigned int i = 0; i < InChunk->Enemies.size(); i++) {
			if (InChunk->Enemies[i].Evolution > 0) {
				continue;
			}
			vector2f Position = InChunk->Enemies[i].Transform.Position.xy;
			float Dist = DistanceToPoint(Position.x, Position.y, Transform.Position.x, Transform.Position.y);
			if (Dist < 128.0f) {
				Goal = Position;
				GoalTimer.Start();
				TimerGoal = 2000 + rand() % GoalReaction;
				return;
			}
		}
	}

	Goal = { 
		Base.x - (float) GoalDist + rand() % (GoalDist * 2), 
		Base.y - (float) GoalDist + rand() % (GoalDist * 2) 
	};
	GameLoop* Loop = (GameLoop*) GetLoop();
	if (Loop->InsideOilPatch(Goal)) {
		FindNextGoal(InChunk, Base);
		return;
	}
	GoalTimer.Start();
	TimerGoal = 3000 + rand() % GoalReaction;
}

void Enemy::Update(GameLoop* Loop, Chunk* InChunk, vector2f ChunkPosition) {

	CheckForOil(InChunk);

	Sprite = Loop->EnemySize[Evolution];
	SpriteAsset* AssetSprite = SpriteAsset::Get(Sprite);
	if (AssetSprite) {
		Transform.Scale.x = (float) AssetSprite->Texture.Size.x / (float) AssetSprite->Properties.Frames;
		Transform.Scale.y = (float) AssetSprite->Texture.Size.y;
		Transform.Scale.x += Growth;
		Transform.Scale.y += Growth;
	}

	if (IsTransformOver(&Loop->MainPlayer->Transform, &Transform)) {
		if (Evolution == 0) {
			if (Loop->MainPlayer->Acceleration > 4.0f && Loop->MainPlayer->Evolution > 0) {
				IsEaten = true;
				KilledBy = 0;
				Loop->BloodParticles.push_back(BloodParticle(Transform.Position.x, Transform.Position.y));
			}
		} else if (Evolution > 0) {
			if (Loop->MainPlayer->Evolution > 2) {
				if (HuntMode) {
					if (Loop->MainPlayer->HasUpgrade[UPGRADE_SHELL]) {
						Loop->MainPlayer->Health -= 0.05f;
					} else {
						Loop->MainPlayer->Health -= 0.4f;
					}
				}
				if (Loop->MainPlayer->HasUpgrade[UPGRADE_CHEW]) {
					Health -= Loop->MainPlayer->Acceleration / 2.0f;
				} else {
					Health -= Loop->MainPlayer->Acceleration / 6.0f;
				}
				if (Health < 0.0f) {
					IsEaten = true;
					KilledBy = 0;
				}
				if (Loop->MainPlayer->Acceleration > 4.0f || HuntMode) {
					Loop->BloodParticles.push_back(BloodParticle(Transform.Position.x, Transform.Position.y));
				}
			} else {
				Health -= Loop->MainPlayer->Acceleration / 3.0f;
				Loop->MainPlayer->Health -= 0.5f;
				if (UpdateTimer.GetMilliseconds() % 6 == 0 && (Loop->MainPlayer->Acceleration > 4.0f || Acceleration > 4.0f)) {
					Loop->BloodParticles.push_back(BloodParticle(Loop->MainPlayer->Transform.Position.x, Loop->MainPlayer->Transform.Position.y));
				}
			}
		}
		return;
	}

	if (UpdateTimer.GetMilliseconds() > 12 && Evolution == 0) {
		for (auto& OtherEnemy : InChunk->Enemies) {
			if (OtherEnemy.HuntMode 
				&& OtherEnemy.Evolution > 0 
				&& IsTransformOver(&Transform, OtherEnemy.Transform.Position.xy, OtherEnemy.Transform.Scale.xy)) 
			{
				IsEaten = true;
				KilledBy = 1;
				KilledByEnemy = &OtherEnemy;
				Loop->BloodParticles.push_back(BloodParticle(Transform.Position.x, Transform.Position.y));
				return;
			}
		}
		UpdateTimer.Start();
	}

	if (GoalTimer.GetMilliseconds() > TimerGoal) {
		FindNextGoal(InChunk, Loop->MainPlayer->Transform.Position);
	}

	int DeltaX = (int) Goal.x - Transform.Scale.x / 2.0f;
	int DeltaY = (int) Goal.y - Transform.Scale.y / 2.0f;

	if (DistanceToPoint(DeltaX, DeltaY, Transform.Position.x, Transform.Position.y) < 128.0f) {
		HuntMode = false;
	}

	float LerpSpeed = 0.0075f;
	if (Evolution > 0) {
		LerpSpeed = 0.0065f;
	}
	Transform.Position.x += ((float) DeltaX - Transform.Position.x) * LerpSpeed;
	Transform.Position.y += ((float) DeltaY - Transform.Position.y) * LerpSpeed;

	Transform.Rotation.z = DegToRad(AngleToTargetPosition(Transform.Position.xy, Goal));

//	UpdateAcceleration();

	IsOutOfBounds = !IsTransformOver(&Transform, ChunkPosition, vector2f(InChunk->Tiles->Texture.Size.x, InChunk->Tiles->Texture.Size.y));
	
	InOil = false;
}