member_definition memberOf_game_state[] = 
{
	{ MetaType_memory_arena, "arena", (u32)&((game_state *)0)->arena }, 
	{ MetaType_Entity, "entities", (u32)&((game_state *)0)->entities }, 
	{ MetaType_Entity, "player", (u32)&((game_state *)0)->player }, 
	{ MetaType_Entity, "selectedEntitys", (u32)&((game_state *)0)->selectedEntitys }, 
	{ MetaType_int, "selectedCount", (u32)&((game_state *)0)->selectedCount }, 
	{ MetaType_int, "maxSelected", (u32)&((game_state *)0)->maxSelected }, 
	{ MetaType_int, "currentEntityCount", (u32)&((game_state *)0)->currentEntityCount }, 
	{ MetaType_ProvinceData, "provinceData", (u32)&((game_state *)0)->provinceData }, 
	{ MetaType_WorldMap, "worldmap", (u32)&((game_state *)0)->worldmap }, 
	{ MetaType_float, "cameraSpeed", (u32)&((game_state *)0)->cameraSpeed }, 
};
