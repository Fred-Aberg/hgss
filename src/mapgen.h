#pragma once
#include "map.h"
#include "c_codebase/src/common.h"


map_t *mapgen_gen_continent(ushort_t w, ushort_t h, uchar_t sea_level, long seed);

void mapgen_plot_river(map_t *map, Image heightmap, ushort_t x, ushort_t y);

map_t *mapgen_gen_from_heightmap(char *file_path, uchar_t sea_level, long seed);
