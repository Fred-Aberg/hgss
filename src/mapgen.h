#pragma once
#include "map.h"
#include "c_codebase/src/common.h"



void mapgen_plot_river(map_t *map, uint16_t x, uint16_t y);

void mapgen_place_rivers(map_t *map);

void mapgen_update_map_environments_from_heightmap(map_t *map);

void mapgen_populate_map(map_t *map, float amp, float randomness, float r_scale);

map_t *mapgen_gen_from_heightmap(Image heightmap, uint8_t sea_level, long seed);

void mapgen_assign_heightmap(map_t *map, int ridge_sharpness, int island_sharpness);

void mapgen_plot_region(map_t *map, uint16_t rt_x, uint16_t rt_y, uint16_t region_id, uint8_t max_cells);

void mapgen_place_regions(map_t *map, uint8_t avg_cells, uint8_t cell_count_spread, uint8_t minimum_cells_in_rgn);

void mapgen_clear_region_ids(map_t *map);
