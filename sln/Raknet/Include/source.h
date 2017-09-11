#pragma once
#ifndef EXPORT
#define EXPORT extern "C" __declspec(dllexport)
#endif

enum Mode
{
	mode_server,
	mode_client,
};

EXPORT void init(Mode mode);
EXPORT void update();
EXPORT void cleanUp();
