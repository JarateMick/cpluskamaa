member_definition membersOf_MapNode[] = 
{
	{0, MetaType_int, "id", (u32)&((MapNode *)0)->id }, 
	{0, MetaType_float, "x", (u32)&((MapNode *)0)->x }, 
	{0, MetaType_float, "y", (u32)&((MapNode *)0)->y }, 
};
member_definition membersOf_game_state[] = 
{
	{0, MetaType_memory_arena, "arena", (u32)&((game_state *)0)->arena }, 
	{0, MetaType_Entity, "entities", (u32)&((game_state *)0)->entities }, 
	{MetaMemberFlag_IsPointer, MetaType_Entity, "player", (u32)&((game_state *)0)->player }, 
	{MetaMemberFlag_IsPointer, MetaType_Entity, "selectedEntitys", (u32)&((game_state *)0)->selectedEntitys }, 
	{0, MetaType_int, "selectedCount", (u32)&((game_state *)0)->selectedCount }, 
	{0, MetaType_int, "maxSelected", (u32)&((game_state *)0)->maxSelected }, 
	{0, MetaType_int, "currentEntityCount", (u32)&((game_state *)0)->currentEntityCount }, 
	{0, MetaType_ProvinceData, "provinceData", (u32)&((game_state *)0)->provinceData }, 
	{0, MetaType_WorldMap, "worldmap", (u32)&((game_state *)0)->worldmap }, 
	{0, MetaType_float, "cameraSpeed", (u32)&((game_state *)0)->cameraSpeed }, 
	{0, MetaType_bool, "dirtyFlag", (u32)&((game_state *)0)->dirtyFlag }, 
	{0, MetaType_ProvinceEditorData, "provinceEditor", (u32)&((game_state *)0)->provinceEditor }, 
	{0, MetaType_PathFindingUi, "pathfindingUi", (u32)&((game_state *)0)->pathfindingUi }, 
};
#define META_HANDLE_TYPE_DUMD(MemberPtra, nextIntendLevel) \
	case MetaType_game_state: { dumpStruct(ArrayCount(membersOf_game_state),membersOf_game_state, memberPtr, nextIntendLevel + 1); } break; \
	case MetaType_MapNode: { dumpStruct(ArrayCount(membersOf_MapNode),membersOf_MapNode, memberPtr, nextIntendLevel + 1); } break; 
