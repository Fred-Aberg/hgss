#pragma once
#include "c_codebase/src/common.h"
#include "c_codebase/src/raytiles.h"
#include "religion.h"
#include "culture.h"
#include "admin.h"
#include "icons.h"
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
	BOURGEOISIE
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
	char baked_food_yield;
	char baked_production_yield;
	uint8_t structure_id;		// -> bonus yields | -> max 255 buildings
	ownership_e ownership_e;
	environment_e env;			// -> base yields
	resource_e resource;		// -> bonus yields
	uint32_t pop_data_id;
	uint16_t region_id;			// -> Max 65,535 regions
}cell_t;

typedef enum 
{
	NO_STATUS,

}status_type_e;

typedef struct
{
	status_type_e type;
	uint8_t duration;			// 255 -> permanent
}status_t;

#define MAX_STATUSES 16
#define MAX_REGION_CELL_CAP 255
typedef struct
{
	uint8_t cell_count;  	// -> max 255 cells for each region
	uint8_t cell_capacity;	
	int16_t baked_food_yield;
	int16_t baked_production_yield;
	int16_t baked_gold_yield;
	uint32_t *cell_ids;
	status_t statuses[MAX_STATUSES];
	administration_t *admin;
}region_t;

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
	cell_t *cells;
	pop_data_t *pop_data;
	uint16_t region_count;
	uint16_t region_capacity;
	region_t *regions; // sort by the address of the first cell? middle cell?
	mapgen_data_t mapg_data;
	structure_t structures[256];
}map_t;

void map_env_to_str(char *buf, environment_e env);

pos16_t map_cell_id_to_xy(map_t *map, uint32_t i);

void map_print_region(map_t *map, uint16_t region_id);

map_t *map_create_map(uint16_t width, uint16_t height);

cell_t *map_get_cell(map_t *map, uint16_t x, uint16_t y);

cell_graphics_t map_calc_cell_graphics(cell_t cell, int rand);

region_t *map_add_region(map_t *map, uint32_t *cell_ids, uint8_t cell_count);

void map_sync_region_ids(map_t *map, uint16_t start_index);

void map_remove_region(map_t *map, uint16_t region_id, bool sync_region_ids);
// 
void map_rebake_region_yields(region_t *region_array, cell_t *cell_array, uint16_t region_id);
// 
// void map_rebake_regions(map_t *map, uint16_t *region_id );
// 

void map_rebake_cell_yields(cell_t *cell_array, uint32_t cell_id);
// 
void map_rebake_cell_yields_propagate(region_t *region_array, cell_t *cell_array, uint32_t cell_id);
// 
void map_rebake_yields(map_t *map);

void map_add_cell_to_region(map_t *map, uint16_t region_id, uint32_t cell_id);
// 
void map_add_cells_to_region(map_t *map, uint16_t region_id, uint32_t *cell_ids, uint8_t cell_count);

bool map_merge_region_a_with_b(map_t *map, uint16_t region_id_a, uint16_t region_id_b, bool sync_region_ids);

// void map_remove_cell_from_region(uint16_t region_id, uint32_t cell_id);
// 
// void map_remove_cells_from_region(uint16_t region_id, uint32_t *cell_ids);

uint16_t map_get_region_pop(map_t *map, uint16_t region_id);

void map_draw_map_onto_grid(grid_t *grid, map_t *map, pos16_t cam_map_pos, char map_font, map_visualisation_e vis, cell_t *selected_cell);

pos16_t map_map_pos16_to_grid_pos(map_t *map, pos16_t grid_pos, pos16_t camera_position);

pos16_t map_grid_pos_to_map_pos(grid_t *grid, map_t *map, pos16_t grid_pos, pos16_t camera_position);


///  BAKED MAP  ///

typedef struct
{
	uint8_t **flag_bytes;
	map_t *map;
	uint16_t b_w;
	uint16_t b_h;
}bmap_t;

bmap_t *bmap_create(map_t *map);

void bmap_bake(bmap_t *bmap);

void bmap_rebake(uint32_t i, bmap_t *bmap);

void bmap_rebake_at(uint16_t x, uint16_t y, bmap_t *bmap);

void bmap_get_instruction(bmap_t *bmap, uint32_t *i, color8b_t *interval_col, uint16_t *interval_len);

uint16_t bmap_get_interval_at(bmap_t *bmap, uint16_t *x, uint16_t y);


/// MAP GEN ///

map_t *map_generate(uint16_t width, uint16_t height, uint8_t sea_lvl);
