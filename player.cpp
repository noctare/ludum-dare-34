#include "player.hpp"
#include "input.hpp"
#include "audio.hpp"
#include "debug.hpp"
#include "engine.hpp"
#include "window.hpp"
#include "game.hpp"

Player::Player() {
	for (int i = 0; i < 6; i++) {
		EvolutionSprite[i] = Asset::RequestBlock("player size " + std::to_string(i));
	}
	Transform.Position.xy = { 512.0f, 512.0f };
	Asset::Request("evolve", &EvolutionSFX);
	EatTimer.Start();
}

void Player::Update() {

	if (Health < 0.0f) {
		return;
	}

	Sprite = EvolutionSprite[Evolution + HasUpgrade[UPGRADE_PLANE]];

	SpriteAsset* AssetSprite = SpriteAsset::Get(Sprite);
	if (AssetSprite) {
		Transform.Scale.x = (float) AssetSprite->Texture.Size.x / (float) AssetSprite->Properties.Frames;
		Transform.Scale.y = (float) AssetSprite->Texture.Size.y;
	}

	float MinAngleSpeed = 0.05f;
	float MaxAngleSpeed = 0.08f;
	float AngleTurnSpeedDivider = 10.0f;

	if (KeyIsDown(KEY_W)) {
		Transform.Position.x += std::cos(Transform.Rotation.z) * Speed.x;
		Transform.Position.y -= std::sin(Transform.Rotation.z) * Speed.y;
		Acceleration += AccelerationRate;
		if (EatTimer.GetMilliseconds() > 5) {
			GameLoop* Loop = (GameLoop*) GetLoop();
			TextureAsset* WeedTexture = TextureAsset::Get(Loop->WeedTexture);
			vector2f WeedSize = { (float) WeedTexture->Size.x, (float) WeedTexture->Size.y };
			for (auto& CurrentChunk : Loop->ChunksInView) {
				for (auto& CurrentWeed : CurrentChunk->Weeds) {
					if (IsTransformOver(&Transform, CurrentWeed.Position, WeedSize)) {
						Growth += 1;
						EatTimer.Start();
						break;
					}
				}
			}
		}
	}

	if (KeyIsDown(KEY_A)) {
		if (!KeyIsDown(KEY_W)) {
			Transform.Rotation.z += MinAngleSpeed;
			Transform.Position.y += std::sin(-Transform.Rotation.z) * Speed.y / AngleTurnSpeedDivider;
		} else {
			Transform.Rotation.z += MaxAngleSpeed;
		}
		Transform.Position.x += std::cos(-Transform.Rotation.z) * Speed.x / AngleTurnSpeedDivider;
	}

	if (KeyIsDown(KEY_S)) {
		Transform.Rotation.z += MinAngleSpeed;
		Transform.Position.x -= std::cos(Transform.Rotation.z) * 0.01f;
		Transform.Position.y += std::sin(Transform.Rotation.z) * 1.0f;
	}

	if (KeyIsDown(KEY_D)) {
		if (!KeyIsDown(KEY_W)) {
			Transform.Rotation.z -= MinAngleSpeed;
			Transform.Position.y += std::sin(-Transform.Rotation.z) * Speed.y / AngleTurnSpeedDivider;
		} else {
			Transform.Rotation.z -= MaxAngleSpeed;
		}
		Transform.Position.x += std::cos(-Transform.Rotation.z) * Speed.x / AngleTurnSpeedDivider;
	}

	UpdateAcceleration();

	InOil = false;
}

bool Player::CanGrow() {
	int EvolutionAfford = Evolution;
	if (Growth > EVOLUTION_GROWTH_4) {
		if (Evolution < 4) {
			EvolutionAfford = 4;
		}
	} else if (Growth > EVOLUTION_GROWTH_3) {
		if (Evolution < 3) {
			EvolutionAfford = 3;
		}
	} else if (Growth > EVOLUTION_GROWTH_2) {
		if (Evolution < 2) {
			EvolutionAfford = 2;
		}
	} else if (Growth > EVOLUTION_GROWTH_1) {
		if (Evolution < 1) {
			EvolutionAfford = 1;
		}
	}
	return Evolution != EvolutionAfford;
}

int Player::GrowCost() {
	if (Growth > EVOLUTION_GROWTH_4) {
		return EVOLUTION_GROWTH_4;
	} else if (Growth > EVOLUTION_GROWTH_3) {
		return EVOLUTION_GROWTH_3;
	} else if (Growth > EVOLUTION_GROWTH_2) {
		return EVOLUTION_GROWTH_2;
	} else if (Growth > EVOLUTION_GROWTH_1) {
		return EVOLUTION_GROWTH_1;
	}
	return EVOLUTION_GROWTH_4 * 2;
}

void Player::Grow() {
	int EvolutionBefore = Evolution;
	if (Growth > EVOLUTION_GROWTH_4) {
		Evolution = Max(Evolution, 4);
	} else if (Growth > EVOLUTION_GROWTH_3) {
		Evolution = Max(Evolution, 3);
	} else if (Growth > EVOLUTION_GROWTH_2) {
		Evolution = Max(Evolution, 2);
	} else if (Growth > EVOLUTION_GROWTH_1) {
		Evolution = Max(Evolution, 1);
	} else {
		Evolution = Max(Evolution, 0);
	}
	if (Evolution != EvolutionBefore) {
		PlaySound(EvolutionSFX);
		SetSoundVolume(EvolutionSFX, 100);
	}
}

void Player::EquipUpgrade(int Upgrade) {
	if (Upgrade == UPGRADE_CHEW) {
		
	} else if (Upgrade == UPGRADE_SHELL) {

	}
}