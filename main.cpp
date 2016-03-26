#include "ThirdParty/GLEW/glew.h"
#include "ThirdParty/SDL/SDL.h"
#include "debug.hpp"
#include "memory.hpp"

#include "game.hpp"

int main(int argc, char** argv) {
	StartEngine("Wildly Growing", 1280, 720);
	MaximiseMainWindow();

	// basic shader
	ShaderProperties ShaderProps;
	ShaderProps.Path = "Assets/Shaders/basic";
	Asset::Prepare(ASSET_TYPE_SHADER, "basic", &ShaderProps);

	// background
	TextureProperties TextureProps;
	TextureProps.Path = "Assets/Textures/Weed.png";
	Asset::Prepare(ASSET_TYPE_TEXTURE, "weed", &TextureProps);
	TextureProps.Path = "Assets/Textures/UIOverlay.png";
	Asset::Prepare(ASSET_TYPE_TEXTURE, "ui overlay", &TextureProps);
	TextureProps.Path = "Assets/Textures/Upgrades/Chewing.png";
	Asset::Prepare(ASSET_TYPE_TEXTURE, "chew", &TextureProps);
	TextureProps.Path = "Assets/Textures/Upgrades/Shell.png";
	Asset::Prepare(ASSET_TYPE_TEXTURE, "shell", &TextureProps);
	TextureProps.Path = "Assets/Textures/Upgrades/Plane.png";
	Asset::Prepare(ASSET_TYPE_TEXTURE, "plane", &TextureProps);
	TextureProps.Path = "Assets/Textures/Fog.png";
	Asset::Prepare(ASSET_TYPE_TEXTURE, "fog", &TextureProps);
	TextureProps.Path = "Assets/Textures/Health.png";
	Asset::Prepare(ASSET_TYPE_TEXTURE, "health", &TextureProps);
	TextureProps.Path = "Assets/Textures/tiles.png";
	TextureProps.PixelsInMemory = true;
	Asset::Prepare(ASSET_TYPE_TEXTURE, "tiles", &TextureProps);
	for (int i = 0; i < 10; i++) {
		TextureProps.Path = "Assets/Textures/Decoration/" + std::to_string(i) + ".png";
		Asset::Prepare(ASSET_TYPE_TEXTURE, "decoration " + std::to_string(i), &TextureProps);
	}
	for (int i = 0; i < 2; i++) {
		TextureProps.Path = "Assets/Textures/Oil/" + std::to_string(i) + ".png";
		Asset::Prepare(ASSET_TYPE_TEXTURE, "oil " + std::to_string(i), &TextureProps);
	}
	for (int i = 1; i < 5; i++) {
		TextureProps.Path = "Assets/Textures/Upgrades/Evo" + std::to_string(i) + ".png";
		Asset::Prepare(ASSET_TYPE_TEXTURE, "evo " + std::to_string(i), &TextureProps);
	}

	// player
	SpriteProperties SpriteProps;
	SpriteProps.Frames = 1;
	SpriteProps.Path = "Assets/Sprites/PlayerSize0.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 0", &SpriteProps);
	SpriteProps.Frames = 6;
	SpriteProps.Path = "Assets/Sprites/Blood.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "blood", &SpriteProps);
	SpriteProps.Frames = 8;
	SpriteProps.Path = "Assets/Sprites/EnemySize0.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "enemy size 0", &SpriteProps);
	SpriteProps.Path = "Assets/Sprites/EnemySize1.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "enemy size 1", &SpriteProps);
	SpriteProps.Path = "Assets/Sprites/PlayerSize1.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 1", &SpriteProps);
	SpriteProps.Path = "Assets/Sprites/PlayerSize2.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 2", &SpriteProps);
	SpriteProps.Path = "Assets/Sprites/PlayerSize3.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 3", &SpriteProps);
	SpriteProps.Path = "Assets/Sprites/PlayerSize4.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 4", &SpriteProps);
	SpriteProps.Frames = 1; // set to 8 for bird mode
	SpriteProps.Path = "Assets/Sprites/PlayerSize5.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 5", &SpriteProps);
	SpriteProps.Frames = 8;
	SpriteProps.Path = "Assets/Sprites/PlayerSize3Mouth.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 3 mouth", &SpriteProps);
	SpriteProps.Path = "Assets/Sprites/PlayerSize3Shell.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 3 shell", &SpriteProps);
	SpriteProps.Path = "Assets/Sprites/PlayerSize4Mouth.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 4 mouth", &SpriteProps);
	SpriteProps.Path = "Assets/Sprites/PlayerSize4Shell.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "player size 4 shell", &SpriteProps);
	SpriteProps.Frames = 1;
	SpriteProps.Path = "Assets/Sprites/FoodOfPeace.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "food", &SpriteProps);
	SpriteProps.Frames = 4;
	SpriteProps.Path = "Assets/Sprites/Squid.png";
	Asset::Prepare(ASSET_TYPE_SPRITE, "enemy size 2", &SpriteProps);

	// tahoma font
	FontProperties FontProps;
	FontProps.IsSystemFont = true;
	FontProps.Size = 20;
	FontProps.Path = "tahoma.ttf";
	Asset::Prepare(ASSET_TYPE_FONT, "tahoma", &FontProps);

	// sounds
	SoundProperties SoundProps;
	for (int i = 1; i < 7; i++) {
		SoundProps.Path = "Assets/Sounds/Eat" + std::to_string(i) + ".wav";
		Asset::Prepare(ASSET_TYPE_SOUND, "eat sfx " + std::to_string(i), &SoundProps);
	}
	for (int i = 1; i < 9; i++) {
		SoundProps.Path = "Assets/Sounds/eery/" + std::to_string(i) + ".wav";
		Asset::Prepare(ASSET_TYPE_SOUND, "eery sfx " + std::to_string(i), &SoundProps);
	}
	SoundProps.Path = "Assets/Sounds/evolve.wav";
	Asset::Prepare(ASSET_TYPE_SOUND, "evolve", &SoundProps);

	// music
	MusicProperties MusicProps;
	MusicProps.Path = "Assets/Music/WildlyGrowing.mp3";
	Asset::Prepare(ASSET_TYPE_MUSIC, "wildly growing", &MusicProps);

	return EnterLoop(&GameLoop());
}