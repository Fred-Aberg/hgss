#pragma once
#include "map.h"
#include "c_codebase/src/common.h"



void mapgen_plot_river(map_t *map, ushort_t x, ushort_t y);

void mapgen_place_rivers(map_t *map);

void mapgen_update_map_environments_from_heightmap(map_t *map);

void mapgen_populate_map(map_t *map, float amp, float randomness, float r_scale);

map_t *mapgen_gen_from_heightmap(Image heightmap, uchar_t sea_level, long seed);

void mapgen_assign_heightmap(map_t *map, int ridge_sharpness, int island_sharpness);

void mapgen_plot_region(map_t *map, ushort_t rt_x, ushort_t rt_y, ushort_t region_id, uchar_t max_cells);

void mapgen_place_regions(map_t *map, uchar_t avg_cells, uchar_t cell_count_spread, uchar_t minimum_cellsin_rgn);

void mapgen_clear_region_ids(map_t *map);
