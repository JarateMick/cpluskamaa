#pragma once

#include <SDL2\SDL_stdinc.h>

enum meta_type
{
	MetaType_memory_arena,
	MetaType_ProvinceData,
	// MetaType_
	MetaType_Entity,
	MetaType_int,
	MetaType_WorldMap,
	MetaType_float,
	MetaType_world_chunk,
	MetaType_uint32,
	MetaType_bool32,
	MetaType_entity_type,
	MetaType_v3,
	MetaType_real32,
	MetaType_sim_entity_collison_volume_group,
	MetaType_hit_points,
	MetaType_entity_reference,
	MetaType_v2,
	MetaTyp_real32,
};

struct member_definition
{
	meta_type type;
	char*     name;
	Uint32    offset;
};
