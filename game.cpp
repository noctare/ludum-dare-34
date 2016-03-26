#include "game.hpp"
#include "audio.hpp"
#include "input.hpp"

Chunk::Chunk(GameLoop* Loop, TileMap* Map, vector2i Pos) {
	Position = { (float) Pos.x, (float) Pos.y };
	Tiles = Map;
	int EnemyCount = rand() % Loop->FishFrequency;
	for (int i = 0; i < EnemyCount; i++) {
		Enemies.push_back(Enemy());
		int RandEvo = rand() % 100;
		if (RandEvo > 94) {
			RandEvo = rand() % 100;
			if (RandEvo > 25) {
				Enemies[i].Evolution = 1;
				Enemies[i].GoalDist = 2200;
				Enemies[i].GoalReaction = 3000;
			} else {
				Enemies[i].Evolution = 2;
				Enemies[i].GoalDist = 800;
				Enemies[i].GoalReaction = 500;
			}
		} else {
			Enemies[i].Evolution = 0;
			Enemies[i].GoalDist = 1200;
			Enemies[i].GoalReaction = 2000;
		}
		Enemies[i].FindNextGoal(this, Loop->MainPlayer->Transform.Position);
		Enemies[i].Transform.Position.xy = { 
			(float) (i / 30) * 200.0f + rand() % 400, 
			(float) (i % 50) * 50.0f + rand() % 200 
		};
		Enemies[i].Transform.Position.x += Position.x;
		Enemies[i].Transform.Position.y += Position.y;
		float Off1 = (float) (rand() % 50);
		float Off2 = (float) (rand() % 50);
		float Off3 = (float) (rand() % 50);
		Enemies[i].Color = {
			(256.0f - Off1) / (float) (Enemies[i].Transform.Position.x / i), 
			(256.0f - Off2) / (float) (Enemies[i].Transform.Position.y / Enemies[i].Transform.Position.x + i), 
			(256.0f - Off3) / (float) (Enemies[i].Transform.Position.y / i)
		};
	}
	int WeedCount = 3 + rand() % 30;
	for (int i = 0; i < WeedCount; i++) {
		Weeds.push_back(Weed());
		Weeds[i].Position = {
			(float) (i / 30) * 200.0f + rand() % 400,
			(float) (i % 50) * 50.0f + rand() % 200
		};
		Weeds[i].Position.x += Position.x;
		Weeds[i].Position.y += Position.y;
	}
}

void RenderTile(TextureAsset* Texture, uint32* Pixels, vector2i Position, int TileIndex, vector2i Offset, vector2i TileSize) {
	const int MapWidth = 96 * GameLoop::TilesPerRow;
	const int TileStartX = TileSize.x * TileIndex;
	const int IndexPosX = TileSize.x * Position.x;
	const int IndexPosY = TileSize.y * Position.y;
	for (int TileY = 0; TileY < TileSize.y; TileY++) {
		for (int TileX = 0; TileX < TileSize.x; TileX++) {
			uint32 Value = Texture->Pixels[(Texture->Size.x * TileY) + TileStartX + TileX];
			if (((Value & 0xFF000000) >> 24) == 0x00000000) {
				continue;
			}
			int Index = (IndexPosY + (TileY + Offset.y)) * MapWidth + IndexPosX + (TileX + Offset.x);
			Pixels[Index] = Value;
		}
	}
}

void GameLoop::FindChunkForEnemy(int ChunkIndex, int EnemyIndex) {
	Transformation MapTrans;
	MapTrans.Scale.xy = { TileSize * (float) TilesPerRow, TileSize * (float) TilesPerRow };
	int MapsPerRow = std::sqrt(Chunks.size());
	for (int i = 0; i < Chunks.size(); i++) {
		if (i != 0) {
			if (i % MapsPerRow == 0) {
				MapTrans.Position.x = 0.0f;
				MapTrans.Position.y += MapTrans.Scale.y;
			} else {
				MapTrans.Position.x += MapTrans.Scale.x;
			}
		}
		if (i == ChunkIndex) {
			continue;
		}
		if (IsTransformOver(&MapTrans, Chunks[ChunkIndex].Enemies[EnemyIndex].Transform.Position.xy, Chunks[ChunkIndex].Enemies[EnemyIndex].Transform.Scale.xy)) {
			Chunks[i].Enemies.push_back(Chunks[ChunkIndex].Enemies[EnemyIndex]);
			Chunks[ChunkIndex].Enemies.erase(Chunks[ChunkIndex].Enemies.begin() + EnemyIndex);
			return;
		}
	}
}

bool GameLoop::InsideOilPatch(vector2f Position) {
	for (auto& Oil : OilPatchesToDraw) {
		if (IsTransformOver(&Oil->Transform, Position)) {
			return true;
		}
	}
	return false;
}

void GameLoop::Start() {

	std::string Config = ReadFile("fish.txt");
	FishFrequency = std::stoi(Config);

	Config = ReadFile("audio.txt");
	MusicVolume = std::stoi(Config);

	UpgradeItems[UPGRADE_CHEW].Cost = UPGRADE_CHEW_COST;
	UpgradeItems[UPGRADE_SHELL].Cost = UPGRADE_SHELL_COST;
	UpgradeItems[UPGRADE_PLANE].Cost = UPGRADE_PLANE_COST;
	UpgradeItems[UPGRADE_PLANE].Evolution = 4;

	UpgradeItems[UPGRADE_CHEW].Description.Text = "Chew Like A Pro!";
	UpgradeItems[UPGRADE_CHEW].PriceInfo.Text = std::to_string(UpgradeItems[UPGRADE_CHEW].Cost) + " Food & Lev. 3";
	UpgradeItems[UPGRADE_SHELL].Description.Text = "MLG Shell Defense!";
	UpgradeItems[UPGRADE_SHELL].PriceInfo.Text = std::to_string(UpgradeItems[UPGRADE_SHELL].Cost) + " Food & Lev. 3";
	UpgradeItems[UPGRADE_PLANE].Description.Text = "Airplane Mode!";
	UpgradeItems[UPGRADE_PLANE].PriceInfo.Text = std::to_string(UpgradeItems[UPGRADE_PLANE].Cost) + " Food & Lev. 4";

	Context.SetCameraReference(0, &Ortho.ViewMatrix, &Ortho.ProjectionMatrix);
	Ortho.Scale = { (float) GetMainWindowWidth(), (float) GetMainWindowHeight() };
	Asset::Request("basic", &BasicShader);
	Asset::Request("tahoma", &Tahoma);
	Asset::Request("wildly growing", &WildlyGrowing);
	Asset::Request("fog", &OverlayFog);
	Asset::Request("health", &HUD.HealthTexture);
	Asset::Request("blood", &BloodSprite);
	Asset::Request("weed", &WeedTexture);
	Asset::Request("chew", &UpgradeItems[UPGRADE_CHEW].Icon);
	Asset::Request("shell", &UpgradeItems[UPGRADE_SHELL].Icon);
	Asset::Request("plane", &UpgradeItems[UPGRADE_PLANE].Icon);
	Asset::Request("ui overlay", &UIOverlay);
	TilesTexture = Asset::RequestBlock("tiles");
	for (int i = 0; i < 10; i++) {
		Decorations[i] = Asset::RequestBlock("decoration " + std::to_string(i));
	}
	for (int i = 0; i < 3; i++) {
		EnemySize[i] = Asset::RequestBlock("enemy size " + std::to_string(i));
	}
	if (TilesTexture == 0) {
		DEBUG(0, NE_ERROR, "Well, this sucks. Couldn't load the tiles texture.");
	}
	for (int i = 0; i < 6; i++) {
		Asset::Request("eat sfx " + std::to_string(i + 1), &EatSFX[i]);
	}
	for (int i = 0; i < 8; i++) {
		Asset::Request("eery sfx " + std::to_string(i + 1), &EerySFX[i]);
	}
	for (int i = 0; i < 2; i++) {
		OilPatchTextures[i] = Asset::RequestBlock("oil " + std::to_string(i));
	}
	for (int i = 0; i < 4; i++) {
		Asset::Request("evo " + std::to_string(i + 1), &EvoIcon[i]);
	}
	Asset::Request("player size 3 mouth", &MouthOverride[3]);
	Asset::Request("player size 4 mouth", &MouthOverride[4]);
	Asset::Request("player size 3 shell", &ShellOverride[3]);
	Asset::Request("player size 4 shell", &ShellOverride[4]);

	Context.CreateQuad();
	Context.CreateQuad();
	Context.CreateQuad();
	MainPlayer = new Player();

	int ChunkCount = 36864 / 2;

	MainPlayer->Transform.Position.x = std::sqrt(ChunkCount) * (((float) (TilesPerRow) / 2.0f) * 96.0f);
	MainPlayer->Transform.Position.y = std::sqrt(ChunkCount) * (((float) (TilesPerRow) / 2.0f) * 96.0f);

	// Set camera
	Ortho.Target = &MainPlayer->Transform;
	Ortho.TargetChaseAspect = { 2.0f, 2.0f };

	// Add tiles
	TextureAsset* TilesTextureAsset = TextureAsset::Get(TilesTexture);
	if (!TilesTextureAsset) {
		return;
	}
	int MapSize = TilesPerRow * (int) TileSize;
	uint64 Start = GetTicks();
	for (int i = 0; i < UniqueTileMaps; i++) {
		ReferenceMaps.push_back(TileMap());
		vector2i TilePos;
		ReferenceMaps[i].Texture.Pixels = new uint32 [MapSize * MapSize];
		ReferenceMaps[i].Texture.Size = { MapSize, MapSize };
		for (int j = 0; j < TotalTilesInMap; j++) {
			if (j != 0) {
				if (j % TilesPerRow == 0) {
					TilePos.x = 0;
					TilePos.y++;
				} else {
					TilePos.x++;
				}
			}
			RenderTile(TilesTextureAsset, ReferenceMaps[i].Texture.Pixels, TilePos, rand() % 4, vector2i(0.0f, 0.0f), vector2i(96, 96));
		}
		int DecorationCount = 150 + rand() % 310;
		for (int j = 0; j < DecorationCount; j++) {
			vector2i Offset = { 32 + rand() % (96 * (TilesPerRow - 2)), 32 + rand() % (96 * (TilesPerRow - 2)) };
			TextureAsset* Decoration = TextureAsset::Get(Decorations[rand() % 10]);
			RenderTile(Decoration, ReferenceMaps[i].Texture.Pixels, vector2i(0, 0), 0, Offset, Decoration->Size);
		}
		LoadTexture(&ReferenceMaps[i].Texture);
		delete [] ReferenceMaps[i].Texture.Pixels;
		ReferenceMaps[i].Texture.Pixels = nullptr;
	}
	DEBUG(0, NE_INFO, "Generated " << ReferenceMaps.size() << " maps in " << (GetTicks() - Start) / 1000 << " milliseconds");

	vector2i CurMapPos;
	int PerRow = std::sqrt(ChunkCount);
	int ChunkWidth = 96 * TilesPerRow;
	int TotalMapWidth = ChunkWidth * PerRow;
	for (int i = 0; i < ChunkCount; i++) {
		if (i != 0) {
			if (i % PerRow == 0) {
				CurMapPos.x = 0;
				CurMapPos.y += 96 * TilesPerRow;
			} else {
				CurMapPos.x += 96 * TilesPerRow;
			}
		}
		Chunks.push_back(Chunk(this, &ReferenceMaps[rand() % ReferenceMaps.size()], CurMapPos));
		int OilCount = rand() % 100;
		if (OilCount > 80) {
			OilCount = 1;
		} else  {
			OilCount = 0;
		}
		for (int j = 0; j < OilCount; j++) {
			AssetHandle OilTextureHandle = OilPatchTextures[rand() % 2];
			TextureAsset* OilTexture = TextureAsset::Get(OilTextureHandle);
			vector2f Offset = { (float) (rand() % (96 * TilesPerRow)), (float) (rand() % (96 * TilesPerRow)) };
			Offset.x += CurMapPos.x;
			Offset.y += CurMapPos.y;
			int RateInt = rand() % 100;
			float Rate = 10.0f / (float) RateInt;
			Chunks[i].OilPatches.push_back(OilPatch(Rate, Offset, OilTexture->Size, OilTextureHandle));
		}
	}

	AddEventHandler(KEYBOARD_KEY_EVENT, [] (Event* e) {
		KeyboardKeyEvent* KeyEvent = (KeyboardKeyEvent*) e;
		GameLoop* Loop = (GameLoop*) KeyEvent->Userdata;
		if (KeyEvent->Key == KEY_SPACE) {
			Loop->BuyEvolution();
		} else if (KeyEvent->Key == KEY_RETURN) {
			Loop->BuyUpgrade();
		}
	}, this);

	EeryTimer.Start();

	SetVSync(false);
}

void GameLoop::Stop() {

}

void GameLoop::Update() {
	
	if (!IsMusicPlaying()) {
		PlayMusic(WildlyGrowing);
		SetMusicVolume(MusicVolume);
	}

	Ortho.Update();

	Transformation MapTrans;
	MapTrans.Scale.xy = { TileSize * (float) TilesPerRow, TileSize * (float) TilesPerRow };
	int MapsPerRow = std::sqrt(Chunks.size());
	vector2f CamBoundPos = { Ortho.Position.x * 2.0f - 512.0f, Ortho.Position.y * 2.0f - 512.0f };
	vector2f CamBoundSize = { Ortho.Scale.x + 1024.0f, Ortho.Scale.y + 1024.0f };
	OilPatchesToDraw.clear();
	ChunksInView.clear();

	for (unsigned int i = 0; i < Chunks.size(); i++) {
		
		MapTrans.Position.xy = { Chunks[i].Position };

		if (!IsTransformOver(&MapTrans, CamBoundPos, CamBoundSize)) {
			continue;
		}

		ChunksInView.push_back(&Chunks[i]);
		
		for (auto& Oil : Chunks[i].OilPatches) {
			OilPatchesToDraw.push_back(&Oil);
		}

		MainPlayer->CheckForOil(&Chunks[i]);

		for (unsigned int j = 0; j < Chunks[i].Enemies.size(); j++) {
			Chunks[i].Enemies[j].Update(this, &Chunks[i], MapTrans.Position.xy);
			if (Chunks[i].Enemies[j].IsEaten) {
				if (Chunks[i].Enemies[j].KilledBy == 0) {
					int EnemyEvo = Chunks[i].Enemies[j].Evolution;
					MainPlayer->Growth += (EnemyEvo == 0 ? 25 : (EnemyEvo == 1 ? 100 : (EnemyEvo == 2 ? 150 : 50)));
					MainPlayer->Growth += Chunks[i].Enemies[j].Growth / 2;
					MainPlayer->Health += 0.5f;
					if (MainPlayer->Health > 100.0f) {
						MainPlayer->Health = 100.0f;
					}
					int Sound = EatSFX[rand() % 6];
					PlaySound(Sound);
					SetSoundVolume(Sound, 30);
				} else {
					Chunks[i].Enemies[j].KilledByEnemy->Growth += 5;
					Chunks[i].Enemies[j].KilledByEnemy->Health += 5.0f;
				}
				Chunks[i].Enemies.erase(Chunks[i].Enemies.begin() + j);
				j--;
			} else if (Chunks[i].Enemies[j].IsOutOfBounds) {
				FindChunkForEnemy(i, j);
				j--;
			}
		}

	}
	
	EnemiesToDraw.clear();

	for (unsigned int i = 0; i < ChunksInView.size(); i++) {
		for (unsigned int j = 0; j < ChunksInView[i]->Enemies.size(); j++) {
			EnemiesToDraw.push_back(&ChunksInView[i]->Enemies[j]);
		}
	}

	MainPlayer->Update();

	if (MainPlayer->Health <= 0.0f) {
		MainPlayer->Health = -0.0001f;
		if (RespawnTimer.GetMilliseconds() > 3000) {
			MainPlayer->Health = 100.0f;
			MainPlayer->Evolution = 0;
			MainPlayer->Growth = 0;
			MainPlayer->HasUpgrade[UPGRADE_CHEW] = false;
			MainPlayer->HasUpgrade[UPGRADE_SHELL] = false;
			MainPlayer->HasUpgrade[UPGRADE_PLANE] = false;
			MainPlayer->Transform.Position.x = std::sqrt(Chunks.size()) * (((float) (TilesPerRow) / 2.0f) * 96.0f);
			MainPlayer->Transform.Position.y = std::sqrt(Chunks.size()) * (((float) (TilesPerRow) / 2.0f) * 96.0f);
			RespawnTimer.Stop();
			Respawning = false;
		} else if (!Respawning) {
			Respawning = true;
			RespawnTimer.Start();
			for (int i = -18; i < 18; i++) {
				for (int j = -14; j < 14; j++) {
					float x = MainPlayer->Transform.Position.x;
					float y = MainPlayer->Transform.Position.y;
					x -= (float) i * 32.0f;
					y -= (float) j * 32.0f;
					x += (float) (-32 + rand() % 64);
					y += (float) (-32 + rand() % 64);
					BloodParticles.push_back(BloodParticle(x, y));
				}
			}
		}
	}

	if (EeryTimer.GetMilliseconds() > 10000) {
		int SoundRand = rand() % 8;
		PlaySound(EerySFX[SoundRand]);
		SetSoundVolume(EerySFX[SoundRand], 10);
		EeryTimer.Start();
	}

	FPSText.Text = "Delta: " + std::to_string(GetDelta()) + "  FPS: " + std::to_string(GetCurrentFPS());
	//PlayerInfo.Text = "Maps in view: " + std::to_string(ChunksInView.size()) + " ... Player: " + MainPlayer->Transform.ToString();
	HUD.FoodText.Text = "Food: " + std::to_string(MainPlayer->Growth);
}

void GameLoop::Display() {
	if (BasicShader == 0) {
		return;
	}

	// Prepare for rendering
	Context.SetShader(BasicShader);
	Context.SetCamera(0);
	Context.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw tiles
	Transformation MapTrans;
	MapTrans.Scale.xy = { TileSize * (float) TilesPerRow, TileSize * (float) TilesPerRow };
	int MapsPerRow = std::sqrt(Chunks.size());
	vector2f CamBoundPos = { Ortho.Position.x * 2.0f - 512.0f, Ortho.Position.y * 2.0f - 512.0f };
	vector2f CamBoundSize = { Ortho.Scale.x + 1024.0f, Ortho.Scale.y + 1024.0f };
	Context.SetQuad(0);
	Context.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	for (auto& CurrentChunk : ChunksInView) {
		MapTrans.Position.xy = { CurrentChunk->Position };
		Context.SetTransform(&MapTrans);
		Context.SetTexture(&CurrentChunk->Tiles->Texture);
		Context.DisplayTexture();
	}

	// Draw enemies
	if (EnemiesToDraw.size() > 0) {
		Context.SetQuad(1);
		for (auto& CurrentEnemy : EnemiesToDraw) {

			Transformation T = CurrentEnemy->Transform;
			T.Position.x += 12.0f;
			T.Position.y += 12.0f;

			Context.SetSprite(EnemySize[CurrentEnemy->Evolution]);

			Context.SetTransform(&T);
			Context.SetColor(0.0f, 0.0f, 0.0f, 0.3f);
			Context.DisplaySprite(&CurrentEnemy->Animation);

			Context.SetTransform(&CurrentEnemy->Transform);
			Context.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			Context.DisplayActiveQuad();
		}
	}

	// Draw player
	Context.SetQuad(1);
	Context.SetSprite(MainPlayer->Sprite);

	Transformation T = MainPlayer->Transform;
	T.Position.x += 12.0f;
	T.Position.y += 12.0f;

	Context.SetTransform(&T);
	Context.SetColor(0.0f, 0.0f, 0.0f, 0.3f);
	Context.DisplaySprite(&MainPlayer->Animation);

	Context.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	Context.SetTransform(&MainPlayer->Transform);
	Context.DisplayActiveQuad();

	if (MainPlayer->HasUpgrade[UPGRADE_CHEW] && MainPlayer->Evolution < 5) {
		Context.SetSprite(MouthOverride[MainPlayer->Evolution]);
		Context.DisplayActiveQuad();
	}

	if (MainPlayer->HasUpgrade[UPGRADE_SHELL] && MainPlayer->Evolution < 5) {
		Context.SetSprite(ShellOverride[MainPlayer->Evolution]);
		Context.DisplayActiveQuad();
	}

	// Draw blood
	T.Rotation.z = 0.0f;
	Context.SetSprite(BloodSprite);
	SpriteAsset* BloodSpriteAsset = SpriteAsset::Get(BloodSprite);
	T.Scale.x = BloodSpriteAsset->Texture.Size.x / (float) BloodSpriteAsset->Properties.Frames;
	T.Scale.y = BloodSpriteAsset->Texture.Size.y;
	for (unsigned int i = 0; i < BloodParticles.size(); i++) {
		T.Position.xy = { BloodParticles[i].Position };
		T.Position.x -= T.Scale.x / 2.0f;
		T.Position.y -= T.Scale.y / 2.0f;
		Context.SetTransform(&T);
		Context.SetColor(1.0f, 1.0f, 1.0f, 0.7f);
		Context.DisplaySprite(&BloodParticles[i].Animation);
		if (BloodParticles[i].Animation.SubFrame < 0.0001f) {
			BloodParticles.erase(BloodParticles.begin() + i);
			i--;
		}
	}

	// Draw weed
	Context.SetQuad(0);
	Context.SetTexture(WeedTexture);
	Transformation WeedTransform;
	TextureAsset* WeedTextureAsset = TextureAsset::Get(WeedTexture);
	WeedTransform.Scale.xy = { (float) WeedTextureAsset->Size.x, (float) WeedTextureAsset->Size.y };
	for (auto& CurrentChunk : ChunksInView) {
		for (auto& CurrentWeed : CurrentChunk->Weeds) {
			WeedTransform.Position.xy = CurrentWeed.Position;
			Context.SetTransform(&WeedTransform);
			Context.DisplayTexture();
		}
	}

	// Draw oil
	if (OilPatchesToDraw.size() > 0) {
		Context.SetColor(1.0f, 1.0f, 1.0f, 0.5f);
		for (auto& Oil : OilPatchesToDraw) {
			Context.SetTexture(Oil->Texture);
			Context.SetTransform(&Oil->Transform);
			Context.DisplayTexture();
		}
	}

	// Draw fog
	Context.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	Transformation Transform;
	Transform.Position.xy = { Ortho.Position.x * 2.0f, Ortho.Position.y * 2.0f };
	Transform.Scale.xy = Ortho.Scale;
	Context.SetTexture(OverlayFog);
	Context.SetTransform(&Transform);
	Context.DisplayTexture();

	// HUD
	Context.SetFont(Tahoma);
	TextureAsset* HealthAsset = TextureAsset::Get(HUD.HealthTexture);
	Context.SetQuad(2);
	Transform.Position.x += Ortho.Scale.x / 2.0f - HealthAsset->Size.x / 2.0f - 570.0f;
	Transform.Position.y += 32.0f;
	Transform.Scale.xy = { (float) HealthAsset->Size.x, (float) HealthAsset->Size.y };
	Context.SetTexture(HUD.HealthTexture);
	Context.SetTransform(&Transform);
	Context.SetColor(1.0f, 1.0f, 1.0f, 0.1f);
	Context.DisplayTexture();
	Transform.Scale.x *= MainPlayer->Health / 100.0f;
	Context.SetTransform(&Transform);
	Context.SetColor(1.0f, 1.0f, 1.0f, 0.5f);
	Context.DisplayTexture();

	// Level Info
	LevelInfo.Text = "Level " + std::to_string(MainPlayer->Evolution);
	LevelInfo.Transform = Transform;
	LevelInfo.Transform.Position.y += HealthAsset->Size.y + 8.0f;
	Context.SetColor(1.0f, 1.0f, 1.0f, 0.8f);
	Context.SetTransform(&LevelInfo.Transform);
	Context.DisplayText(&LevelInfo);

	// Food Info
	Context.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	Transformation WeedIconTransform = HUD.FoodText.Transform;
	WeedIconTransform.Position.x = Transform.Position.x + (float) HealthAsset->Size.x + 64.0f;
	WeedIconTransform.Position.y = Ortho.Position.y * 2.0f + 16.0f;
	WeedIconTransform.Scale.xy = { WeedTransform.Scale.x / 3.0f, WeedTransform.Scale.y / 3.0f };
	Context.SetTexture(WeedTexture);
	Context.SetTransform(&WeedIconTransform);
	Context.DisplayTexture();
	HUD.FoodText.Transform.Position.xy = {
		Transform.Position.x + (float) HealthAsset->Size.x + WeedIconTransform.Scale.x + 80.0f,
		Ortho.Position.y * 2.0f + 16.0f + WeedIconTransform.Scale.y / 2.0f - HUD.FoodText.Transform.Scale.y / 2.0f,
	};
	Context.SetTransform(&HUD.FoodText.Transform);
	Context.DisplayText(&HUD.FoodText);

	// Upgrades
	TextureAsset* ChewTexture = TextureAsset::Get(UpgradeItems[UPGRADE_CHEW].Icon);
	Transform.Position.x = WeedIconTransform.Position.x - ((float) ChewTexture->Size.x * 1.5f) + 360.0f;
	Transform.Scale.x = (float) ChewTexture->Size.x;
	Transform.Scale.y = (float) ChewTexture->Size.y;

	if (!MainPlayer->HasUpgrade[UPGRADE_CHEW]) {
		UpgradeItems[UPGRADE_CHEW].PriceInfo.Text = std::to_string(UpgradeItems[UPGRADE_CHEW].Cost) + " Food & Lev. 3";
	}

	if (!MainPlayer->HasUpgrade[UPGRADE_SHELL]) {
		UpgradeItems[UPGRADE_SHELL].PriceInfo.Text = std::to_string(UpgradeItems[UPGRADE_SHELL].Cost) + " Food & Lev. 3";
	}

	if (!MainPlayer->HasUpgrade[UPGRADE_PLANE]) {
		UpgradeItems[UPGRADE_PLANE].PriceInfo.Text = std::to_string(UpgradeItems[UPGRADE_PLANE].Cost) + " Food & Lev. 3";
	}

	for (unsigned int i = 0; i < 3; i++) {

		if (!MainPlayer->HasUpgrade[i] && MainPlayer->Growth >= UpgradeItems[i].Cost && MainPlayer->Evolution >= UpgradeItems[i].Evolution) {
			UpgradeItems[i].PriceInfo.Text = "[ENTER] Buy: " + std::to_string(UpgradeItems[i].Cost) + " Food";
		}

		bool FullAlpha = MainPlayer->Evolution >= UpgradeItems[i].Evolution 
					 && (MainPlayer->HasUpgrade[i] || MainPlayer->Growth >= UpgradeItems[i].Cost);

		Transform.Position.y -= 20.0f;
		Context.SetColor(1.0f, 1.0f, 1.0f, (FullAlpha ? 1.0f : 0.1f));
		Context.SetTransform(&Transform);
		Context.SetTexture(UpgradeItems[i].Icon);
		Context.DisplayTexture();
		Transform.Position.y += 20.0f;
		Transform.Position.x += (float) ChewTexture->Size.x;
		Context.SetColor(1.0f, 1.0f, 1.0f, (FullAlpha ? 0.8f : 0.1f));
		UpgradeItems[i].Description.Transform.Position = Transform.Position;
		Context.SetTransform(&UpgradeItems[i].Description.Transform);
		Context.DisplayText(&UpgradeItems[i].Description);
		Transform.Position.y += 32.0f;
		UpgradeItems[i].PriceInfo.Transform.Position = Transform.Position;
		Context.SetTransform(&UpgradeItems[i].PriceInfo.Transform);
		Context.SetColor(1.0f, 1.0f, 1.0f, (FullAlpha ? 0.6f : 0.05f));
		Context.DisplayText(&UpgradeItems[i].PriceInfo);
		Transform.Position.x += 256.0f;
		Transform.Position.y -= 32.0f;
	}

	// You Can Upgrade
	if (MainPlayer->CanGrow()) {
		YouCanUpgrade.Transform.Position.x = Ortho.Position.x * 2.0f + Ortho.Scale.x / 2.0f - YouCanUpgrade.Transform.Scale.x / 2.0f;
		YouCanUpgrade.Transform.Position.y = Ortho.Position.y * 2.0f + Ortho.Scale.y / 4.5f;
		YouCanUpgrade.Text = "You Can Evolve!";
		YouCanUpgradeHelp.Text = "[Press SPACE To Upgrade]";
		Context.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		Context.SetTransform(&YouCanUpgrade.Transform);
		Context.DisplayText(&YouCanUpgrade);
		YouCanUpgradeHelp.Transform = YouCanUpgrade.Transform;
		YouCanUpgradeHelp.Transform.Position.y += 32.0f;
		Context.SetTransform(&YouCanUpgradeHelp.Transform);
		Context.DisplayText(&YouCanUpgradeHelp);
		TextureAsset* EvoTexture = TextureAsset::Get(EvoIcon[MainPlayer->Evolution]);
		TextureAsset* UIOverlayTex = TextureAsset::Get(UIOverlay);
		if (EvoTexture) { // Just check in case there is an evolution bug
			Transform.Position = YouCanUpgrade.Transform.Position;
			Transform.Scale.xy = {
				(float) UIOverlayTex->Size.x,
				(float) UIOverlayTex->Size.y
			};
			Transform.Position.x -= 32.0f;
			Transform.Position.y -= Transform.Scale.y / 2.0f;
			Context.SetTransform(&Transform);
			Context.SetTexture(UIOverlay);
			Context.DisplayTexture();

			Transform.Position = YouCanUpgrade.Transform.Position;
			Transform.Position.x += YouCanUpgrade.Transform.Scale.x / 2.0f - (float) EvoTexture->Size.x / 2.0f;
			Transform.Position.y -= (float) EvoTexture->Size.y + 32.0f;
			Transform.Scale.x = EvoTexture->Size.x;
			Transform.Scale.y = EvoTexture->Size.y;
			Context.SetTransform(&Transform);
			Context.SetTexture(EvoIcon[MainPlayer->Evolution]);
			Context.DisplayTexture();
		}
	}

	// Draw delta and FPS
	FPSText.Transform.Position.xy = { Ortho.Position.x * 2.0f + 20.0f, Ortho.Position.y * 2.0f + 20.0f };
	//PlayerInfo.Transform.Position.xy = { Ortho.Position.x * 2.0f + 20.0f, Ortho.Position.y * 2.0f + 40.0f };
	Context.SetColor(1.0f, 1.0f, 1.0f, 0.2f);
	Context.DisplayText(&FPSText);
	//Context.DisplayText(&PlayerInfo);

}

void GameLoop::BuyUpgrade() {
	for (int i = 0; i < 3; i++) {
		if (MainPlayer->Evolution < UpgradeItems[i].Evolution) {
			continue;
		}
		if (!MainPlayer->HasUpgrade[i]) {
			if (MainPlayer->Growth >= UpgradeItems[i].Cost) {
				MainPlayer->Growth -= UpgradeItems[i].Cost;
				MainPlayer->HasUpgrade[i] = true;
				UpgradeItems[i].PriceInfo.Text = "[Upgraded]";
				MainPlayer->EquipUpgrade(i);
			}
		}
	}
}

void GameLoop::BuyEvolution() {
	if (MainPlayer->CanGrow()) {
		MainPlayer->Grow();
	}
}