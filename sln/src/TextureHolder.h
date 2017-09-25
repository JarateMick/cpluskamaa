#pragma once

#include <SDL2/SDL.h>
#include <glm\glm.hpp>

#define MAX_BUTTONS 5 // kuinka monta nappia on hiiressä?
struct InputManager
{
	void update()
	{
		for (int i = 0; i < SDL_KEYCODE_SIZE; ++i)
		{
			lastFrame[i] = thisFrame[i];
		}
		for (int i = 0; i < MAX_BUTTONS; ++i)
		{
			lastFrameButton[i] = thisFrameButton[i];
		}
	}
	void pressKey(int keyID)
	{
		thisFrame[keyID] = true;
	}
	void releaseKey(int keyID)
	{
		thisFrame[keyID] = false;
	}
	void pressMouse(int key)
	{
		if (key > MAX_BUTTONS - 1)
		{
			// TODO: assert macro XD
			int *a = 0;
			int b = *a;
		}
		thisFrameButton[key] = true;
	}
	void releaseMouse(int key)
	{
		thisFrameButton[key] = false;
	}
	bool isKeyDown(int keyID)
	{
		return thisFrame[keyID];
	}
	bool isKeyPressed(int keyID)
	{
		return thisFrame[keyID] && !lastFrame[keyID];
	}

	bool isMouseDown(int button = 0)
	{
		// Assert
		return thisFrameButton[button];
	}

	bool isMouseClicked(int button = 1)
	{
		return thisFrameButton[button] && !lastFrameButton[button];
	}

	void reset()
	{
		memset(thisFrame, 0, SDL_KEYCODE_SIZE);
		memset(lastFrame, 0, SDL_KEYCODE_SIZE);
		memset(thisFrameButton, 0, MAX_BUTTONS);
		memset(lastFrameButton, 0, MAX_BUTTONS);
	}

	static const int SDL_KEYCODE_SIZE = SDL_NUM_SCANCODES; // +1 // 512 
	static const int SDL_BUTTON_COUNT = MAX_BUTTONS;
	bool thisFrame[SDL_KEYCODE_SIZE];
	bool lastFrame[SDL_KEYCODE_SIZE];

	glm::vec2 rawMouse;
	glm::vec2 mouse;

	bool thisFrameButton[MAX_BUTTONS];
	bool lastFrameButton[MAX_BUTTONS];
};

//class LTexture
//{
//public:
//	void free(); // heh heh
//
//	void render(SDL_Renderer* renderer, int x, int y)
//	{
//		SDL_Rect quad{ x - camera->x, y - camera->y, width, height };
//		SDL_RenderCopy(renderer, texture, 0, &quad);
//	}
//
//	void setColor(Uint8 red, Uint8 green, Uint8 blue)
//	{
//		SDL_SetTextureColorMod(texture, red, green, blue);
//	}
//
//	void setColor(SDL_Color* color)
//	{
//		SDL_SetTextureColorMod(texture, color->r, color->g, color->b);
//	}
//
//	// pointers mrjrrmmrmrs
//	void setBlendMode(SDL_BlendMode blending)
//	{
//		SDL_SetTextureBlendMode(texture, blending); // SDL_BLENDMODE_BLEND
//	}
//	void setAlpha(Uint8 alpha)
//	{
//		SDL_SetTextureAlphaMod(texture, alpha);
//	}
//
//	// clip / uv_rect
//	void render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip)
//	{
//		SDL_Rect renderQuad = { x, y, width, height };
//
//		if (clip != nullptr)
//		{
//			renderQuad.w = clip->w;
//			renderQuad.h = clip->h;
//		}
//
//		SDL_RenderCopy(renderer, texture, clip, &renderQuad);
//	}
//
//	void render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
//	{
//		SDL_Rect renderQuad = { x, y, width, height };
//
//		if (clip != NULL)
//		{
//			renderQuad.w = clip->w;
//			renderQuad.h = clip->h;
//		}
//		SDL_RenderCopyEx(renderer, texture, clip, &renderQuad, angle, center, flip);
//	}
//
//	static void setCamera(SDL_Rect* camera)
//	{
//		LTexture::camera = camera;
//	}
//
//	SDL_Texture* texture;
//	static SDL_Rect* camera;
//	int width;
//	int height;
//};
//
//SDL_Rect* LTexture::camera = 0;

//enum Textures
//{
//	Texture_shield,
//	Texture_sword,
//	// ...
//	// kirjoita nämä tietokoneella!
//	// ...
//	Texture_Max
//};
//
//const char* paths[Texture_Max] =
//{
//	"shield.txt",
//	"sword.txt"
//	// kirjoita nämä pc:llä!
//};
//
//#include "enumtest.cpp"
//struct TextureHolder
//{
//	LTexture textures[Texture_Max];
//	bool inited[Texture_Max]; // BITSET?
//	SDL_Renderer* renderer;
//
//	TTF_Font* font; // TODO: OTHER FONTS
//
//	//FC_Font* fcFont;
//	//void LoadFCfonts(SDL_Renderer* renderer, Uint32 size, const char* name, SDL_Color color)
//	//{
//	//	fcFont = FC_CreateFont();
//	//	FC_LoadFont(fcFont, renderer, name, size, color, TTF_STYLE_NORMAL);
//
//	//	// 	 FC_LoadFont(font, gRenderer, "rs.ttf", 20, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);
//	//}
//
//	//void draw()
//	//{
//	//	
//	//}
//
//	//Mix_Music* gMusic = Mix_LoadMUS("beat.wav");
//	//if (gMusic == NULL)
//	//{
//	//	printf("Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
//	//}
//	//Mix_Chunk* gScratch = Mix_LoadWAV("scratch.wav");
//	//if (gScratch == NULL)
//	//{
//	//	printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
//	//}
//	LTexture getTexture(Textures textureID)
//	{
//		if (inited[textureID])
//		{
//			return textures[textureID]; // öö xD
//		}
//		textures[textureID] = loadLTexture(paths[textureID]);
//		inited[textureID] = true;
//		return textures[textureID];
//	}
//
//	LTexture getUniqueTexture(Textures textureID)
//	{
//		return loadLTexture(paths[textureID]);
//	}
//
//	LTexture loadLTexture(const std::string &path)
//	{
//		LTexture result{};
//		SDL_Texture* newTexture = NULL;
//		SDL_Surface* loadedSurface = IMG_Load(path.c_str());
//		if (loadedSurface == NULL)
//		{
//			printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
//		}
//		else
//		{
//			newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
//			if (newTexture == NULL)
//			{
//				printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
//			}
//			result.texture = newTexture;
//			result.width = loadedSurface->w;
//			result.height = loadedSurface->h;
//			SDL_FreeSurface(loadedSurface);
//		}
//		return result;
//	}
//	LTexture loadFromRenderedText(std::string textureText, SDL_Color textColor)
//	{
//		LTexture result{};
//		SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
//		if (textSurface == NULL)
//		{
//			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
//		}
//		else
//		{
//			//Create texture from surface pixels
//			result.texture = SDL_CreateTextureFromSurface(renderer, textSurface);
//			if (result.texture == NULL)
//			{
//				printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
//			}
//			else
//			{
//				//Get image dimensions
//				result.width = textSurface->w;
//				result.height = textSurface->h;
//			}
//
//			//Get rid of old surface
//			SDL_FreeSurface(textSurface);
//		}
//		return result;
//	}
//};
//
//struct Music
//{
//	Mix_Music* music[Music_Max];
//	Mix_Chunk  effect[Effect_Max];
//	Musics     current;
//
//	void init() // kun on paljon musiikkeja niin tee async lataus ja freea music sitä mukaan kuin soitat
//	{
//		for (int i = 0; i < Music_Max; ++i)
//		{
//			//load
//			// Mix_LoadMUS()
//			// Mix_LoadWAV()
//		}
//		for (int i = 0; i < Effect_Max; ++i)
//		{
//			//load
//			// Mix_LoadMUS()
//			// Mix_LoadWAV()
//		}
//	}
//	void playMusic(Musics musicID)
//	{
//		// -1 meinaa soittaa niin kauan kunnes musa loppuu
//		Mix_PlayMusic(music[musicID], -1);
//	}
//	void pauseMusic()
//	{
//		Mix_PausedMusic();
//	}
//	void continuePlaying()
//	{
//		Mix_ResumeMusic();
//	}
//	void haltMusic()
//	{
//		Mix_HaltMusic();
//	}
//	bool isMusicPaused()
//	{
//		return Mix_PausedMusic();
//	}
//	bool isMusicPlaying()
//	{
//		return Mix_PlayingMusic();
//	}
//	void playEffect(Effects effectID, int loops = 1)
//	{
//		Mix_PlayChannel(-1, &effect[effectID], loops);
//	}
//};

//class TextureHolder
//{
//public:
//	LTexture* getTexture(std::string textureName)
//	{
//		LTexture* result;
//		if (textures.count(textureName) > 0)
//		{
//			result = &textures.at(textureName);
//		}
//		else
//		{
//			// loadTexture
//			LTexture texture = loadLTexture(textureName);
//			textures.insert(std::pair<std::string, LTexture>(textureName, texture));
//			result = &textures.at(textureName);
//		}
//		return result;
//	}
//
//	LTexture* dirtyInit(std::string textureName)
//	{
//		LTexture texture = loadLTexture(textureName);
//		benchmark.insert(std::pair<int, LTexture>(2, texture));
//		return &benchmark.at(2);
//	}
//	LTexture* dirtyGet(int i)
//	{
//		return &benchmark.at(i);
//	}
//
//	LTexture loadLTexture(std::string path)
//	{
//		LTexture result{};
//		SDL_Texture* newTexture = NULL;
//		SDL_Surface* loadedSurface = IMG_Load(path.c_str());
//		if (loadedSurface == NULL)
//		{
//			printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
//		}
//		else
//		{
//			newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
//			if (newTexture == NULL)
//			{
//				printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
//			}
//			result.texture = newTexture;
//			result.width = loadedSurface->w;
//			result.height = loadedSurface->h;
//			SDL_FreeSurface(loadedSurface);
//		}
//		return result;
//	}
//
//
//	LTexture loadFromRenderedText(std::string textureText, SDL_Color textColor)
//	{
//		LTexture result{};
//		SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
//		if (textSurface == NULL)
//		{
//			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
//		}
//		else
//		{
//			//Create texture from surface pixels
//			result.texture = SDL_CreateTextureFromSurface(renderer, textSurface);
//			if (result.texture == NULL)
//			{
//				printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
//			}
//			else
//			{
//				//Get image dimensions
//				result.width = textSurface->w;
//				result.height = textSurface->h;
//			}
//
//			//Get rid of old surface
//			SDL_FreeSurface(textSurface);
//		}
//		return result;
//	}
//
//	// init theeese
//	SDL_Renderer* renderer; // hehehe
//	
//private:
//	std::map<std::string, LTexture> textures;
//	std::map<int, LTexture> benchmark;
//};