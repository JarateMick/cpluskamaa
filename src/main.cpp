#define VC_EXTRALEAN 1
#define _MATH_DEFINES_DEFINED 1
#define WIN32_LEAN_AND_MEAN 1

#include <chrono>
#include <lua.hpp>
#include <Windows.h>

#include <string>
#include <vector>
#include <stdio.h>

#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "Imgui/imgui_impl_sdl_gl3.cpp"
#include "Imgui/imgui_demo.cpp"

#include "TextureHolder.h"
#include <cstdarg>
#include <process.h>

#include <atomic>

// #include "imgui/imgui_internal.h"
#include "game.h" 
#include "imguiTools.h"


//typedef struct  
//{
//	unsigned char buttons;
//} nesinput;

#include "InputRecorder.cpp"
struct TextureFile
{
	std::string name;
	std::string path; // iso overhead atm mutta ehk� tulevaisuudessa -> ei tarvisi
	FILETIME lastFileTime;
};


#define internal static
std::vector<TextureFile> gAssetFileTimes;

typedef void(*LoopType)(EngineCore*);
typedef void(*DrawType)(EngineCore*);

// typedef void(*ImguiType)(EngineCore*, ImGuiContext*, void*, AssetFileInfo*);
Imgui_func* ImguiPtr;

// #define IMGUIFUNC(name) void name(EngineCore* core, ImGuiContext* context, void* tmpData, AssetFileInfo* assetInfo)

LoopType LoopPtr;
DrawType DrawPtr;
HMODULE GameDLL;
HMODULE ImguiDLL;
FILETIME GameDLLWriteTime;

int SCREEN_WIDTH = 1200;
int SCREEN_HEIGHT = 1000;

internal void UnloadGameDLL()
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
internal void LoadGameDLL()
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


internal std::vector<std::string> get_all_files_names_within_folder(std::string folder)
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
// MATH UTILS

//internal void poll(SDL_Event &e, InputManager& inputManager)
//{
//	while (SDL_PollEvent(&e) != 0)
//	{
//		//User requests quit
//		if (e.type == SDL_QUIT)
//		{
//			// quit = true;
//		}
//		if (e.type == SDL_MOUSEBUTTONDOWN)
//		{
//			inputManager.pressMouse(e.button.button);
//		}
//		if (e.type == SDL_MOUSEBUTTONUP)
//		{
//			inputManager.releaseMouse(e.button.button);
//		}
//		if (e.type == SDL_MOUSEMOTION)
//		{
//			inputManager.mouseX = e.motion.x;
//			inputManager.mouseY = e.motion.y;
//		}
//		if (e.type == SDL_KEYDOWN)
//		{
//			inputManager.pressKey(e.key.keysym.scancode);
//		}
//		if (e.type == SDL_KEYUP)
//		{
//			inputManager.releaseKey(e.key.keysym.scancode);
//		}
//	}
//}
//

// #include "game.h"
static AssetFileInfo gAssetFileInfo;

internal void pollTextureFiles()
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
internal void pollForNewFiles()
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

internal void initFileWatch(std::string AssetPath)
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

internal std::vector<std::string> GetAllFilePathsInFolder(std::string AssetPath)
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

internal void
getInputRecordName(int index, char* filename, bool state)
{
	sprintf(filename, "input_%s_%i.input", state ? "state" : "stream", index);
}

internal void
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

internal void
recordInput(InputManager* manager, PlaybackState* state)
{
	fwrite(manager, sizeof(InputManager), 1, state->recordingFile);
}

internal void
endRecording(PlaybackState* state)
{
	fclose(state->recordingFile);
}

internal void
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

internal void
endplayBack(PlaybackState* state)
{
	fclose(state->playingFile);
}

internal void
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


bool circleContains(float x, float y, float r, Entity* e)
{
	if (pow((x - e->x), 2) + pow((y - e->y), 2) < pow(r, 2)) // entity on sisällä
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
		if (core->input->isKeyDown(SDL_SCANCODE_Y))
		{
			// looppaa kaikki läpi katso onko lähistöllä entityä
			ImGui::Text("Current Entity count: %i", gameState->currentEntityCount);
			for (int i = 0; i < gameState->currentEntityCount; i++)
			{
				Entity* e = &gameState->entities[i];
				if (circleContains(core->input->mouse.x, core->input->mouse.y, 20.f, e))
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
			ImGui::Text("Pos x:%f y:%f", selected->x, selected->y);
			ImGui::DragFloat("x", &selected->x);
			ImGui::DragFloat("y", &selected->y);

			if (core->input->isMouseDown(1) && circleContains(core->input->mouse.x, core->input->mouse.y, 40.f, selected))
			{
				selected->x = core->input->mouse.x;
				selected->y = core->input->mouse.y;
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

	auto context = ImGui::GetCurrentContext();
	ImguiPtr(core, context, (void*)(&gAssetFileTimes), &gAssetFileInfo);
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

#include <thread>

// number |= 1 << x;   // setting
// number &= ~(1 << x); // clear

// number ^= 1 << x; // togle
// bit = (number >> x) & 1; // checking
// number ^= (-x ^ number) & (1 << n); // setting 1 or 0

#include <bitset>
#include <iostream>


 // #include <sol\sol.hpp>
 // #include "../sol/sol.hpp"

 // #include <lua.hpp>
 // #include <sol\sol.hpp>

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

// #include "gl/picoPNG.cpp"

// #include "glad.c" 

// #include "gl/TextureCache.cpp"
// #include "gl/ResourceManager.cpp"

// #include "gl/SpriteBatch.cpp"
// #include "gl/Camera2D.cpp"
// #include "gl/GLTexture.h"

// m_textureProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
// m_textureProgram.addAttribute("vertexPosition");
// m_textureProgram.addAttribute("vertexColor");
// m_textureProgram.addAttribute("vertexUV");
// m_textureProgram.linkShaders();


// doublebuffer
// glclearcolor
// ikkunan luonnin aika koodit

// spritebatch.init()
// hudspritebatch.init();
// camera init

UpiEngine::GLSLProgram textureProgram;
UpiEngine::Camera2D    camera2D;
void initShaders()
{
	textureProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
	textureProgram.addAttribute("vertexPosition");
	textureProgram.addAttribute("vertexColor");
	textureProgram.addAttribute("vertexUV");
	textureProgram.linkShaders();
}


#include "source.h"
int main(int argc, char* argv[])
{
	init(mode_client);
	update();
	cleanUp();

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
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

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

	initShaders();

	UpiEngine::SpriteBatch spriteBatch;
	spriteBatch.init();
	// spriteBatch.init();
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
	gameMemory.permanentStorageSize = megaBytes(64);
	gameMemory.transientStorageSize = gigaBytes((uint64_t)1);
	uint64_t totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;

	gameMemory.permanentStorage = // malloc(totalSize);
		VirtualAlloc(baseAddress, (size_t)totalSize, // KIITOS virtualAlloc
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	gameMemory.transientStorage = ((uint8_t*)gameMemory.permanentStorage + gameMemory.permanentStorageSize);
	inputState.memory = gameMemory.permanentStorage;
	inputState.totalMemorySize = megaBytes(64); // + gigaBytes(1);


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
	float FT_SLICE{ 1.0f };
	float  FT_STEP{ 1.0f };
	float ft = 0.f;
	float ftSeconds = 0.f;
#define avgFTSize 100
#define avgFpsSize 100
	float avgFT[avgFTSize];
	float avgFps[avgFpsSize];
	bool devConsole = false;

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
	// core.resources = &textureHolder; TODO: port to allegro5
	// core.window = gWindow; -> backBuffer
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

	UpiEngine::DebugRenderer debugger;
	debugger.init();
	core.debugger = &debugger;
	Debug::_Debugger = &debugger;
	// huom deubbgslgjal


	// al_start_timer(timer);
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



	UpiEngine::GLTexture asdf = UpiEngine::ResourceManager::getTexture("tileset2.png");
	GLuint ididididid = asdf.id;
	GLuint testId = UpiEngine::ResourceManager::getTexture("test.png").id;

	// TileSheet 
	UpiEngine::TileSheet tileset;
	tileset.init(asdf, { 10, 16 });
	core.testyTexture = &tileset;
	// core.testyTexture UpiEngine::ResourceManager::getTexture("tileset.png");

	core.slopeMapTexture = UpiEngine::ResourceManager::getTexture("slopemap.png").id;

	// Lua Scripting Init****************************************
	static bool init1 = false;
	static lua_State *L = luaL_newstate();
	core.script.L = L;
	static const char* lua_main_filename = "lua/main2.lua";
	{
		if (!L)
		{
			printf("could not create lua newstate");
			debugBreak();
			exit(1);
		}
		luaL_openlibs(L);

		int succ = luaL_loadfile(L, lua_main_filename);
		lua_setglobal(L, "main_function");


		if (succ == 0)
		{
			printf("Lua file: %s loaded succesfully!", lua_main_filename);
		}
		else
		{
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			debugBreak();
			exit(1);
		}
	}
	  lua_getglobal(L, "main_function");
	  lua_pcall(L, 0, 0, 0);

	// getchar();
	//
	// Lua Scripting Init****************************************

	// TODO: create console here!

	while (!quit)
	{
		auto timePoint1(std::chrono::high_resolution_clock::now());

		// reddit
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

		// number |= 1 << x;   // setting
		// number &= ~(1 << x); // clear

		// nesInput.buttons = 0xFF;

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

			// TODO: toimii vain 10 kpl XDDDDDDD
			std::vector<std::string> filenames = get_all_files_names_within_folder(playbackFileDir);
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

		simpleRecorder.Update();

		ImGui_ImplSdlGL3_NewFrame(window);
		ImguiTest(clear_color, &core);

		if (inputManager.isKeyPressed(SDL_SCANCODE_T))
		{
			system("waitfor /si build");
		}

		FILETIME newTime = Win32GetLastWriteTime("game.dll");
		if (CompareFileTime(&newTime, &GameDLLWriteTime))
		{
			printf("loaded!\n");
			LoadGameDLL();
		}
		if (core.filewatcher.update())
		{
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
				// exit(1);
			}
		}



		int steps = 0;
		currentSlice += lastFT;

		// while(simulationTime >= timestep)
		core.deltaTime = (1.f / 60.f) * g_gameSpeed;
		core.timeMultiplier = g_gameSpeed;


		// Wow that's a huge loop!

		int updates = 0;
		for (; currentSlice >= FT_SLICE; currentSlice -= FT_SLICE)	// hud input should be polled!
		{
			if (updates > 4) break;

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
						//	al_draw_rectangle(inputManager.mouseX - 2.f, inputManager.mouseY - 2.f, inputManager.mouseX + 2.f, inputManager.mouseY + 2.f, al_map_rgb(255, 0, 0), 2.0f);
							// al_set_mouse_xy(display, inputManager.mouseX, inputManager.mouseY);
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

		if (inputManager.isKeyDown(SDL_SCANCODE_E))
		{
			camera2D.setScale(camera2D.getScale() - 0.01f);
		}
		if (inputManager.isKeyDown(SDL_SCANCODE_Q))
		{
			camera2D.setScale(camera2D.getScale() + 0.01f);
		}
		camera2D.update();
		hudCamera.update();

		// glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		textureProgram.use();

		glActiveTexture(GL_TEXTURE0);
		GLint textureLocation = textureProgram.getUniformLocation("enemySampler");
		glUniform1i(textureLocation, 0);

		GLint plocation = textureProgram.getUniformLocation("P");
		glm::mat4 cameraMatrix = camera2D.getCameraMatrix();
		glUniformMatrix4fv(plocation, 1, GL_FALSE, &(cameraMatrix[0][0]));

		spriteBatch.begin(UpiEngine::GlyphSortType::BACK_TO_FRONT);

		DrawPtr(&core);

		spriteBatch.end();
		spriteBatch.renderBatch();



		//*************************************debug*******************************

		glm::mat4 hudCameraMatrix = hudCamera.getCameraMatrix();
		glUniformMatrix4fv(plocation, 1, GL_FALSE, &(hudCameraMatrix[0][0]));

		hudSpriteBatch.begin();
		debugger._dDrawSlots(hudSpriteBatch);
		hudSpriteBatch.end();

		hudSpriteBatch.renderBatch();

		glBindTexture(GL_TEXTURE_2D, 0);
		textureProgram.unuse();


		debugger.render(cameraMatrix, 2.0f);
		debugger.end();


		ImGui::Render();
		SDL_GL_SwapWindow(window);



		//if (ft != 0.f)
		//{
		//	auto fps(1.f / ftSeconds);

		//	static int counter = 0;
		//	avgFT[counter] = ft;
		//	avgFps[counter++] = fps;
		//	if (counter == avgFTSize - 1)
		//	{
		//		char buffer[64];
		//		float sum = 0;
		//		float fpsSum = 0;

		//		for (int i = 0; i < avgFTSize; i++)
		//		{
		//			sum += avgFT[i];
		//		}
		//		for (int i = 0; i < avgFpsSize; i++)
		//		{
		//			fpsSum += avgFps[i];
		//		}
		//		sum /= avgFTSize;
		//		fpsSum /= avgFpsSize;

		//		// sprintf_s(buffer, "Frametime: %f ms/frame, fps: %f\n", sum, fpsSum);
		//		// printf("%s", buffer);
		//		counter = 0;
		//	}
		//}

		// al_flip_display();
		// al_wait_for_vsync();

		auto timePoint2(std::chrono::high_resolution_clock::now());
		auto elapsedTime(timePoint2 - timePoint1);
		ft = { std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(elapsedTime).count() };
		lastFT = ft;
		ftSeconds = (ft / 1000.f);
		// printf("%f\n", ft);

	}

	ImGui_ImplSdlGL3_Shutdown();
	// SDL_GL_DeleteCOntext(glcontext);s	
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

