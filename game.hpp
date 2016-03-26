#pragma once

#include "engine.hpp"
#include "math.hpp"
#include "graphics.hpp"
#include "asset.hpp"
#include "camera.hpp"
#include "platform.hpp"
#include "window.hpp"
#include "memory.hpp"
#include "debug.hpp"

#include "player.hpp"
#include "entity.hpp"

class OilPatch {
public:
	Transformation Transform;
	AssetHandle Texture = 0;
	OilPatch(float Rate, vector2f Position, vector2i Scale, AssetHandle OilTexture) {
		Transform.Position.xy = { Position };
		Transform.Scale.xy = { (float) Scale.x, (float) Scale.y };
		Texture = OilTexture;
	}
};

class Weed {
public:
	vector2f Position;
};

class TileMap {
public:
	TextureAsset Texture;
};

class Chunk {
public:
	vector2f Position;
	TileMap* Tiles;
	std::vector<Enemy> Enemies;
	std::vector<OilPatch> OilPatches;
	std::vector<Weed> Weeds;
	Chunk(GameLoop* Loop, TileMap* Map, vector2i Position);
};

class BloodParticle {
public:
	vector2f Position;
	SpriteAnimation Animation;
	BloodParticle(float X, float Y) {
		Position = { X, Y };
		Animation.FPS = 3.0f;
	}
};

#define UPGRADE_CHEW	0
#define UPGRADE_SHELL	1
#define UPGRADE_PLANE	2

#define UPGRADE_CHEW_COST	1200
#define UPGRADE_SHELL_COST	2000
#define UPGRADE_PLANE_COST	4000

class UpgradeItem {
public:
	AssetHandle Icon = 0;
	int Cost = 0;
	FontText Description;
	FontText PriceInfo;
	int Evolution = 3;
};

class GameLoop : public AbstractLoop {
public:
	AssetHandle UIOverlay = 0;

	FontText YouCanUpgrade;
	FontText YouCanUpgradeHelp;
	AssetHandle EvoIcon[4];
	UpgradeItem UpgradeItems[3];

	Timer EeryTimer;

	AssetHandle MouthOverride[5];
	AssetHandle ShellOverride[5];

	FontText LevelInfo;

	int FishFrequency = 10;
	int MusicVolume = 40;

	static const int TilesPerRow = 32;
	const float TileSize = 96.0f;
	const int TileTypeCount = 4;
	const int TotalTilesInMap = TilesPerRow * TilesPerRow;
	const int UniqueTileMaps = 8;

	Timer RespawnTimer;
	bool Respawning = false;

	DisplayContext Context;
	OrthoCamera Ortho;

	AssetHandle BasicShader = 0;
	AssetHandle Tahoma = 0;

	AssetHandle WildlyGrowing = 0;
	AssetHandle EatSFX[6];
	AssetHandle EerySFX[8];

	AssetHandle TilesTexture = 0;
	AssetHandle Decorations[10];
	AssetHandle EnemySize[3];
	AssetHandle OverlayFog = 0;
	AssetHandle OilPatchTextures [2];
	AssetHandle BloodSprite = 0;
	AssetHandle WeedTexture = 0;

	Player* MainPlayer;
	FontText FPSText;
	FontText PlayerInfo;
	
	struct {
		AssetHandle HealthTexture = 0;
		FontText FoodText;
	} HUD;

	std::vector<TileMap> ReferenceMaps;
	std::vector<Chunk> Chunks;
	std::vector<Enemy*> EnemiesToDraw;
	std::vector<OilPatch*> OilPatchesToDraw;
	std::vector<Chunk*> ChunksInView;

	std::vector<BloodParticle> BloodParticles;

	void Start();
	void Stop();
	void Update();
	void Display();

	void FindChunkForEnemy(int ChunkIndex, int EnemyIndex);

	void BuyUpgrade();
	void BuyEvolution();

	bool InsideOilPatch(vector2f Position);

};