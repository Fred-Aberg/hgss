#pragma once
#include "map.h"
#include "c_codebase/src/common.h"



void mapgen_plot_river(map_t *map, uint16_t x, uint16_t y);

void mapgen_place_rivers(map_t *map);

void mapgen_update_map_environments_from_heightmap(map_t *map);

void mapgen_populate_map(map_t *map, float amp, float randomness, float r_scale);

map_t *mapgen_gen_from_heightmap(Image heightmap, uint8_t sea_level, long seed);

void mapgen_assign_heightmap(map_t *map, int ridge_sharpness, int island_sharpness);
