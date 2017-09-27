#include "imguiTools.h"
#include "imgui/imgui.h"
#include "fileSystem.cpp"
// #include "game.cpp"
#include "entity.cpp"
#include "core.h"
#include <map>
// #include "game.cpp"


// #include "utility.h"
// #include "imgui/imgui.h"

// structi johon tökättäisiin function pointerit
// pelin IO systeemeihin
// Nämä editor työkaluhin

static EngineCore* g_tools_core;

void cameraControls(game_state* gameState)
{
	ImGui::DragFloat("Camera speed", &gameState->cameraSpeed);
}

int str_cut(char *str, int begin, int len)
{
	int l = strlen(str);

	if (len < 0) len = l - begin;
	if (begin + len > l) len = l - begin;
	memmove(str + begin, str + begin + len, l - len + 1);

	return len;
}



void SaveProvinceId(game_state* gameState, int inputId)
{
	ProvinceData* data = &gameState->provinceData;
	WorldMap* map = &gameState->worldmap;
	map->editor.inputProvinceId = inputId;

	if (*data->currentCount < data->maxProvinces - 1)
	{
		// Uint32 color = reverse_nibbles(map->editor.inputProvinceId);

		auto rgba = ImGui::ColorConvertU32ToFloat4(map->editor.editorColor);
		// -> abgr formaattiin

		data->colorToId->insert(std::make_pair(map->editor.editorColor, map->editor.inputProvinceId));
		data->idToColor[map->editor.inputProvinceId] = map->editor.editorColor;
		data->positions[map->editor.inputProvinceId] = { (int)map->editor.inputX, (int)map->editor.inputY };

		if (*data->currentCount == map->editor.inputProvinceId)
			(*data->currentCount)++;

		printf("SAVETETTU!");
	}
}




// TODO: konsolin pitää olla osa corea!
// Tällä hetkellä se on vain yksittäisenä jossain bit avaruudessa
struct Console
{
	char                  InputBuf[256];
	ImVector<char*>       Items;
	bool                  ScrollToBottom;
	ImVector<char*>       History;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImVector<const char*> Commands;

	Console()
	{
		ClearLog();
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
		Commands.push_back("HELP");
		Commands.push_back("HISTORY");
		Commands.push_back("CLEAR");
		Commands.push_back("CLASSIFY");  // "classify" is here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
		// AddLog("Welcome to ImGui!");
	}
	~Console()
	{
		ClearLog();
		for (int i = 0; i < History.Size; i++)
			free(History[i]);
	}

	// Portable helpers
	static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
	static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
	static char* Strdup(const char *str) { size_t len = strlen(str) + 1; void* buff = malloc(len); return (char*)memcpy(buff, (const void*)str, len); }

	void    ClearLog()
	{
		for (int i = 0; i < Items.Size; i++)
			free(Items[i]);
		Items.clear();
		ScrollToBottom = true;
	}

	void    AddLog(const char* fmt, ...) IM_PRINTFARGS(2)
	{
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		Items.push_back(Strdup(buf));
		ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open)
	{
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}

		// ImGui::TextWrapped("This example implements a console with basic coloring, completion and history. A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
		ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

		// TODO: display items starting from the bottom

		if (ImGui::SmallButton("Add Dummy Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); } ImGui::SameLine();
		if (ImGui::SmallButton("Add Dummy Error")) AddLog("[error] something went wrong"); ImGui::SameLine();
		if (ImGui::SmallButton("Clear")) ClearLog(); ImGui::SameLine();
		if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom = true;
		//static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		static ImGuiTextFilter filter;
		filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::PopStyleVar();
		ImGui::Separator();

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
		// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
		// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
		//     ImGuiListClipper clipper(Items.Size);
		//     while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// However take note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
		// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		for (int i = 0; i < Items.Size; i++)
		{
			const char* item = Items[i];
			if (!filter.PassFilter(item))
				continue;
			ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // A better implementation may store a type per-item. For the sample let's just parse the text.
			if (strstr(item, "[error]")) col = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
			else if (strncmp(item, "# ", 2) == 0) col = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::TextUnformatted(item);
			ImGui::PopStyleColor();
		}
		if (ScrollToBottom)
			ImGui::SetScrollHere();
		ScrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
		{
			char* input_end = InputBuf + strlen(InputBuf);
			while (input_end > InputBuf && input_end[-1] == ' ') input_end--; *input_end = 0;
			if (InputBuf[0])
				ExecCommand(InputBuf);
			strcpy(InputBuf, "");
		}

		// Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

		ImGui::End();
	}

	void    ExecCommand(char* command_line)
	{
		AddLog("# %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = History.Size - 1; i >= 0; i--)
			if (Stricmp(History[i], command_line) == 0)
			{
				free(History[i]);
				History.erase(History.begin() + i);
				break;
			}
		History.push_back(Strdup(command_line));

		// Process command
		if (Stricmp(command_line, "CLEAR") == 0)
		{
			ClearLog();
		}
		else if (Stricmp(command_line, "HELP") == 0)
		{
			AddLog("Commands:");
			for (int i = 0; i < Commands.Size; i++)
				AddLog("- %s", Commands[i]);
		}
		else if (Stricmp(command_line, "HISTORY") == 0)
		{
			for (int i = History.Size >= 10 ? History.Size - 10 : 0; i < History.Size; i++)
				AddLog("%3d: %s\n", i, History[i]);
		}
		else if (strlen(command_line) > 0)
		{
			if (command_line[0] == ':')
			{
				int l = str_cut(command_line, 0, 1);
				const char* str = g_tools_core->script.executeCommand(command_line);
				AddLog("%s", str);
			}
		}
		else
		{
			AddLog("Unknown command: '%s'\n", command_line);
		}
	}

	static int TextEditCallbackStub(ImGuiTextEditCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	int     TextEditCallback(ImGuiTextEditCallbackData* data)
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			// Build a list of candidates
			ImVector<const char*> candidates;
			for (int i = 0; i < Commands.Size; i++)
				if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
					candidates.push_back(Commands[i]);

			if (candidates.Size == 0)
			{
				// No match
				AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
			}
			else if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				// List matches
				AddLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					AddLog("- %s\n", candidates[i]);
			}

			break;
		}
		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos)
			{
				data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (HistoryPos >= 0) ? History[HistoryPos] : "");
				data->BufDirty = true;
			}
		}
		}
		return 0;
	}
};

static Console& GetConsoleInstance()
{
	static Console console;
	return console;
}

static void ShowConsole(bool* p_open)
{
	GetConsoleInstance().Draw("Console", p_open);
}

void showFileType(FileWatcher* watcher, ResourceType type)
{
	resourceData* data = &watcher->resources[type];
	for (int i = 0; i < data->watchFilesCount; i++)
	{
		ImGui::Text("%s", data->filepathsToWatch[i].c_str());
	}
}

unsigned int reverse_nibbles(unsigned int x)
{
	unsigned int out = 0, i;
	for (i = 0; i < 4; ++i)
	{
		const unsigned int byte = (x >> 8 * i) & 0xff;
		out |= byte << (24 - 8 * i);
	}
	return out;
}

void ShowFileWatcher(FileWatcher* filewatcher)
{
	ImGui::Begin("Filewatcher:");

	ImGui::Text("Scripts: ");
	showFileType(filewatcher, Resource_script);

	// ImGui::Text("Textures: ");
	// showFileType(filewatcher, Resource_script);
	ImGui::Separator();

	ImGui::Text("Shaders: ");
	showFileType(filewatcher, Resource_shader);


	ImGui::End();
}

void AddToConsole(const char* text)
{
	GetConsoleInstance().AddLog("%s", text);
}

void VisualizeNodes(std::vector<int>* path, int startId, game_state* state)
{
	if (path->size() <= 0)
	{
		printf("no path\n");
		return;
	}

	v2 startPos = state->provinceData.positions[startId];
	Debug::drawBox({ startPos.x, startPos.y, 20, 20 });

	for (int i = path->size() - 1; i > -1; i--)
	{
		int provinceID = path->at(i);
		auto v2 = state->provinceData.positions[provinceID];

		Debug::drawLine({ (float)startPos.x, (float)startPos.y }, { (float)v2.x, (float)v2.y });
		Debug::drawBox({ v2.x, v2.y, 20, 20 });

		// printf("(%i, %i)\n", v2.x, v2.y);
	}
}

EXPORT IMGUIFUNC(Imgui)
{
	ImGui::SetCurrentContext(context);
	game_state *gameState = (game_state*)core->memory->permanentStorage;
	// ImGui_ImplSdlGL3_NewFrame(window);

	static char buffer[248];
	DefineInput(core);

	Debug::_Debugger = core->debugger;

	// Debug::console = &GetConsoleInstance().AddLog;
	static bool init = false;
	if (!init)
	{
		core->AddToConsole = AddToConsole;
	}

	g_tools_core = core;

	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);

	bool open = true;
	static bool no_titlebar = false;
	static bool no_border = true;
	static bool no_resize = false;
	static bool no_move = false;
	static bool no_scrollbar = false;
	static bool no_collapse = false;
	static bool no_menu = false;

	// Demonstrate the various window flags. Typically you would just use the default.
	ImGuiWindowFlags window_flags = 0;
	if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
	if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
	if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
	if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
	if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;

	ImGui::Begin("hello there!", &open, window_flags);
	ImGui::Text("Eetu keranene");
	ImGui::Separator();
	ImGui::Text("hell2 %s, %f", "string", 100.f);

	// ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiSetCond_FirstUseEver);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Valikko"))
		{
			ImGui::MenuItem("eka", NULL, false, false);
			if (ImGui::MenuItem("uusi tiedotso"))
			{
				// yeah
			}
			ImGui::EndMenu();
		}
		bool kamitSelected = false;
		if (ImGui::BeginMenu("help desk"))
		{
			ImGui::MenuItem("kamit", "ctr-t", &kamitSelected);
			ImGui::MenuItem("apua");
			ImGui::MenuItem("lumpsi on paras");

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("aapin pintaala asetukset"))
	{
		ImGui::BulletText("terveisia alabamasta!");
	}
	if (ImGui::CollapsingHeader("Buttons"))
	{
		bool seuraava = false;
		ImGui::Checkbox("buttons", &seuraava); ImGui::SameLine(150);
		ImGui::Checkbox("black button", &seuraava);

		if (ImGui::TreeNode("tyylikas"))
		{
			float value = 0;
			ImGui::PushItemWidth(100);
			ImGui::DragFloat("kurwa pierdole", &value, 0.0f, 20.0f, 100.f);

			ImGui::PopItemWidth();
			ImGui::TreePop();
		}
	}

	//// tänne temps
	//int size = ((std::vector<TextureFile>*)tmpData)->size();
	//TextureFile* dataArray = ((std::vector<TextureFile>*)tmpData)->data();

	//static int selected = -1; // Gets resetted when .dll
	//if (ImGui::CollapsingHeader("Columpsit"))
	//{
	//	ImGui::Text("{borders}");

	//	ImGui::Columns(3, "mycolums3", true);
	//	ImGui::Separator();
	//	ImGui::Text("id"); ImGui::NextColumn();
	//	ImGui::Text("nimi"); ImGui::NextColumn();
	//	ImGui::Text("relative path"); ImGui::NextColumn();
	//	//  ImGui::Text("path"); ImGui::NextColumn();
	//	ImGui::Separator();

	//	const char* names[3] = { "One", "two", "three" };
	//	const char* paths[3] = { "1", "2", "3" };

	//	for (int i = 0; i < size; ++i)
	//	{
	//		char label[32];

	//		sprintf_s(label, "%04d", i);
	//		if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
	//		{
	//			selected = i;
	//		}
	//		ImGui::NextColumn();
	//		ImGui::Text(dataArray[i].name.c_str()); ImGui::NextColumn();
	//		ImGui::Text(dataArray[i].path.c_str()); ImGui::NextColumn();
	//		// ImGui::Text("%f", dataArray[i].lastFileTime);
	//		// ImGui::Text("....");   ImGui::NextColumn();
	//	}
	//	ImGui::Columns(1);
	//	ImGui::Separator();

	//	// columpsit buttons
	//	ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
	//	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(1 / 7.0f, 0.7f, 0.7f));
	//	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(1 / 7.0f, 0.8f, 0.8f));
	//	if (ImGui::Button("nappi"))
	//	{
	//		printf("hei\n");
	//	}

	//	static char strNameOfFile[128];
	//	static char contents[128];
	//	ImGui::InputText("tiedoston nimi.txt", strNameOfFile, IM_ARRAYSIZE(strNameOfFile));
	//	ImGui::InputText("contents", contents, IM_ARRAYSIZE(contents));
	//	if (ImGui::Button("kirjoita tiedosto"))
	//	{
	//		printf("%i\n", selected);

	//		writeEntireFile(contents, IM_ARRAYSIZE(contents), strNameOfFile);
	//	}
	//	static bool showFileButtonToggle = true;
	//	if (ImGui::Button("nayta tiedosto"))
	//	{
	//		// set tooltip tai beginTooltip() ja endTooltip pari, jolla saa fancy tooltippejä!
	//		showFileButtonToggle = !showFileButtonToggle;
	//		char *tmp = 0;
	//		int tmpSize = 0;
	//		tmp = readEntireFile(strNameOfFile, tmpSize);

	//		int t = strlen(tmp);
	//		memcpy(contents, tmp, tmpSize);
	//		free(tmp);
	//	}
	//	if (ImGui::IsItemHovered())
	//		ImGui::SetTooltip("nayttaa tiedoston");

	//	if (ImGui::Button("kirjoita cpp"))
	//	{
	//		createTextureEnums(dataArray, assetInfo->textureCount, assetInfo);
	//	}

	//	if (ImGui::Button("Play selected music"))
	//	{
	//		if (selected > assetInfo->textureCount - 1)
	//		{
	//			// play music
	//		}
	//	}

	//	// open .png
	//	// preview .png

	//	if (showFileButtonToggle)
	//	{
	//		ImGui::PushItemWidth(150);
	//		static float arr[] = { 0.5f, 0.3f, 0.8f, 0.9f, 0.0f, 1.5f, 2.5f, 0.8f };
	//		ImGui::PlotLines("Kurwa!", arr, IM_ARRAYSIZE(arr));
	//		ImGui::PopItemWidth();
	//	}
	//	ImGui::PopStyleColor(3);


	//} // columpsissa


	ImGui::End();


	// ImGui::Text("Welcome to the Profiler");
	// ImGui::ProgressBar(0.2f, ImVec2(0.0f, 0.0f));

	game_state* state = (game_state*)core->memory->permanentStorage;

	ImGui::Begin("Profiler", &open, window_flags);
	if (core->memory->isInitialized)
	{
		memory_arena* Arena = &state->arena;
		double prog = ((double)state->arena.used / (double)Arena->size);

		char buf[32];
		sprintf(buf, "%d/%d", (int)(state->arena.used), (int)Arena->size);
		ImGui::ProgressBar(prog, ImVec2(0.f, 0.f), buf);
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("Memory used");
	}
	ImGui::End();

	//  ImGui::Begin("Playback", &open, window_flags);
	//  ImGui::Text("Welcome to playback");
	//  ImGui::End();


	cameraControls(state);

	static bool consoleOpen = true;
	ShowConsole(&consoleOpen);

	ShowFileWatcher(&core->filewatcher);


	WorldMap* map = &gameState->worldmap;

	static float colors[4];
	static Uint32 replacement = 0;
	replacement = ImGui::GetColorU32(ImVec4(colors[0], colors[1], colors[2], colors[3]));
	if (replacement != map->editor.editorColor) // jos vaihdettin editorista
	{
		auto color = ImGui::ColorConvertU32ToFloat4(map->editor.editorColor);
		colors[0] = color.x;
		colors[1] = color.y;
		colors[2] = color.z;
		colors[3] = color.w;
	}

	//if (ImGui::ColorEdit4("vari", colors, true))
	//{
	//	replacement = ImGui::GetColorU32(ImVec4(colors[0], colors[1], colors[2], colors[3]));
	//	map->editor.editorColor = replacement;
	//}

	//ImGui::Text("MousePos: %f, %f", input->mouse.x, input->mouse.y);

	//ImGui::Text("CurrentInputPos: %i, %i", (int)map->editor.inputX, (int)map->editor.inputY);

	//ImGui::InputText("Province ID: ", buffer, sizeof(buffer));
	//ImGui::SameLine();
	//if (ImGui::Button("save province ids"))
	//{
	//	SaveProvinceId(gameState, atoi(buffer));
	//}

	/*static bool pickingColor = false;
	ImGui::Checkbox("Toggle color pickings mode (press 6 to pick color)", &pickingColor);
	if (pickingColor)
		ImGui::Text("Picking");
	else
		ImGui::Text("Coloring");*/


		/*if (gameState->player)
		{
			auto vec4 = ImGui::ColorConvertU32ToFloat4(gameState->player->player.side);
			ImGui::Text("hello");
		}*/


	ImGui::Text("Spawn units");
	static int SpawnCount = 1;
	ImGui::InputInt("count: ", &SpawnCount);
	// ImGui::ColorEdit4("Side: ", )
	if (ImGui::Button("spawn stuff"))
	{
		for (int i = 0; i < SpawnCount; i++)
		{
			Entity* e = newEntity(input->mouse.x, input->mouse.y, Entity_unit, gameState);
			Uint32 unitColor = ImGui::GetColorU32(ImVec4(colors[0], colors[1], colors[2], colors[3]));
			e->unit.side = unitColor;
			e->unit.attackRange = 200.f;
			// e->unit.
		}
	}

	static bool selectTarget = false;
	ImGui::Checkbox("select target (g)", &selectTarget);
	if (selectTarget)
	{
		if (input->isKeyPressed(SDL_SCANCODE_G))
		{
			Entity* targetEntity = 0;
			Rect rect{ input->mouse.x, input->mouse.y, 60.f, 60.f };

			for (int i = 0; i < gameState->currentEntityCount; i++)
			{
				Entity* e = &gameState->entities[i];
				if (e->type == Entity_unit)
				{
					if (rect.Contains(e->x, e->y))
					{
						targetEntity = e;
						break;
					}
				}
			}

			if (targetEntity)
			{
				for (int i = 0; i < gameState->selectedCount; i++)
				{
					// nullaa jos kuollut target
					gameState->selectedEntitys[i]->unit.attackTarget = targetEntity;
				}
				printf("target set\n");
			}
		}
	}


	// Province editings
	//ImGui::NewLine();
	//static bool editNodes = false;
	//static std::vector<int> neighbours;

	//ImGui::Checkbox("Edit Nodes", &editNodes);

	//if (input->isMouseClicked() && editNodes)
	//{
	//	Uint32 side = gameState->worldmap.GetSideUnderMouse(&core->input->mouse);
	//	int id = GetColorToId(gameState, side);

	//	if (id != -1)
	//	{
	//		neighbours.push_back(id);
	//		GetConsoleInstance().AddLog("Added %i to neighbourlist", id);
	//	}
	//	else
	//	{
	//		GetConsoleInstance().AddLog("Can't add to neighbourslist");
	//	}
	//}

	//if (ImGui::Button("Clear neighbour list"))
	//{
	//	neighbours.clear();
	//	GetConsoleInstance().AddLog("Cleared list");
	//}

	//ImGui::Text("Neighbours ");
	//for (int i = 0; i < neighbours.size(); i++)
	//{
	//	ImGui::Text("%i", neighbours[i]); // Setti olis parempi ei samoja 2 kpl
	//}

	//if (ImGui::Button("save province data!"))
	//{
	//	gameState->dirtyFlag = true;
	//}


	// pathfinding part
	ImGui::Separator();
	ImGui::Text("Pathfinding: ");


	PathFindingUi* pathui = &gameState->pathfindingUi;
	ImGui::InputInt("StartID", &pathui->startId);
	ImGui::InputInt("EndID", &pathui->endId);
	ImGui::Checkbox("Show path", &pathui->drawPath);
	//	ImGui::SameLine();

		///////////////////////////////////////////////////////// SUPER EDITOR

	static bool superEditor = false;
	ImGui::Checkbox("super editor", &superEditor);
	if (superEditor)
	{
		ImGui::Begin("Super editor", &superEditor);

		if (ImGui::Button("Save changes "))
		{
			gameState->dirtyFlag = true;
			SaveProvinceId(gameState, map->editor.inputProvinceId);
		}
		else
		{
			if (input->isMouseClicked()) // mitä tapahtuu jos uusi provinssi
			{
				Uint32 colorUnderMouse = gameState->worldmap.GetSideUnderMouse(&input->mouse);
				map->editor.editorColor = colorUnderMouse;

				auto iter = gameState->provinceData.colorToId->find(colorUnderMouse); // 
				if (iter != gameState->provinceData.colorToId->end())
				{
					printf("%i", iter->second);
					map->editor.inputProvinceId = iter->second;
				}
				else // uusi provinssi
				{
					// gameState->provinceData.colorToId
					// gameState->provinceEditor.selectedProvinceId 
					ProvinceData* provData = &gameState->provinceData;

					int newProvinceId = *provData->currentCount;
					(*provData->currentCount) += 1;

					provData->colorToId->insert(std::make_pair(colorUnderMouse, newProvinceId));
					provData->idToColor[newProvinceId] = colorUnderMouse;
					provData->positions[newProvinceId] = { (int)input->mouse.x, (int)input->mouse.y };

					map->editor.inputProvinceId = newProvinceId;
					// debugBreak();
					gameState->newNode(map->editor.inputProvinceId, map->editor.inputProvinceId, (int)input->mouse.x, (int)input->mouse.y);
				}

				// set up neighbours
				std::vector<int>* neighbours = &gameState->provinceEditor.selectedNeighbours;
				neighbours->clear();

				std::vector<int> currentN = gameState->getAllProvinceNeighbours(map->editor.inputProvinceId);
				*neighbours = currentN;
			}

			if (input->isMouseClicked(2))
			{
				Uint32 colorUnderMouse = gameState->worldmap.GetSideUnderMouse(&input->mouse);
				map->editor.editorColor = colorUnderMouse;

				auto iter = gameState->provinceData.colorToId->find(colorUnderMouse);

				if (iter != gameState->provinceData.colorToId->end()) // onko id -> color
				{
					auto iter2 = std::find(gameState->provinceEditor.selectedNeighbours.begin(),
						gameState->provinceEditor.selectedNeighbours.end(), iter->second);

					if (iter2 == gameState->provinceEditor.selectedNeighbours.end())
					{
						gameState->provinceEditor.selectedNeighbours.push_back(iter->second);
						printf("added %i ", iter->second);
					}
					else
					{
						auto begin = gameState->provinceEditor.selectedNeighbours.begin();
						auto end = gameState->provinceEditor.selectedNeighbours.end();
						gameState->provinceEditor.selectedNeighbours.erase(std::remove(begin, end, iter->second));

						printf("removed %i ", iter->second);
					}
				}
			}

			// show data about current:
			ImGui::Text("Current id: %i", map->editor.inputProvinceId);
			std::vector<int>* neighbours = &gameState->provinceEditor.selectedNeighbours;
			if (neighbours->size() > 0)
			{

				ImGui::Text("Current Neighbours: ");
				for (int i = 0; i < neighbours->size(); i++)
				{
					ImGui::Text("%i, ", neighbours->at(i)); // Setti olis parempi ei samoja 2 kpl
				}

				// visualize current neighbours:
				VisualizeNodes(neighbours, map->editor.inputProvinceId, gameState);
			}
			if (ImGui::ColorEdit4("vari", colors, true))
			{
				replacement = ImGui::GetColorU32(ImVec4(colors[0], colors[1], colors[2], colors[3]));
				map->editor.editorColor = replacement;
			}

			ImGui::Text("Province Capital: %i, %i", (int)map->editor.inputX, (int)map->editor.inputY);
			ImGui::End();
		}
		// ImGui::InputText("Province id: ",  );
	}

	ImGui::Begin("fake player gui");
	if (ImGui::Button("build factory"))
	{
		gameState->player->player.selectedBuildingType = building_millitary_factory;
	}
	if (ImGui::Button("build mill"))
	{
		gameState->player->player.selectedBuildingType = building_mill;
	}
	ImGui::End();
}
//								______  ______  _____   ______
//							   /\__  _\/\  __`\/\  _`\ /\  __`\        
//							    \/_/\ \/\ \ \/\ \ \ \/\ \ \ \/\ \  __   
//                                 \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \/\_\  
//                                  \ \ \ \ \ \_\ \ \ \_\ \ \ \_\ \/_/_           korjaa jotain
//                                   \ \_\ \ \_____\ \____/\ \_____\ /\_\
//                                    \/_/  \/_____/\/___/  \/_____/ \/_/


// ---------------------------------------------------------------------
//if (ImGui::CollapsingHeader("Map Editor Settings"))
//{
//	ImGui::Text("hello");
//	if (state->mapEditorSettings.active)
//	{
//		if (ImGui::Button("stop Editing"))
//			state->mapEditorSettings.active = false;
//	}
//	else
//	{
//		if (ImGui::Button("start editing"))
//			state->mapEditorSettings.active = true;
//	}
//	ImGui::DragInt("Troop insert count", &state->mapEditorSettings.insertCount, 1, -99, 99);
//	ImGui::Text("Side");
//	ImGui::SameLine();
//	for (int i = 0; i < Unit_Side_Max; i++)
//	{
//		if (ImGui::Button(Unit_Side_Names[i]))
//		{
//			state->mapEditorSettings.insertedSide = (Unit_Side)i;
//		}
//		ImGui::SameLine();
//	}

//	ImGui::Checkbox("Region Mode", &state->mapEditorSettings.regionMode);
//	ImGui::DragInt("Region ID", &state->mapEditorSettings.regionID, 1, 0, 16); // TODO: taikai ?????
//}
