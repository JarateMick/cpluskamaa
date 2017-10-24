#define _MATH_DEFINES_DEFINED 1
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1

#include <chrono>
#include <mutex>
std::mutex mutex;

#include <lua.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <thread>
#include <fstream>

#include <atomic>
#include <mutex>
#include <condition_variable>

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "Imgui/imgui_impl_sdl_gl3.cpp"
#include "Imgui/imgui_demo.cpp"

#include "game.h" 
#include "imguiTools.h"
#include "TextureHolder.h"
#include "fileSystem.cpp"      // aika hack
#include "InputRecorder.cpp"

#include <ResourceManager.h>
#include "graph.h"

struct TextureFile
{
	std::string name;
	std::string path; // iso overhead atm mutta ehk� tulevaisuudessa -> ei tarvisi
	FILETIME lastFileTime;
};

#define internal2 static
std::vector<TextureFile> gAssetFileTimes;

typedef void(*LoopType)(EngineCore*);
typedef void(*DrawType)(EngineCore*);

Imgui_func* ImguiPtr;
// typedef void(*ImguiType)(EngineCore*, ImGuiContext*, void*, AssetFileInfo*);
// #define IMGUIFUNC(name) void name(EngineCore* core, ImGuiContext* context, void* tmpData, AssetFileInfo* assetInfo)

LoopType LoopPtr;
DrawType DrawPtr;
HMODULE GameDLL;
HMODULE ImguiDLL;
FILETIME GameDLLWriteTime;

int SCREEN_WIDTH = 1200;
int SCREEN_HEIGHT = 1000;

internal2 void UnloadGameDLL()
{
	if (!FreeLibrary(GameDLL))
	{
		printf("can'r free lib\n");
	}

	GameDLL = 0;
	LoopPtr = 0;
	DrawPtr = 0;
	ImguiPtr = 0;
#if IMGUI
#endif
	FreeLibrary(ImguiDLL);
}


void* LoadAndCheckFunction(HMODULE* dll, const char* functionName)
{
	void* functionPtr = GetProcAddress(*dll, functionName);

	if (!functionPtr)
	{
		DWORD err = GetLastError();

		printf("Cant load func1: %d\n", err);
		Debug::logError("can't load function %s", functionName);
		debugBreak();
		// TODO: better logging
	}
	return functionPtr;
}

// TODO: dll latauksesta kivempi k�ytt�� 
internal2 void LoadGameDLL()
{
	WIN32_FILE_ATTRIBUTE_DATA unused;
	if (!GetFileAttributesEx("lock.tmp", GetFileExInfoStandard, &unused))
	{
		UnloadGameDLL();
		CopyFile("game.dll", "game_temp.dll", 0);
		GameDLL = LoadLibrary("game_temp.dll");

		CopyFile("imguiTools.dll", "imgui_temp.dll", 0);
		ImguiDLL = LoadLibrary("imgui_temp.dll");

		if (!GameDLL || !ImguiDLL)
		{
			DWORD err = GetLastError();
			printf("Can't load lib: %d\n", err);
			return;
		}

		LoopPtr = (LoopType)LoadAndCheckFunction(&GameDLL, "Loop");
		DrawPtr = (DrawType)LoadAndCheckFunction(&GameDLL, "Draw");

		ImguiPtr = (Imgui_func*)LoadAndCheckFunction(&ImguiDLL, "Imgui");

		GameDLLWriteTime = Win32GetLastWriteTime("game.dll");
	}
}


internal2 std::vector<std::string> get_all_files_names_within_folder(std::string folder)
{
	std::vector<std::string> names;
	std::string search_path = folder + "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

// MATH UTILS
inline float norm(float value, float min, float max)
{
	return (value - min) / (max - min);
}

inline float lerp(float norm, float min, float max)
{
	return (max - min) * norm + min;
}

inline float map(float value, float sourceMin, float sourceMax, float destMin, float destMax)
{
	return lerp(norm(value, sourceMin, sourceMax), destMin, destMax);
}

static AssetFileInfo gAssetFileInfo;

internal2 void pollTextureFiles()
{
	// compare write times
	for (int i = 0; i < gAssetFileTimes.size(); ++i)
	{
		FILETIME newTime = Win32GetLastWriteTime(gAssetFileTimes[i].path.c_str());
		auto& assetData = gAssetFileTimes[i];
		if (CompareFileTime(&assetData.lastFileTime, &newTime))
		{
			// tee kirjoitus hotload jne...
			assetData.lastFileTime = newTime;

			// lue sis�lt� tee jotain.. 
			// tai sitten guissa tee jotain.. 
		}
	}
}
// paths to assets!!!!!!
std::string gAssetPath = "..\\TEST_ENUMERATE\\Texture2";
std::string gMusicPath = "..\\TEST_ENUMERATE\\Music";
std::string gEffectPath = "..\\TEST_ENUMERATE\\Effects";

static int lastFileWatchTextureCount;
internal2 void pollForNewFiles()
{
	std::string AssetPath = gAssetPath;

	// TODO: funktio, joka palauttaisi counting tiedostoista (halvempi?)
	std::vector<std::string> fileNames = get_all_files_names_within_folder(AssetPath);

	if (lastFileWatchTextureCount != fileNames.size())
	{
		// uusia l�ydetty
		// std::unique jne...
		// algo
	}
}

internal2 void initFileWatch(std::string AssetPath)
{
	// TODO: Asset hotloading
	// TODO: struct
	// config files
	// filetime, filename
	// voisi olla toisella threadilla koko ajan tarkastamassa jos on paljon assetteja
	// std::string AssetPath = gAssetPath;
	std::vector<std::string> fileNames = get_all_files_names_within_folder(AssetPath);

	for (int i = 0; i < fileNames.size(); ++i)
	{
		std::string path = AssetPath + "\\" + fileNames[i];
		const char* p = path.c_str();
		// filetimes.push_back(Win32GetLastWriteTime(p));
		fileNames[i].resize(fileNames[i].size() - 4);
		gAssetFileTimes.push_back({ fileNames[i], path, Win32GetLastWriteTime(p) });
	}
	lastFileWatchTextureCount = fileNames.size();
}

internal2 std::vector<std::string> GetAllFilePathsInFolder(std::string AssetPath)
{
	std::vector<std::string> fileNames = get_all_files_names_within_folder(AssetPath);

	for (int i = 0; i < fileNames.size(); ++i)
	{
		std::string path = AssetPath + "\\" + fileNames[i];
		fileNames.push_back(path);
	}
	return fileNames;
}

/// TODO: 
/* ENGINE:
 *  p�he�   dev console:
 *  loggaus dev consolille! -> dev logger
 *
 *  AssetManager:
 *  open asset with paint.net from it???
 *
 *  for second game:
 *  Valinta haluaako openGl vai SDL render�innin ?
 *  lis�� fileIO:ta
 *
 *  ------------------- bestesterest fileWatch
 *  hotloading.txt files?
 *  tilemap loading: tiled for square/isometric maps also hotload everything
 *  varmaankin oma editori hex mapeille koska tiled ei niin k��nny niille
 *  engineen varmaankin sis��n rakennettu tilemaps ja !KAMERA!
 *
 * MEMORY_MANAGEMENT
 * stack/pool for some random alllocs ? -- problems
 *
 * Fonteille sama kuin texture_
 *
 * random facility? -- pretermed randoms for dev
 *
 * wishlist: networking / uNet?
 * periaattessa jos pelin koodaa t�ll� tyylill� niin voisi ehk� memcpy:ll� tehd� moninpelin?
 *
 * GAME CODE:
 *  Hex mappien viimeistely hiiren k�ytt�ytymin distancet
 *  sitten entityt: aluksi kultaisella kolmen s��nn�ll�
 *    rakennukset
 *    unitsit
 *
 *  jonkinlainen debug struct pointersit gameStateen
 */

 /*
  *  - DONE:
  *  Reload toimii kovakoodatuista filepatheist�
  *  build,  reload assets/asset -- Done!
  *  imgui .dll done!
  *
  */



#define STATE_FILENAME_COUNT MAX_PATH
struct ReplayBuffer	// memMaps
{
	HANDLE FileHandle;
	HANDLE MemoryMap;
	char fileName[STATE_FILENAME_COUNT];
	void* memoryBlock;
};

struct PlaybackState			// WINDOW handles + memory map
{
	FILE* recordingFile; // input
	FILE* playingFile;

	void* memory;
	uint32_t totalMemorySize;

	int playbackSlotIndex;
	int recordMode;		//		/index
	bool playing;
	bool recording;

	char* EXEFileName;	// needed ?
	ReplayBuffer replayBuffer[4];
};

internal2 void
getInputRecordName(int index, char* filename, bool state)
{
	sprintf(filename, "input_%s_%i.input", state ? "state" : "stream", index);
}

internal2 void
startRecordInput(InputManager* manager, PlaybackState* state, int playbackSlotIndex)
{
	char inputRecordFile[STATE_FILENAME_COUNT];
	getInputRecordName(state->playbackSlotIndex, inputRecordFile, false);
	state->recordingFile = fopen(inputRecordFile, "wb+");
	// save whole gamestate

	// fwrite(state->memory, state->totalMemorySize, 1, state->recordingFile);
	assert(playbackSlotIndex < ArrayCount(state->replayBuffer));
	ReplayBuffer* replayBuffer = &state->replayBuffer[playbackSlotIndex];
	memcpy(replayBuffer->memoryBlock, state->memory, state->totalMemorySize); // bencmark?
}

internal2 void
recordInput(InputManager* manager, PlaybackState* state)
{
	fwrite(manager, sizeof(InputManager), 1, state->recordingFile);
}

internal2 void
endRecording(PlaybackState* state)
{
	fclose(state->recordingFile);
}

internal2 void
startInputPlayback(InputManager* manager, PlaybackState* state, int playbackSlotIndex)
{
	// open file*
	// in loop read from file streaming
	char inputRecordFile[STATE_FILENAME_COUNT];
	getInputRecordName(state->playbackSlotIndex, inputRecordFile, false);
	state->playingFile = fopen(inputRecordFile, "rb+");

	auto* buffer = &state->replayBuffer[playbackSlotIndex];
	memcpy(state->memory, buffer->memoryBlock, state->totalMemorySize);

	// 	fread(state->memory, state->totalMemorySize, 1, state->playingFile);
}

internal2 void
endplayBack(PlaybackState* state)
{
	fclose(state->playingFile);
}

internal2 void
playBackInput(InputManager* manager, PlaybackState* state)
{
	int elementsRead = fread(manager, sizeof(InputManager), 1, state->playingFile);
	if (elementsRead != 1)
	{
		// eof	
		endplayBack(state);
		startInputPlayback(manager, state, state->playbackSlotIndex);
		fread(manager, sizeof(InputManager), 1, state->playingFile);
	}
}


bool circleContains(float x, float y, float r, PhysicsBody* body)
{
	if (pow((x - body->x), 2) + pow((y - body->y), 2) < pow(r, 2)) // entity on sisällä
	{
		printf("contains");
		return true;
	}
	return false;
}

static int   g_frameAdvanceCount = 1;
static float g_gameSpeed = 1.f;
static int   g_currentFrame = 0;
static bool  g_allowNegativeDelta;

void ImguiTest(ImVec4 clear_color, EngineCore* core)
{
	game_state *gameState = (game_state*)core->memory->permanentStorage;

	static bool show_test_window = true;
	static bool show_another_window = false;
	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		static float f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		static bool entityFound = false;
		static Entity* selected = nullptr;
			ImGui::Text("Current Entity count: %i", gameState->currentEntityCount);

		if (core->input->isKeyDown(SDL_SCANCODE_I))
		{
			// looppaa kaikki läpi katso onko lähistöllä entityä
			for (int i = 0; i < gameState->currentEntityCount; i++)
			{
				Entity* e = &gameState->entities[i];
				PhysicsBody* body = gameState->bodies + i;
				if (circleContains(core->input->mouse.x, core->input->mouse.y, 20.f, body))
				{
					selected = e;
					entityFound = true;
					break;
				}
			}
		}
		ImGui::Checkbox("Game pause", &core->pause);
		core->advanceNextFrame = ImGui::Button("Next Frame");
		ImGui::SameLine();
		ImGui::InputInt("Tick count", &g_frameAdvanceCount);
		ImGui::DragFloat("Game Simulation Speed", &g_gameSpeed, 0.1f);
		if (g_gameSpeed < 0.01f && !g_allowNegativeDelta)
		{
			g_gameSpeed = 0.01f;
		}
		ImGui::Checkbox("Allow negative delta", &g_allowNegativeDelta);
		ImGui::Text("Current frame: %i", g_currentFrame);
		if (core->advancedLastFrame)
		{
			core->advanceNextFrame = false;
			core->advancedLastFrame = true;
		}
		else
		{
			core->advancedLastFrame = false;
		}

		if (entityFound)
		{
			ImGui::Text("Entity type: %s, ID:", EntityNames[selected->type], selected->guid);
			PhysicsBody* body = gameState->bodies + selected->guid;
			ImGui::Text("Pos x:%f y:%f", body->x, body->y);
			ImGui::DragFloat("x", &body->x);
			ImGui::DragFloat("y", &body->y);

			if (core->input->isMouseDown(1) && circleContains(core->input->mouse.x
				, core->input->mouse.y, 40.f, body))
			{
				body->x = core->input->mouse.x;
				body->y = core->input->mouse.y;
			}

			int newType = (int)selected->type;
			ImGui::InputInt("Change type:", &newType);
			if (newType > 0 && newType < Entity_MAX)
			{
				selected->type = (Entity_Enum)newType;
			}
		}

		if (ImGui::Button("Skip!")) // func pointer or smething
		{
			core->beginSkipToFrame = true;
		}
		ImGui::SameLine();
		ImGui::InputInt("Skip to frame", &core->skipToFrame);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}


static void load_thread(PlaybackState* data)
{
	PlaybackState* inputState = data;

	for (int i = 0; i < ArrayCount(inputState->replayBuffer); i++)
	{
		ReplayBuffer* replayBuffer = &inputState->replayBuffer[i];

		// working dir?
		getInputRecordName(i, replayBuffer->fileName, true);

		// create always ?
		replayBuffer->FileHandle = CreateFileA(replayBuffer->fileName, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);

		LARGE_INTEGER maxSize;

		maxSize.QuadPart = inputState->totalMemorySize;
		replayBuffer->MemoryMap = CreateFileMapping(replayBuffer->FileHandle,
			0, PAGE_READWRITE, maxSize.HighPart, maxSize.LowPart, 0);

		replayBuffer->memoryBlock = MapViewOfFile(replayBuffer->MemoryMap, FILE_MAP_ALL_ACCESS,
			0, 0, inputState->totalMemorySize);

		if (!replayBuffer->memoryBlock)
		{
			assert(false);
		}
	}
}

// number |= 1 << x;   // setting
// number &= ~(1 << x); // clear

// number ^= 1 << x; // togle
// bit = (number >> x) & 1; // checking
// number ^= (-x ^ number) & (1 << n); // setting 1 or 0


std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

UpiEngine::GLSLProgram textureProgram;
UpiEngine::Camera2D    camera2D;
bool initShaders(UpiEngine::GLSLProgram& textureProgram)
{
	textureProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
	textureProgram.addAttribute("vertexPosition");
	textureProgram.addAttribute("vertexColor");
	textureProgram.addAttribute("vertexUV");
	return textureProgram.linkShaders();
}

#define LUAFILESCOUNT 3
const char* luaFiles[LUAFILESCOUNT] = {
	 "lua/main3.lua", "lua/main2.lua", "lua/file.lua"
};

// shader file watcher !
#define SHADERFILECOUNT 4
const char* shaderFiles[SHADERFILECOUNT] = {
	"Shaders/colorShading.frag", "Shaders/colorShading.vert",
	"Shaders/province.frag", "Shaders/province.vert"
};


// temp = pixel & fmt->Rmask;  /* Isolate red component */
// temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
// temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
// red = (Uint8)temp;

///* Get Green component */
//temp = pixel & fmt->Gmask;  /* Isolate green component */
//temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
//temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
//green = (Uint8)temp;
//
///* Get Blue component */
//temp = pixel & fmt->Bmask;  /* Isolate blue component */
//temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
//temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
//blue = (Uint8)temp;
//
///* Get Alpha component */
//temp = pixel & fmt->Amask;  /* Isolate alpha component */
//temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
//temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */
//alpha = (Uint8)temp;


// Proper thread safe resource manager
SDL_Surface* LoadSurface(const char* image)
{
	SDL_Surface* surface = IMG_Load(image);
	if (!surface)
		printf("IMG_Load: %s\n", IMG_GetError());
	return surface;
}

GLuint TurnSurfaceIntoGlTexture(SDL_Surface* surface)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	int Mode = GL_RGB;

	if (surface->format->BytesPerPixel == 4) {
		Mode = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, Mode, surface->w, surface->h, 0, Mode, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return textureID;
}

void FreeTexture(GLuint* texture)
{
	glDeleteTextures(1, texture);
}



// Logic Thread
std::thread logicThread;
static bool join = false;
std::thread LogicThread;
std::mutex logicMutex;
std::condition_variable cond;
volatile bool logicThreadRunning = true;

static bool debugDraw = false;
void LogicThreadUpdate(EngineCore* core)
{
	while (logicThreadRunning)
	{
		std::unique_lock<std::mutex> locker(logicMutex);
		cond.wait(locker);

		LoopPtr(core);


		if (core->input->isKeyPressed(SDL_SCANCODE_Y))
		{
			debugDraw = !debugDraw;
		}

		core->input->mouse = camera2D.convertScreenToWorld(core->input->rawMouse);
		core->input->update(); // TODO: maybe poll for input

		locker.unlock();
	}
}

struct Bulletst
{
	int count;

	// Uint32         allSides[60000];
	BulletBody     bulletBodies[MAXIUM_BULLETS];
};

struct gameStateCopy
{
	PhysicsBody*  bodiesCopy;
	UpiEngine::ColorRGBA8* colors;
	int           copiesCount;
	unsigned int  mapCopy;
	glm::ivec4    dimensions{};

	Bulletst       bullets;

	glm::vec4     uvs[MAX_ENTITY_COUNT];

	Entity        player;

	int mouseX, mouseY;
};

void SyncBullets(Bulletst* bullets, game_state* state)
{
	memcpy(bullets->bulletBodies, state->bulletBodies, state->bulletCount * sizeof(BulletBody));
	bullets->count = state->bulletCount;
}

static UpiEngine::ColorRGBA8 colorCopies[MAX_ENTITY_COUNT]{};
void threadedDraw(PhysicsBody* bodies, UpiEngine::ColorRGBA8* colors, int count, UpiEngine::SpriteBatch* spriteBatch, gameStateCopy* copy)
{
	for (int i = 0; i < count; i++)
	{
		PhysicsBody* body = bodies + i;
		UpiEngine::ColorRGBA8* color = colors + i;
		static auto ART = UpiEngine::ResourceManager::getTexture("spritesheet.png").id;
		spriteBatch->draw(glm::vec4{ body->x - 20, body->y - 20, 40, 40 }, copy->uvs[i], ART, 1.0f, *color);
	}

	Bulletst* bullets = &copy->bullets;
	for (int i = 0; i < bullets->count; i++)
	{
		const auto& pos = (bullets->bulletBodies + i)->position;
		static auto notWorking = UpiEngine::ResourceManager::getTexture("pixel.png").id;
		static auto yellow = UpiEngine::ColorRGBA8(255, 255, 0, 255);
		spriteBatch->draw(glm::vec4{ pos.x - 3.f, pos.y - 3.f, 6.f, 6.f }, glm::vec4{ 0.f, 0.f, 1.f, 1.f },
			notWorking, 1.0f, yellow); // depth !
	}

	if (debugDraw)
	{
		int startI = copy->mouseY / cellSize;
		int startJ = copy->mouseX / cellSize;
		for (int i = startI - 50; i < startI + 50; i++)
		{
			for (int j = startJ - 50; j < startJ + 50; j++)
			{
				glm::vec4 box(j * cellSize, i * cellSize, cellSize, cellSize);
				Debug::drawBox(box);
			}
		}
	}

	copy->player.player.selectionRect.DrawRect();
	// printf("%f\n", copy->player.player.selectionRect.x);
}

int main(int argc, char* argv[])
{
	gameStateCopy* copy = new gameStateCopy; // ? ? ?
	copy->bodiesCopy = 0;

	// update()
	int windowWidth = 1280;
	int windowHeight = 720;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);  // instanced

	Uint32 flags = SDL_WINDOW_OPENGL;
	SDL_Window* window = SDL_CreateWindow("Age of Empires I Definitive Edition", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, flags);

	if (!window)
	{
		debugBreak();
	}
	SDL_GLContext glContex = SDL_GL_CreateContext(window);
	if (!glContex)
	{
		debugBreak();
	}

	SDL_GL_SetSwapInterval(0);

	if (!gladLoadGL())
	{
		printf("failed to initaliaze GLAD\n");
		debugBreak();
		return -1;
	}

	printf("*** OpenGL Version: %s  ***\n", glGetString(GL_VERSION));

	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(114.f / 255.0f, 144.f / 255.0f, 154.f / 255.0f, 1.0f);


	initShaders(textureProgram);

	UpiEngine::SpriteBatch spriteBatch;
	spriteBatch.init();
	camera2D.init(windowWidth, windowHeight);
	camera2D.setPosition(glm::vec2(windowWidth / 2, windowHeight / 2));

	UpiEngine::Camera2D hudCamera;
	hudCamera.init(windowWidth, windowHeight);
	hudCamera.setPosition(glm::vec2(windowWidth / 2, windowHeight / 2));
	UpiEngine::SpriteBatch hudSpriteBatch;
	hudSpriteBatch.init();

	ImGui_ImplSdlGL3_Init(window);

	// TODO: asdlkf color
	ImVec4 clear_color = ImColor(114, 144, 154);

	char buf[256];
	GetCurrentDirectory(256, buf);
	LoadGameDLL();


	LPVOID baseAddress = (LPVOID)teraBytes(2);
	PlaybackState inputState{};

	game_memory gameMemory = {};
	gameMemory.permanentStorageSize = megaBytes(128);
	gameMemory.transientStorageSize = gigaBytes((uint64_t)1);
	uint64_t totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;

	gameMemory.permanentStorage = // malloc(totalSize);
		VirtualAlloc(baseAddress, (size_t)totalSize, // KIITOS virtualAlloc
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	gameMemory.transientStorage = ((uint8_t*)gameMemory.permanentStorage + gameMemory.permanentStorageSize);
	inputState.memory = gameMemory.permanentStorage;
	inputState.totalMemorySize = megaBytes(128); // + gigaBytes(1);


	// TODO: Terminate thread
	std::atomic<bool> fileLoadingThreadDone(false);
	std::thread thread([&fileLoadingThreadDone, &inputState] {
		printf("start!\n");
		load_thread(&inputState);
		printf("done!\n");
		fileLoadingThreadDone = true;
	});


	InputManager inputManager = {};

	//////////////////////////////////////////////
	float currentSlice{ 1.1f };
	float lastFT{ 0.f };
	float  FT_STEP{ 1.0f };

	// WHAT THE FUCK!
	initFileWatch(gAssetPath);
	int total = gAssetFileInfo.textureCount = gAssetFileTimes.size();
	initFileWatch(gMusicPath);
	gAssetFileInfo.musicCount = (gAssetFileTimes.size() - total);
	total += gAssetFileInfo.musicCount;
	initFileWatch(gEffectPath);
	gAssetFileInfo.effectCount = gAssetFileTimes.size() - total;

	// core ?
	EngineCore core = {};
	core.spriteBatch = &spriteBatch;
	core.input = &inputManager;
	core.memory = &gameMemory;
	core.deltaTime = FT_STEP;
	core.screenHeight = SCREEN_WIDTH;
	core.screenWidth = SCREEN_HEIGHT;

	core.glcontext = &glContex;
	core.window = window;
	// core.backBuffer = backBuffer;
	core.pause = false;
	core.advanceNextFrame = false;
	core.advancedLastFrame = false;
	core.screenWidth = windowWidth;
	core.screenHeight = windowHeight;
	core.camera2D = &camera2D;
	UpiEngine::ResourceManager::init();
	core.ctx.textureCacheCtx = UpiEngine::ResourceManager::GetContext();


	// TODO: korjaa resurssit
	game_state *gameState = (game_state*)core.memory->permanentStorage;


	core.resources.FreeTexture = FreeTexture;
	core.resources.SurfaceToGlTexture = TurnSurfaceIntoGlTexture;
	core.resources.LoadSurface = LoadSurface;

	UpiEngine::DebugRenderer debugger;
	debugger.init();
	core.debugger = &debugger;
	Debug::_Debugger = &debugger;
	// huom deubbgslgjal

	core.filewatcher.init(luaFiles, LUAFILESCOUNT, Resource_script);
	// "lua/main3.lua", "lua/main2.lua"

	bool quit = false;

	std::chrono::nanoseconds simulationTime(0);
	std::chrono::nanoseconds timestep(16);
	auto currentTime = std::chrono::high_resolution_clock::now();

	double t = 0.0;
	double dt = 1 / 60.0;
	currentTime = std::chrono::high_resolution_clock::now();


	NesInput nesInput{};
	SimpleInputRec simpleRecorder{};
	simpleRecorder._nesinput = &nesInput;


	// Lua Scripting Init****************************************
	static lua_State *L = luaL_newstate();
	core.script.L = L;
	static const char* lua_main_filename = "lua/main2.lua";
	{
		if (!L)
		{
			printf("could not create lua_state");
			debugBreak();
			exit(1);
		}
		luaL_openlibs(L);

		int succ = luaL_loadfile(L, lua_main_filename);
		lua_setglobal(L, "main_function");

		if (succ == 0)
		{
			printf("Lua file: %s loaded succesfully!\n", lua_main_filename);
		}
		else
		{
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			debugBreak();
			exit(1);
		}
	}

	// lua_getglobal(L, "main_function");
	// char buffer[512];
	// int succ = lua_pcall(L, 0, 0, 0);

	//if (succ != 0)
	//{
	//	sprintf(buffer, "[error]: %s\n", lua_tostring(L, -1));
	//	printf("%s", buffer);
	//	core.AddToConsole(buffer);
	//	//	debugBreak();
	//}
	//else
	//{
	//	lua_settop(L, 0);
	//}

	// lua_getglobal(L, "testiPrinter");
	// lua_pcall(L, 0, 0, 0);
	// lua_settop(L, 0);
	{
		auto asdf = UpiEngine::ResourceManager::getTexture("tekstuuri");
		core.filewatcher.init(shaderFiles, SHADERFILECOUNT, Resource_shader);
	}
	GLuint randomTexture = UpiEngine::ResourceManager::getTexture("test.png").id;
	{
		auto b = UpiEngine::ResourceManager::getTexture("building.png");

		auto notWorking = UpiEngine::ResourceManager::getTexture("pixel.png");
		auto notWorking2 = UpiEngine::ResourceManager::getTexture("Archer.png");
	}
	auto realTime(std::chrono::high_resolution_clock::now());
	auto simulationTIMER(std::chrono::high_resolution_clock::now());

	bool firstFrame = true;
	while (!quit)
	{
		auto timePoint1(std::chrono::high_resolution_clock::now());

		auto delta_time = std::chrono::high_resolution_clock::now() - currentTime;
		currentTime = std::chrono::high_resolution_clock::now();
		simulationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(delta_time);

		// TODO: thread pool
		if (fileLoadingThreadDone)
		{
			printf("thread done!\n");
			thread.join();
			fileLoadingThreadDone = false;
		}

		ImGui_ImplSdlGL3_NewFrame(window);
		SDL_Event ev;
		while (SDL_PollEvent(&ev))
		{
			ImGui_ImplSdlGL3_ProcessEvent(&ev);

			switch (ev.type)
			{
			case SDL_QUIT:
			{
				quit = true;
			} break;
			case SDL_MOUSEMOTION:
			{
				inputManager.rawMouse.x = ev.motion.x;
				inputManager.rawMouse.y = ev.motion.y;
			} break;
			case SDL_MOUSEBUTTONDOWN:
			{
				inputManager.pressMouse(ev.button.button);
			} break;
			case SDL_MOUSEBUTTONUP:
			{
				inputManager.releaseMouse(ev.button.button);
			} break;
			case SDL_KEYDOWN:
			{
				inputManager.pressKey(ev.key.keysym.scancode);

				if (ev.key.keysym.scancode > 0 && ev.key.keysym.scancode < SDL_SCANCODE_3)
				{
					nesInput.buttons |= 1 << ev.key.keysym.scancode; // keydown
				}

				if (ev.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					if (!inputState.recording && !inputState.playing) // eka
					{
						printf("record");
						startRecordInput(&inputManager, &inputState, 0);
						inputState.recording = true;
					}
					else if (inputState.recording) // toka
					{
						printf("playe");
						endRecording(&inputState);
						startInputPlayback(&inputManager, &inputState, 0);
						inputState.recording = false;
						inputState.playing = true;
					}
					else
					{
						// start recording second slot?
						// or change recording with shift + 1
					}
				}
				else if (ev.key.keysym.scancode == SDL_SCANCODE_F2)
				{
#if 0
					if (inputState.playing)
					{
						// endplayBack(&inputState);
						inputState.playing = false;
						inputManager.reset();
				}
					else
					{
						inputState.playing = true;
						inputManager.reset();
					}
#else
					inputState.playing = !inputState.playing;
					inputManager.reset();
#endif
			}
			} break;
			case SDL_KEYUP:
			{
				inputManager.releaseKey(ev.key.keysym.scancode);

				if (ev.key.keysym.scancode > 0 && ev.key.keysym.scancode < SDL_SCANCODE_3)
				{
					nesInput.buttons &= ~(1 << ev.key.keysym.scancode);
				}
			} break;
		}
	} // POLL EvENTS

			// number |= 1 << x;   // setting bit 
			// number &= ~(1 << x); // clear 

			// nesInput.buttons = 0xFF;

		if (inputManager.isKeyPressed(SDL_SCANCODE_ESCAPE))
		{
			quit = true;
		}

		if (core.filewatcher.update(Resource_shader))
		{
			UpiEngine::GLSLProgram newshader;
			if (initShaders(newshader))
			{
				textureProgram.unuse();
				textureProgram.dispose();
				textureProgram = newshader;
				printf("shader loaded!");
			}
			else
				printf("shader loading failed!");
		}

		const char* playbackFileDir = "../TEST_ENUMERATE/Playback/";
		if (inputManager.isKeyPressed(SDL_SCANCODE_G) && false)
		{
			simpleRecorder.LoadFile("../TEST_ENUMERATE/Playback/_autorec.rec");
			printf("playback");
		}
		else if (inputManager.isKeyPressed(SDL_SCANCODE_B) && false)
		{
			// löydä eka tiedosto nimi, joka on vapaa
			std::string path = "../TEST_ENUMERATE/Playback/";
			std::string name = "_autorec";
			std::string end = ".rec";

			// TODO: toimii vain 10 kpl 
			std::vector<std::string> filenames = get_all_files_names_within_folder(playbackFileDir);
			LoopPtr(&core); // GAME LOOP    // tarvitsen sijainnit tai komennot renderointiin ?
			int recordNum = 0;
			std::string recordName = "";
			while (1)
			{
				recordName = name + std::to_string(recordNum) + end;
				if (filenames.size() == recordNum || filenames[recordNum] != recordName)
				{
					simpleRecorder.WriteToFile((path + recordName).c_str());
					break;
				}

				recordNum++;
			}

			printf("saved as %s\n", recordName.c_str());
		}
		else if (inputManager.isKeyPressed(SDL_SCANCODE_X) && false)
		{
			simpleRecorder.Reset(); // starts new record
		}

		if (!firstFrame)
			//Sleep(1); // program runs too fast input bug
			firstFrame = false;

		simpleRecorder.Update();
		ImguiTest(clear_color, &core);

		auto context = ImGui::GetCurrentContext();
		ImguiPtr(&core, context, (void*)(&gAssetFileTimes), &gAssetFileInfo);



		// if (inputManager.isKeyPressed(SDL_SCANCODE_T))
		// {
			// system("waitfor /si build");
		// }

		FILETIME newTime = Win32GetLastWriteTime("game.dll");
		if (CompareFileTime(&newTime, &GameDLLWriteTime))
		{
			printf("loaded!\n");
			LoadGameDLL();
		}

		// FILEUPDATE*************************************************************
#if 0
		if (const char* updatedFile = core.filewatcher.update(Resource_script))
		{
			static char buffer[2048]; // mainin alkuun iso random buffer?
			int succ = luaL_loadfile(L, updatedFile);

			if (succ == 0)
			{
				lua_setglobal(L, "loaded");
				lua_getglobal(L, "loaded");
				succ = lua_pcall(L, 0, 0, 0);

				if (succ == 0)
				{
					sprintf(buffer, "Lua file: %s loaded succesfully!\n", updatedFile);
					printf("%s", buffer);
					core.AddToConsole(buffer);
				}
				else
				{
					sprintf(buffer, "[error]: %s\n", lua_tostring(L, -1));
					printf("%s", buffer);
					core.AddToConsole(buffer);
					debugBreak();
				}
			}
			else
			{
				sprintf(buffer, "[error]: %s\n", lua_tostring(L, -1));
				printf("%s", buffer);
				core.AddToConsole(buffer);
				debugBreak();
			}
}
#endif
		// fprintf(stderr, "%s\n", lua_tostring(L, -1));

		int steps = 0;
		currentSlice += lastFT;

		// while(simulationTime >= timestep)
		core.deltaTime = (1.f / 60.f) * g_gameSpeed;
		core.timeMultiplier = g_gameSpeed;


		// Wow that's a huge loop!     main loop 0
#if 0

		int updates = 0;
		for (; currentSlice >= FT_SLICE; currentSlice -= FT_SLICE)	// hud input should be polled!!!
		{
			if (updates > 10)
			{
				break;
			}
			// printf("jarraut++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
			// break;


			simulationTime -= timestep;
			updates++;

			if (!core.pause || core.advanceNextFrame)
			{
				for (int i = 0; i < g_frameAdvanceCount; i++)
				{
					if (inputState.recording)
					{
						recordInput(&inputManager, &inputState);
					}
					else if (inputState.playing)
					{
						playBackInput(&inputManager, &inputState);
					}

					LoopPtr(&core); // GAME LOOP
					g_currentFrame++;

					if (core.beginSkipToFrame)
					{
						if (core.skipToFrame == g_currentFrame)
						{
							core.beginSkipToFrame = false;
							core.pause = true;
							g_frameAdvanceCount = 1;
							break;
						}
						else
						{
							g_frameAdvanceCount = 50;  // laske max simulointi nopeus skipaten framet, jotenkinh smooth
						}
					}

					inputManager.mouse = camera2D.convertScreenToWorld(core.input->rawMouse);
					inputManager.update(); // TODO: maybe poll for input
				}
				core.advanceNextFrame = false;
			}
		}

#else

		realTime = std::chrono::high_resolution_clock::now();
		int updates = 0;

		if (inputManager.isKeyDown(SDL_SCANCODE_E)) // input
		{
			camera2D.setScale(camera2D.getScale() - 0.0005f); // TODO: delta broken
			if (camera2D.getScale() < 0.01f)
			{
				camera2D.setScale(0.01f);
			}
		}
		if (inputManager.isKeyDown(SDL_SCANCODE_Q))
		{
			camera2D.setScale(camera2D.getScale() + 0.0005f);
		}

		while (simulationTIMER < realTime || core.beginSkipToFrame)
		{
			static bool threadInit = false;
			simulationTIMER += std::chrono::milliseconds(16); //Timeslice is ALWAYS 16ms. 

			if (threadInit)
				cond.notify_all();

			// LoopPtr(&core);
			// printf("Update %i \n", updates);
			bool allowBreak = false;
			if (core.pause)
				allowBreak = true;

			if (updates > 1)
				printf("probably not working!\n");


			if (!core.pause || core.advanceNextFrame)
			{
				for (int i = 0; i < g_frameAdvanceCount; i++)
				{
					if (g_frameAdvanceCount > 1)
					{
						simulationTIMER = std::chrono::high_resolution_clock::now();
						allowBreak = true;
					}

					if (inputState.recording)
					{
						recordInput(&inputManager, &inputState);
					}
					else if (inputState.playing)
					{
						playBackInput(&inputManager, &inputState);
					}


					if (!threadInit) // aka ei montaa framea per renders
					{
						logicThread = std::thread(LogicThreadUpdate, &core); // calc next frame
						join = true;
						// LoopPtr(&core); // GAME LOOP    // tarvitsen sijainnit tai komennot renderointiin ?
						threadInit = true;
					}

					g_currentFrame++;

					//	if (core.beginSkipToFrame)
					//	{
					//		if (core.skipToFrame == g_currentFrame)
					//		{
					//			core.beginSkipToFrame = false;
					//			core.pause = true;
					//			g_frameAdvanceCount = 1;
					//			break;
					//		}
					//		else
					//		{
					//			g_frameAdvanceCount = 100;  // laske max simulointi nopeus skipaten framet, jotenkinh smooth
						//	}
						//}

			//			inputManager.mouse = camera2D.convertScreenToWorld(core.input->rawMouse);
			//			inputManager.update(); // TODO: maybe poll for input
				}
				//	core.advanceNextFrame = false;
			}

			if (allowBreak)
			{
				break;
			}
		}

		//			RenderWorld();
#endif



		// printf("%f\n", camera2D.getScale());
		ImGui::Text("camera: %f", camera2D.getScale());

		camera2D.update();
		hudCamera.update();

		// glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		textureProgram.use();

		glActiveTexture(GL_TEXTURE0);
		GLint textureLocation = textureProgram.getUniformLocation("enemySampler");
		glUniform1i(textureLocation, 0);

		https://www.latex-project.org/
	// httpss://www.latex-project.org2/

		GLint plocation = textureProgram.getUniformLocation("P");
		glm::mat4 cameraMatrix = camera2D.getCameraMatrix();
		glUniformMatrix4fv(plocation, 1, GL_FALSE, &(cameraMatrix[0][0]));

		spriteBatch.begin(UpiEngine::GlyphSortType::BACK_TO_FRONT);
		spriteBatch.draw(glm::vec4{ 200.f, 100.f, 40.f, 40.f }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, randomTexture, 1.0f);

		// DrawPtr(&core);
		if (copy->bodiesCopy)
		{
			spriteBatch.draw(copy->dimensions, glm::vec4{ 0.f, 0.f, 1.f, 1.f }, copy->mapCopy, 1.0f); // map draw
			threadedDraw(copy->bodiesCopy, colorCopies, copy->copiesCount, &spriteBatch, copy);
		}

		spriteBatch.end();
		spriteBatch.renderBatch();

		//*************************************debug*******************************
#if 1  // debuggeriin voi olla hyvin vaikea koskea
		glm::mat4 hudCameraMatrix = hudCamera.getCameraMatrix();
		glUniformMatrix4fv(plocation, 1, GL_FALSE, &(hudCameraMatrix[0][0]));

		hudSpriteBatch.begin();
		// debugger._dDrawSlots(hudSpriteBatch);        // debuggeriin 
		hudSpriteBatch.end();

		hudSpriteBatch.renderBatch();

		glBindTexture(GL_TEXTURE_2D, 0);
		textureProgram.unuse();


		debugger.render(cameraMatrix, 2.0f);
		debugger.end();
#endif
		ImGui::Render();
		SDL_GL_SwapWindow(window);


		auto timePoint2(std::chrono::high_resolution_clock::now());
		auto elapsedTime(timePoint2 - timePoint1);
		float ft = { std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(elapsedTime).count() };
		lastFT = ft;
		float ftSeconds = (ft / 1000.f);


		// SYNC
		std::unique_lock<std::mutex> locker(logicMutex);

		{
			game_state *gameState = (game_state*)core.memory->permanentStorage;

			memcpy(gameState->threadShared.lastFrame, gameState->bodies, sizeof(PhysicsBody) * gameState->currentEntityCount);

			memcpy(colorCopies, gameState->entityColors, sizeof(UpiEngine::ColorRGBA8) * gameState->currentEntityCount);
			copy->copiesCount = gameState->currentEntityCount;
			copy->bodiesCopy = gameState->threadShared.lastFrame;

			SyncBullets(&copy->bullets, gameState);

			memcpy(copy->uvs, gameState->unitAnimations.uvs, sizeof(glm::vec4) * gameState->currentEntityCount);


			if (gameState->worldmap.visual.surface)
			{
				FreeTexture(&copy->mapCopy);
				copy->mapCopy = TurnSurfaceIntoGlTexture(gameState->worldmap.visual.surface);
				copy->dimensions = gameState->worldmap.dimensions;
				memcpy(&copy->player, gameState->player, sizeof(Entity));
			}

			copy->mouseX = core.input->mouse.x;
			copy->mouseY = core.input->mouse.y;

			if (gameState->player && gameState->player->player.selectingTroops)
				gameState->player->player.selectionRect.DrawRect();
		}

		locker.unlock();
	}

	logicThreadRunning = false;
	cond.notify_all();
	logicThread.join();

	ImGui_ImplSdlGL3_Shutdown();
	SDL_GL_DeleteContext(&core.ctx);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

