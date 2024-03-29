#pragma once

//class Debug
//{
//public:
//	static bool restartLog();
//	static bool log(const char* message, ...);
//	static bool logError(const char* message, ...);
//};

#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <DebugRenderer.h>
#define LOG_FILE "error.log"



// TODO: static class niin ei p��se k�siksi noihin kahteen
namespace Debug
{
	static UpiEngine::DebugRenderer* _Debugger = 0;

	void AddToConsole(const char* message)
	{
	}

	bool restartLog()
	{
		FILE* file = fopen(LOG_FILE, "w");
		if (!file)
		{
			fprintf(stderr,
				"ERROR: could not open LOG_FILE log file %s for writing\n",
				LOG_FILE);
			return false;
		}
		time_t now = time(NULL);
		char* date = ctime(&now);
		fprintf(file, "LOG_FILE log. local time %s\n", date);
		fclose(file);
		return true;
	}

	bool log(const char* message, ...)
	{
		va_list argptr;
		FILE* file = fopen(LOG_FILE, "a");
		if (!file)
		{
			fprintf(
				stderr,
				"ERROR: could not open LOG_FILE %s file for appending\n",
				LOG_FILE
			);
			return false;
		}
		va_start(argptr, message);
		vfprintf(file, message, argptr);
		va_end(argptr);
		fclose(file);
		return true;
	}

	bool logError(const char* message, ...)
	{
		va_list argptr;
		FILE* file = fopen(LOG_FILE, "a");
		if (!file)
		{
			fprintf(stderr,
				"ERROR: could not open LOG_FILE %s file for appending\n",
				LOG_FILE);
			return false;
		}
		va_start(argptr, message);
		vfprintf(file, message, argptr);
		va_end(argptr);
		va_start(argptr, message);
		vfprintf(stderr, message, argptr);
		va_end(argptr);
		fclose(file);
		return true;
	}

	void drawTextf(int slot, char* format, ...)
	{
		char buffer[128];

		va_list args;
		va_start(args, format);
		vsnprintf(buffer, 128, format, args);
		va_end(args);
	
		_Debugger->drawText(buffer, slot);
	}

	void drawText(char* buffer, int slot)
	{
		_Debugger->drawText(buffer, slot);
	}

	void drawCircle(const glm::vec2& center, const UpiEngine::ColorRGBA8& color, float radius)
	{
		_Debugger->drawCircle(center, color, radius);
	}

	void drawBox(const glm::vec4& destRect, const UpiEngine::ColorRGBA8& color, float angle)
	{
		_Debugger->drawBox(destRect, color, angle);
	}

	void drawBox(const glm::vec4& destRect)
	{
		static UpiEngine::ColorRGBA8 white(255, 255, 255, 255); // TODO: default color
		_Debugger->drawBox(destRect, white, 0);
	}

	void drawBox(float x, float y, float w, float h)
	{
		static UpiEngine::ColorRGBA8 color(255, 255, 255, 255);
		_Debugger->drawBox({x, y, w, h}, color, 0);
	}

	void drawLine(glm::vec2& start, glm::vec2 end, const UpiEngine::ColorRGBA8& color)
	{
		_Debugger->drawLine(start, end, color);
	}

	void drawLine(float x1, float y1, float x2, float y2, const UpiEngine::ColorRGBA8& color)
	{
		glm::vec2 start2{ x1, y1 };
		glm::vec2 end2{ x2, y2 };
		drawLine(start2, end2, color);
	}

	void drawLine(v2_d start, v2_d end, const UpiEngine::ColorRGBA8& color)
	{
		glm::vec2 start2{ start.x, start.y };
		glm::vec2 end2{ end.x, end.y };
		drawLine(start2, end2, color);
	}

	void drawLine(v2_d start, v2_d end)
	{
		static UpiEngine::ColorRGBA8 white(255, 255, 255, 255); // TODO: default color
		glm::vec2 start2{ start.x, start.y };
		glm::vec2 end2{ end.x, end.y };
		drawLine(start2, end2, white);
	}
	
};

// UpiEngine::DebugRenderer* Debug::_Debugger = 0;
// static Console* console 
