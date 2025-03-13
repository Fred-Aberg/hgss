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
	HEIGHTMAP
}map_visualisation_e;

typedef struct
{
	resource_e resource;
	char icon;
	uchar_t gold_price;
	uchar_t production_price;
	char food_yield;
	char production_yield;
	char resource_yield;
	char *name;
	char *desc;
}structure_t;

typedef struct
{
	uchar_t influence;
	uchar_t culture_id;
}culture_influence_t;

typedef struct
{
	uchar_t influence;
	uchar_t religion_id;
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
	uchar_t pop_lvl;
	char baked_food_yield;
	char baked_production_yield;
	uchar_t structure_id;		// -> bonus yields | -> max 255 buildings
	ownership_e ownership_e;
	environment_e env;			// -> base yields
	resource_e resource;		// -> bonus yields
	ulong_t pop_data_id;
	ushort_t region_id;			// -> Max 65,535 regions
}cell_t;

typedef enum 
{
	NO_STATUS,

}status_type_e;

typedef struct
{
	status_type_e type;
	uchar_t duration;			// 255 -> permanent
}status_t;

#define MAX_STATUSES 16
typedef struct
{
	uchar_t cell_count;  	// -> max 255 cells for each region
	uchar_t cell_capacity;	
	short baked_food_yield;
	short baked_production_yield;
	short baked_gold_yield;
	ulong_t *cell_ids;
	status_t statuses[MAX_STATUSES];
	administration_t *admin;
}region_t;

typedef struct
{
	uint_t width;
	uint_t height;
	cell_t *cells;
	pop_data_t *pop_data;
	ushort_t region_count;
	ushort_t region_capacity;
	region_t *regions; // sort by the address of the first cell? middle cell?
	Image heightmap;
	structure_t structures[255];
}map_t;

void map_env_to_str(char *buf, environment_e env);

map_t *map_create_map(uint_t width, uint_t height);

cell_t *map_get_cell(map_t *map, ushort_t x, ushort_t y);

cell_graphics_t map_calc_cell_graphics(cell_t cell);

// region_t *map_add_region(map_t *map, ulong_t *cell_ids);
// 
// void map_remove_region(map_t *map, ushort_t region_id);
// 
// void map_rebake_region_yields(map_t *map, ushort_t region_id);
// 
// void map_rebake_regions(map_t *map, ushort_t *region_id );
// 
// void map_rebake_cell_yields(map_t *map, ulong_t cell_id);
// 
// void map_rebake_cell_yields_propagate(map_t *map);
// 
// void map_add_cell_to_region(ushort_t region_id, ulong_t cell_id);
// 
// void map_add_cells_to_region(ushort_t region_id, ulong_t *cell_ids);
// 
// void map_remove_cell_from_region(ushort_t region_id, ulong_t cell_id);
// 
// void map_remove_cells_from_region(ushort_t region_id, ulong_t *cell_ids);

void map_draw_map_onto_grid(grid_t *grid, map_t *map, Pos_t camera_position, char map_font, map_visualisation_e vis);

Pos_t map_map_pos_to_grid_pos(map_t *map, Pos_t grid_pos, Pos_t camera_position);

Pos_t map_grid_pos_to_map_pos(grid_t *grid, map_t *map, Pos_t grid_pos, Pos_t camera_position);


///  BAKED MAP  ///

typedef struct
{
	uchar_t **flag_bytes;
	map_t *map;
	ushort_t b_w;
	ushort_t b_h;
}bmap_t;

bmap_t *bmap_create(map_t *map);

void bmap_bake(bmap_t *bmap);

void bmap_rebake(ulong_t i, bmap_t *bmap);

void bmap_rebake_at(ushort_t x, ushort_t y, bmap_t *bmap);

void bmap_get_instruction(bmap_t *bmap, ulong_t *i, color8b_t *interval_col, ushort_t *interval_len);

ushort_t bmap_get_interval_at(bmap_t *bmap, ushort_t *x, ushort_t y);


/// MAP GEN ///

map_t *map_generate(uint_t width, uint_t height, uchar_t sea_lvl);
