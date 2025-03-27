#pragma once
#include "c_codebase/src/common.h"
#include "c_codebase/src/raytiles.h"
#include "religion.h"
#include "culture.h"
#include "admin.h"
#include "raylib.h"

typedef signed long long llong_t;

typedef enum 
{
	NONE,				// -1
	WATER,				// 0
	
	RIVER,				// 1
	BEACH,				// 1
	MARSH,				// 1
	FLOODPLAINS,		// 1
	URBAN,				// 1
	RUINS,				// 1
	
	GRASSLAND,			// 2
	WOODLANDS,			// 2
	RIVERLANDS,			// 2
	URBAN_LAND,			// 2
	RUINED_LANDS,		// 2
	
	HILLS,				// 3
	WOODLAND_HILLS,		// 3
	RIVER_HILLS,		// 3
	URBAN_HILLS,		// 3
	RUINED_HILLS,		// 3
	
	HIGHLANDS,			// 4
	HIGHLANDS_WOODS,	// 4
	RIVER_HIGHLANDS,	// 4
	URBAN_HIGHLANDS,	// 4
	RUINED_HIGHLANDS,	// 4
	
	MOUNTAINS			// 5
}environment_e;

typedef enum 
{
	NO_RESOURCE,
	// GOLD,
	COAL,
	OIL,
	METALS
}resource_e;

typedef enum 
{
	STATE,
	BOURGEOISIE,
	CLERGY
}ownership_e;

typedef enum 
{
	DEFAULT,
	POPULATION,
	HEIGHTMAP,
	REGIONS,
	TERRAIN
}map_visualisation_e;

typedef struct
{
	resource_e resource;
	char icon;
	uint8_t gold_price;
	uint8_t production_price;
	int8_t food_yield;
	int8_t production_yield;
	int8_t resource_yield;
	char *name;
	char *desc;
}structure_t;

typedef struct
{
	uint8_t influence;
	uint8_t culture_id;
}culture_influence_t;

typedef struct
{
	uint8_t influence;
	uint8_t religion_id;
}religion_influence_t;

#define MAX_CLTS_IN_CELL 16
#define MAX_RLGNS_IN_CELL 16
typedef struct
{
	culture_influence_t clt_influences[MAX_CLTS_IN_CELL];
	religion_influence_t rlgn_influences[MAX_RLGNS_IN_CELL];
}pop_data_t;


typedef struct
{
	uint8_t pop_lvl;
	int8_t infrastructure_lvl;
	int8_t baked_food_yield;
	int8_t baked_production_yield;
	uint8_t structure_id;		// -> bonus yields | -> max 255 buildings
	ownership_e ownership_e;
	environment_e env;			// -> base yields
	resource_e resource;		// -> bonus yields
	uint16_t realm_id;
	uint32_t pop_data_id;
}cell_t;

typedef struct
{
	color8b_t bg_col;
	color8b_t char_col;
	char icon;
}cell_graphics_t;

typedef struct
{
	uint8_t sea_level;
	int32_t seed;
	Image heightmap;
	Image climate_map;
}mapgen_data_t;

typedef struct
{
	uint16_t width;
	uint16_t height;
	cell_t *cells;						// w * h
	pop_data_t *pop_data;				// w * h
	cell_graphics_t *b_cell_graphics;	// w * h
	mapgen_data_t mapg_data;
	structure_t structures[256];
}map_t;

/// General map_t functions

map_t *map_create_map(uint16_t width, uint16_t height);

cell_t *map_get_cell_p(map_t *map, pos16_t p);
cell_t *map_get_cell_i(map_t *map, uint32_t i);

cell_t *map_get_pop_data_p(map_t *map, pos16_t p);
cell_t *map_get_pop_data_i(map_t *map, uint32_t i);

cell_t *map_get_cell_gfx_p(map_t *map, pos16_t p);
cell_t *map_get_cell_gfx_i(map_t *map, uint32_t i);

uint32_t map_p_to_i(map_t *map, pos16_t p);
pos16_t map_i_to_p(map_t *map, uint32_t i);

/// Per cell_t functions

cell_graphics_t map_calc_cell_graphics(cell_t cell, int rand);

void map_rebake_cell_yields(cell_t *cell_array, uint32_t cell_id);

/// Operate on all cells

void map_rebake_yields(map_t *map);

void map_rebake_graphics(map_t *map);

void map_rebake_cells(map_t *map); // graphics and yields

/// Rendering

void map_draw_map_onto_grid(grid_t *grid, map_t *map, pos16_t cam_map_pos, char map_font, map_visualisation_e vis, cell_t *selected_cell);

pos16_t map_map_pos_to_grid_pos(map_t *map, pos16_t grid_pos, pos16_t camera_position);

pos16_t map_grid_pos_to_map_pos(grid_t *grid, map_t *map, pos16_t grid_pos, pos16_t camera_position);

void map_env_to_str(char *buf, environment_e env);
