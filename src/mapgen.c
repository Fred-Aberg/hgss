#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mapgen.h"
#include "raylib.h"
#include "c_codebase/src/common.h"

#define M_LVL_P 0.50f
#define HL_LVL_P 0.60f
#define HLS_LVL_P 0.80f
#define FLT_LVL_P 0.98f
#define MIN_CELLS_IN_REGION 10

// Divide seed into two 16 bit numbers
#define seed_x ((int *)(&map->mapg_data.seed))[0] 
#define seed_y ((int *)(&map->mapg_data.seed))[1] 

// Divide seed into four 8 bit numbers
#define seed_b1 ((char *)(&map->mapg_data.seed))[0]
#define seed_b2 ((char *)(&map->mapg_data.seed))[1]
#define seed_b3 ((char *)(&map->mapg_data.seed))[2]
#define seed_b4 ((char *)(&map->mapg_data.seed))[3]

#define val(v) c(v,v,v)
#define p_val(image, x, y) GetImageColor(image, x, y).r

bool watery(cell_t *c)
{
	return c->env == WATER || c->env == RIVER;
}

Pos_t find_nearest_water_source(map_t *map, ushort_t x, ushort_t y)
{
	ushort_t x0;
	ushort_t x1;
	ushort_t y0;
	ushort_t y1;
	for (ushort_t srch_dst = 1; srch_dst < 100; srch_dst++)
	{
		for (ushort_t _y = max((int)y - srch_dst / 2, 0); _y < min((int)y + srch_dst / 2, map->height); _y++)
		{
			x0 = max((int)x - srch_dst / 2, 0);
			x1 = min((int)x + srch_dst / 2, map->width);
			if (map_get_cell(map, x0, _y)->env == WATER || map_get_cell(map, x0, _y)->env == RIVER)
				return pos(x0, _y);
			else if (map_get_cell(map, x1, _y)->env == WATER || map_get_cell(map, x1, _y)->env == RIVER)
				return pos(x1, _y);
		}

		for (ushort_t _x = max((int)x - srch_dst / 2, 0); _x < min((int)x + srch_dst / 2, map->width); _x++)
		{
			y0 = max((int)y - srch_dst / 2, 0);
			y1 = min((int)y + srch_dst / 2, map->height);
			if (map_get_cell(map, _x, y0)->env == WATER || map_get_cell(map, _x, y0)->env == RIVER)
				return pos(_x, y0);
			else if (map_get_cell(map, _x, y1)->env == WATER || map_get_cell(map, _x, y1)->env == RIVER)
				return pos(_x, y1);
		}
	}

	return pos(0,0); // No rivers for (0,0) lmao
}

ushort_t find_nearest_region(map_t *map, ushort_t x, ushort_t y)
{
	ushort_t c_region_id = map_get_cell(map, x, y)->region_id;
	ushort_t x0;
	ushort_t x1;
	ushort_t y0;
	ushort_t y1;
	for (ushort_t srch_dst = 1; srch_dst < 100; srch_dst++)
	{
		for (ushort_t _y = max((int)y - srch_dst / 2, 0); _y < min((int)y + srch_dst / 2, map->height); _y++)
		{
			x0 = max((int)x - srch_dst / 2, 0);
			x1 = min((int)x + srch_dst / 2, map->width);

			ushort_t N_cell_r_id = map_get_cell(map, x0, _y)->region_id;
			ushort_t S_cell_r_id = map_get_cell(map, x1, _y)->region_id;
			
			if (N_cell_r_id != 0 && N_cell_r_id != c_region_id)
				return N_cell_r_id;
			else if (S_cell_r_id != 0 && S_cell_r_id != c_region_id)
				return S_cell_r_id;
		}

		for (ushort_t _x = max((int)x - srch_dst / 2, 0); _x < min((int)x + srch_dst / 2, map->width); _x++)
		{
			y0 = max((int)y - srch_dst / 2, 0);
			y1 = min((int)y + srch_dst / 2, map->height);

			ushort_t W_cell_r_id = map_get_cell(map, _x, y0)->region_id;
			ushort_t E_cell_r_id = map_get_cell(map, _x, y1)->region_id;
			
			if (W_cell_r_id != 0 && W_cell_r_id != c_region_id)
				return W_cell_r_id;
			else if (E_cell_r_id != 0 && E_cell_r_id != c_region_id)
				return E_cell_r_id;
		}
	}

	return 0; // No region found
}

Image *qsort_heightmap;

// elev. comparison function
int compare_elev(const void* a, const void* b) 
{
   return (int)p_val(*qsort_heightmap, (*(Pos_t *)a).x, (*(Pos_t *)a).y) - (int)p_val(*qsort_heightmap, (*(Pos_t *)b).x, (*(Pos_t *)b).y);
}

ushort_t dst(ushort_t origin_x, ushort_t origin_y, ushort_t x, ushort_t y)
{
	return sqrt(pow(x-origin_x, 2) + pow(y-origin_y, 2));
}

ushort_t n_rivers = 0;
bool plot_river_3(map_t *map, ushort_t origin_x, ushort_t origin_y, ushort_t x, ushort_t y, ushort_t parent_x, ushort_t parent_y, 
					ushort_t length, ushort_t id, ushort_t max_len)
{	
	if(x < 3 || y < 3 || x > map->width-3 || y > map->height-3)
		return false;

	ushort_t distance = dst(origin_x, origin_y, x, y);
	if(length > distance * 3) // Loop / meandering detection
	{
		puts("\nKilled by distance");
		return false;
	}

	int neighbours = (map_get_cell(map, x + 1, y)->region_id == id) + (map_get_cell(map, x - 1, y)->region_id == id) +
					 (map_get_cell(map, x, y + 1)->region_id == id) + (map_get_cell(map, x, y - 1)->region_id == id);

	if (neighbours > 1)
		return false;

	int forgiveness = 3;
	
	cell_t *c_cell = map_get_cell(map, x, y);
	c_cell->region_id = id;

	if(c_cell->env == WATER) // Ocean reached
		return true;

	if(c_cell->env == RIVER) // Join or create lake ?
		return true;
	
	Pos_t river_cnds[4] = {pos(x, min((int)y + 1, map->height - 1)), pos(x, max((int)y - 1, 0)), pos(min((int)x + 1, map->width - 1), y), pos(max((int)x - 1, 0), y)};
	
	uchar_t c_elevation = p_val(map->mapg_data.heightmap, x, y);
	
	qsort_heightmap = &map->mapg_data.heightmap;
	qsort(river_cnds, 4, sizeof(Pos_t), compare_elev);

	uchar_t cnd_elevation;

	printf("\n[%u] L: %u Dst: %u (%u, %u) -> %u", id, length, distance, x, y, c_elevation);

	for (int i = 0; i < 4; i++)
	{
		cnd_elevation = p_val(map->mapg_data.heightmap, river_cnds[i].x, river_cnds[i].y);
		
		if(river_cnds[i].x == parent_x && river_cnds[i].y == parent_y)
			continue;
		if(map_get_cell(map, river_cnds[i].x, river_cnds[i].y)->region_id == id)
			continue;

		if(cnd_elevation <= c_elevation + forgiveness)
		{
			if(plot_river_3(map, origin_x, origin_y, river_cnds[i].x, river_cnds[i].y, x, y, length + 1, id, max_len))
			{
				c_cell->env = RIVER;
				c_cell->pop_lvl = 0;
				return true;
			}
			else
			{
				map_get_cell(map, river_cnds[i].x, river_cnds[i].y)->region_id = id;
			}
		}
	}
	
	// map_get_cell(map, x, y)->env = WATER;
	// map_get_cell(map, x + 1, y)->env = WATER;
	// map_get_cell(map, x - 1, y)->env = WATER;
	// map_get_cell(map, x, y + 1)->env = WATER;
	// return true;
	if(length > max_len) // too long mate
	{
		map_get_cell(map, x, y)->env = WATER;
		map_get_cell(map, x + 1, y)->env = WATER;
		map_get_cell(map, x - 1, y)->env = WATER;
		map_get_cell(map, x, y + 1)->env = WATER;
		return true;
	}
	return false;
}

#define PATH_ARRAY_LEN 512
bool in_path(Pos_t *parent_path, ushort_t x, ushort_t y) // check adjacency?
{
	for (ushort_t i = 0; i < PATH_ARRAY_LEN; i++)
		if(parent_path[i].x == x && parent_path[i].y == y)
			return true;
	return false;
}

bool plot_river_2(map_t *map, ushort_t origin_x, ushort_t origin_y, ushort_t x, ushort_t y, ushort_t parent_x, ushort_t parent_y, 
					ushort_t length, Pos_t *parent_path, ushort_t path_count)
{		
	if(in_path(parent_path, x, y))
		return false;

	ushort_t distance = dst(origin_x, origin_y, x, y);
	if(length > distance * 4) // Loop / meandering detection
	{
		puts("\nKilled by distance");
		return false; 
	}
	
	if(length > PATH_ARRAY_LEN) // too long mate
	{
		map_get_cell(map, x, y)->env = WATER;
		map_get_cell(map, x + 1, y)->env = WATER;
		map_get_cell(map, x - 1, y)->env = WATER;
		map_get_cell(map, x, y + 1)->env = WATER;
		return true;
	}

	int forgiveness = 2;
	
	cell_t *c_cell = map_get_cell(map, x, y);

	if(c_cell->env == WATER) // Ocean reached
		return true;

	if(c_cell->env == RIVER) // Join or create lake ?
		return true;
	
	Pos_t c_path[PATH_ARRAY_LEN];
	memcpy(&c_path[0], &parent_path[0], PATH_ARRAY_LEN * sizeof(Pos_t));
	c_path[path_count % PATH_ARRAY_LEN] = pos(x, y);
	path_count++;

	if(parent_x != x)
	{
		c_path[path_count % PATH_ARRAY_LEN] = pos(x + ((int)parent_x - (int)x), y + 1);
		path_count++;	
		c_path[path_count % PATH_ARRAY_LEN] = pos(x + ((int)parent_x - (int)x), y - 1);
		path_count++;	
	}
	else
	{
		c_path[path_count % PATH_ARRAY_LEN] = pos(x + 1, y + ((int)parent_y - (int)y));
		path_count++;	
		c_path[path_count % PATH_ARRAY_LEN] = pos(x - 1, y + ((int)parent_y - (int)y));
		path_count++;
	}
	
	
	Pos_t river_cnds[4] = {pos(x, min((int)y + 1, map->height - 1)), pos(x, max((int)y - 1, 0)), pos(min((int)x + 1, map->width - 1), y), pos(max((int)x - 1, 0), y)};
	
	uchar_t c_elevation = p_val(map->mapg_data.heightmap, x, y);
	
	qsort_heightmap = &map->mapg_data.heightmap;
	qsort(river_cnds, 4, sizeof(Pos_t), compare_elev);

	uchar_t cnd_elevation;

	printf("\n\tP[%u/%u]\nL: %u Dst: %u (%u, %u) -> %u", path_count % PATH_ARRAY_LEN, PATH_ARRAY_LEN, length, distance, x, y, c_elevation);

	for (int i = 0; i < 4; i++)
	{
		cnd_elevation = p_val(map->mapg_data.heightmap, river_cnds[i].x, river_cnds[i].y);
		
		if(river_cnds[i].x == parent_x && river_cnds[i].y == parent_y)
			continue;

		if(cnd_elevation <= c_elevation + forgiveness)
		{
			if(plot_river_2(map, origin_x, origin_y, river_cnds[i].x, river_cnds[i].y, x, y, length + 1, &c_path[0], path_count))
			{
				c_cell->env = RIVER;
				c_cell->pop_lvl = 0;
				return true;
			}
			else
			{
				c_path[path_count % PATH_ARRAY_LEN] = pos(river_cnds[i].x, river_cnds[i].y);
				path_count++;
			}
		}
	}
	return false;
}

void mapgen_plot_river(map_t *map, ushort_t x, ushort_t y)
{
	// Pos_t path[PATH_ARRAY_LEN];
	// memset(&path[0], 0, PATH_ARRAY_LEN * sizeof(Pos_t)); // Null path array
	plot_river_3(map,x, y, x, y, x, y, 0, n_rivers++, 50);
}

void mapgen_place_rivers(map_t *map)
{
	for (uint_t y = 0; y < map->height; y++)
		for (uint_t x = 0; x < map->width; x++)
		{
			int rand = GetRandomValue(0, 10000);
			if (map_get_cell(map, x, y)->env == MOUNTAINS && rand > 9999)
				mapgen_plot_river(map, x, y);
			else if (map_get_cell(map, x, y)->env == HIGHLANDS && rand > 9998)
				mapgen_plot_river(map, x, y);
			else if ((map_get_cell(map, x, y)->env == HILLS || map_get_cell(map, x, y)->env == WOODLAND_HILLS) && rand > 9997)
				mapgen_plot_river(map, x, y);
			else if ((map_get_cell(map, x, y)->env == GRASSLAND || map_get_cell(map, x, y)->env == WOODLANDS) && rand > 9996)
				mapgen_plot_river(map, x, y);
		}
}

void mapgen_assign_heightmap_from_image(map_t *map, Image heightmap)
{
	map->mapg_data.heightmap = heightmap;
}

int ridgeify(float old_height, int sharpness)
{
	float new_height = 1.0f - fabs(((old_height/255.0f) - 0.5f) * 2.0f);
	return 255 * pow(new_height, sharpness);
}

int islands(float old_height, int sharpness)
{
	return 255 * pow((old_height / 255.0f), sharpness);
}

void mapgen_assign_heightmap(map_t *map, int ridge_sharpness, int island_sharpness)
{
	Image heightmap = GenImageColor(map->width, map->height, BLACK);
	Image perlin_ridges = GenImagePerlinNoise(map->width, map->height, seed_x, seed_y, 3.0f);
	Image perlin_macro = GenImagePerlinNoise(map->width, map->height, seed_x, seed_y, 0.5f);
	Image perlin_micro = GenImagePerlinNoise(map->width, map->height, seed_x, seed_y, 15.0f);
	Image cell = GenImageCellular(map->width, map->height, 10.0f);
	Image cutoff = GenImageGradientSquare(map->width, map->height, 0.35f, WHITE, BLACK);
	
	map->mapg_data.heightmap = heightmap;


	for (uint_t y = 0; y < map->height; y++)
		for (uint_t x = 0; x < map->width; x++)
		{
			ImageDrawPixel(&heightmap, x, y, val(
				clamp( 0,(
					  (int)p_val(perlin_macro, x, y)
					+ ridgeify(p_val(perlin_ridges, x, y), ridge_sharpness)
					+ (int)p_val(perlin_micro, x, y)
					+ islands(-p_val(cell, x, y), island_sharpness)
					) / 2, 255) 
				* ((float)p_val(cutoff, x , y) / 255.0f)
			));
		}
	mapgen_update_map_environments_from_heightmap(map);
}

void mapgen_populate_map(map_t *map, float amp, float randomness, float r_scale)
{
	Image perlin_pop = GenImagePerlinNoise(map->width, map->height, seed_x, seed_y, r_scale);
	for (uint_t y = 0; y < map->height; y++)
		for (uint_t x = 0; x < map->width; x++)
		{
			uchar_t cell_height = p_val(map->mapg_data.heightmap, x, y);
			cell_t *c_cell = map_get_cell(map, x, y);
			
			c_cell->pop_lvl = (cell_height > map->mapg_data.sea_level) * clamp(0, 255 - cell_height - map->mapg_data.sea_level, 255) * amp;
			c_cell->pop_lvl = clamp(0, c_cell->pop_lvl * ((((int)p_val(perlin_pop, x, y) + 255) * randomness) / 255.0f), 255);
		}
	UnloadImage(perlin_pop);
}

void mapgen_update_map_environments_from_heightmap(map_t *map)
{
	Image perlin_woods = GenImagePerlinNoise(map->width, map->height, seed_x, seed_y, 100.0f);
	
	uchar_t mountain_level = 255 - (255 - map->mapg_data.sea_level)*M_LVL_P;
	uchar_t highlands_level = 255 - (255 - map->mapg_data.sea_level)*HL_LVL_P;
	uchar_t hills_level = 255 - (255 - map->mapg_data.sea_level)*HLS_LVL_P;
	uchar_t flatlands_level = 255 - (255 - map->mapg_data.sea_level)*FLT_LVL_P;
	
	for (uint_t y = 0; y < map->height; y++)
	{
		for (uint_t x = 0; x < map->width; x++)
		{
			uchar_t cell_height = p_val(map->mapg_data.heightmap, x, y);
			cell_t *c_cell = map_get_cell(map, x, y);
			
			uchar_t wood_val = p_val(perlin_woods, x, y);

			if(cell_height > mountain_level)
			{
				c_cell->env = MOUNTAINS;
			}
			else if(cell_height > highlands_level)
				if(wood_val > 200)
				{
					c_cell->env = HIGHLANDS_WOODS;
				}
				else
				{
					c_cell->env = HIGHLANDS;
				}
			else if(cell_height > hills_level)
			{
				if(wood_val > 140)
				{
					c_cell->env = WOODLAND_HILLS;
				}
				else
				{
					c_cell->env = HILLS;
				}
			}
			else if(cell_height > flatlands_level)
			{
				if(wood_val > 127)
				{
					c_cell->env = GRASSLAND;
				}
				else
				{
					c_cell->env = WOODLANDS;
				}
			}
			else if(cell_height > map->mapg_data.sea_level + 1)
			{
				if(wood_val > 200)
				{
					c_cell->env = MARSH;
				}
				else
				{
					c_cell->env = FLOODPLAINS;
				}

			}
			else if(cell_height == map->mapg_data.sea_level + 1)
			{
					c_cell->env = BEACH;
			}
			else if(cell_height <= map->mapg_data.sea_level)
				c_cell->env = WATER;
			
				
		}
	}
	UnloadImage(perlin_woods);
}


map_t *mapgen_gen_from_heightmap(Image heightmap, uchar_t sea_level, long seed)
{
	ushort_t w = heightmap.width;
	ushort_t h = heightmap.height;

	map_t *map = map_create_map(w, h);
	map->mapg_data.heightmap = heightmap;
	map->mapg_data.sea_level = sea_level;
	map->mapg_data.seed = seed;
	
	mapgen_update_map_environments_from_heightmap(map);
	
	return map;
}

void mapgen_clear_region_ids(map_t *map)
{
	map->region_count = 0;
	for (uint_t y = 0; y < map->height; y++)
		for (uint_t x = 0; x < map->width; x++)
		{
			map_get_cell(map, x, y)->region_id = 0;
		}
}

#define MAX_B_CELLS 512

void add_border_cells(map_t *map, Pos_t *border_cells, ushort_t x, ushort_t y)
{
	bool add_S = (y < map->height - 1) && (map_get_cell(map, x, y + 1)->region_id == 0);
	bool add_N = (y > 0) && (map_get_cell(map, x, y - 1)->region_id == 0);
	bool add_E = (x < map->width - 1) && (map_get_cell(map, x + 1, y)->region_id == 0);
	bool add_W = (x > 0) && (map_get_cell(map, x - 1, y)->region_id == 0);

	for (int i = 0; i < MAX_B_CELLS; i++)
	{
		if(border_cells[i].x == 0 && border_cells[i].y == 0)
		{
			if(add_S)
				{border_cells[i] = pos(x, y + 1); add_S = false;}
			else if(add_N)
				{border_cells[i] = pos(x, y - 1); add_N = false;}
			else if(add_E)
				{border_cells[i] = pos(x + 1, y); add_E = false;}
			else if(add_W)
				{border_cells[i] = pos(x - 1, y); add_W = false;}
			else
				return;
		}
	}
	
}

ushort_t calc_cell_diff(map_t *map, cell_t *root_cell, ushort_t rt_x, ushort_t rt_y, ushort_t x, ushort_t y)
{
	cell_t *c_cell = map_get_cell(map, x, y);
	ushort_t distance = dst(rt_x, rt_y, x, y);
	if (c_cell->env == WATER || c_cell->env == RIVER)
		return 1000 + distance * 10;
	return abs((int)c_cell->env - (int)root_cell->env) + distance * 5;
}

bool cell_already_present(ulong_t *cell_ids, ushort_t cell_count, ulong_t id)
{
	for(ushort_t i = 0; i < cell_count; i++)
		if(cell_ids[i] == id)
		{
			return true;
		}

	return false;
}

void mapgen_plot_region(map_t *map, ushort_t rt_x, ushort_t rt_y, ushort_t region_id, uchar_t max_cells)
{
	#define xy_to_i(x, y) x + y * map->width

	cell_t *root_cell = map_get_cell(map, rt_x, rt_y);
	if(root_cell->region_id != 0 || root_cell->env == WATER)
		return;

	if(root_cell->env == MOUNTAINS)
		max_cells = MAX_REGION_CELL_CAP - 1;
		
	root_cell->region_id = region_id;

	uchar_t n_region_cells = 1;
	ulong_t region_cell_ids[max_cells];
	region_cell_ids[0] = xy_to_i(rt_x, rt_y);

	Pos_t border_cells[MAX_B_CELLS];
	memset(&border_cells[0], 0, MAX_B_CELLS * sizeof(Pos_t));
	add_border_cells(map, border_cells, rt_x, rt_y);
	cell_t *winner;
	bool border_cells_empty = false;
	for(int i = 0; i < max_cells; i++)
	{
		ushort_t border_winner;
		ushort_t lowest_diff = 65000;
		ushort_t c_diff;
		border_cells_empty = true;
		for(int j = 0; j < MAX_B_CELLS; j++)
		{
			if(border_cells[j].x != 0 && border_cells[j].y != 0)
			{
				border_cells_empty = false;
				c_diff = calc_cell_diff(map, root_cell, rt_x, rt_y, border_cells[j].x, border_cells[j].y);
				if(c_diff < lowest_diff)
				{
					lowest_diff = c_diff;
					border_winner = j;
				}
			}
		}
		if(border_cells_empty)
			break;
		
		winner = map_get_cell(map, border_cells[border_winner].x, border_cells[border_winner].y);
		if(watery(winner))
		{
			border_cells[border_winner] = pos(0,0);
			continue;
		}

		max_cells = clamp(0, max_cells - lowest_diff / 5, 255); // Additional cost to expensive expansion
		winner->region_id = region_id;

		// Skip duplicates
		if(cell_already_present(region_cell_ids, n_region_cells, xy_to_i(border_cells[border_winner].x, border_cells[border_winner].y)))
		{
			border_cells[border_winner] = pos(0,0);
			continue;
		}
		
		region_cell_ids[n_region_cells] = xy_to_i(border_cells[border_winner].x, border_cells[border_winner].y);
		n_region_cells++;
		
		add_border_cells(map, border_cells, border_cells[border_winner].x, border_cells[border_winner].y);
		border_cells[border_winner] = pos(0,0);
	}


	
	printf("\n\t[%u](%u) - r(%u, %u)", region_id, n_region_cells, rt_x, rt_y);
	map_add_region(map, region_cell_ids, n_region_cells);
}

ushort_t assign_region(map_t *map, ushort_t x, ushort_t y)
{

	Pos_t neighbours[4] = {pos(x, min((int)y + 1, map->height - 1)), pos(x, max((int)y - 1, 0)), pos(min((int)x + 1, map->width - 1), y), pos(max((int)x - 1, 0), y)};
	uchar_t i = (x + seed_x);
	for (uchar_t j = 0; j < 4; j++)
	{
		if(map_get_cell(map, neighbours[i%4].x, neighbours[i%4].y)->region_id != 0)
			return map_get_cell(map, neighbours[i%4].x, neighbours[i%4].y)->region_id;
		i++;
	}
	
	return 0;
}


/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. 

	Stolen from: https://stackoverflow.com/questions/6127503/shuffle-array-in-c
   */
void shuffle(Pos_t *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          Pos_t t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void mapgen_place_regions(map_t *map, uchar_t avg_cells, uchar_t cell_count_spread, uchar_t minimum_cellsin_rgn)
{
	#define xy_to_i(x, y) x + y * map->width
	ulong_t n_unclaimed = 0;
	Pos_t unclaimed[map->height * map->width];

	for (uint_t y = 0; y < map->height; y++)
		for (uint_t x = 0; x < map->width; x++)
		{
			cell_t *c_cell = map_get_cell(map, x, y);
			if(c_cell->env == WATER || c_cell->env == RIVER)
				continue;
			if(c_cell->region_id == 0)
			{
				unclaimed[n_unclaimed++] = pos(x, y);
			}
		}

	shuffle(unclaimed, n_unclaimed);
	ushort_t region_id;
	for(ulong_t i = 0; i < n_unclaimed; i++)
	{
		if(map_get_cell(map, unclaimed[i].x, unclaimed[i].y)->region_id != 0)
			continue;
		
		region_id = assign_region(map, unclaimed[i].x, unclaimed[i].y);
		if(region_id != 0)
		{
			map_get_cell(map, unclaimed[i].x, unclaimed[i].y)->region_id = region_id;
			map_add_cell_to_region(map, region_id, xy_to_i(unclaimed[i].x, unclaimed[i].y));
		}
		else
		{
			mapgen_plot_region(map, unclaimed[i].x, unclaimed[i].y, map->region_count, clamp(10, avg_cells + 
GetRandomValue((int)avg_cells - (int)cell_count_spread, (int)avg_cells + (int)cell_count_spread), MAX_REGION_CELL_CAP - 1));
		}
	}

	
	/// TODO: Prioritize mergers with physical borders (over only checking absolute distance from center)
	bool mergers_possible = true;
	while (mergers_possible)
	{
		mergers_possible = false;
		for (ushort_t i = 0; i < map->region_count; i++)
		{
			if(map->regions[i].cell_count < minimum_cellsin_rgn)
			{
				ulong_t first_cell_id = map->regions[i].cell_ids[0];
				Pos_t region_pos = map_cell_id_to_xy(map, first_cell_id);
				ushort_t nearest_region = find_nearest_region(map, region_pos.x, region_pos.y);
				if(map->regions[i].cell_count + map->regions[nearest_region].cell_count < MAX_REGION_CELL_CAP && nearest_region != 0)
				{
					if(map_merge_region_a_with_b(map, i, nearest_region, true))
					{
						printf("\n\t Merged [%u]->[%u]", i, nearest_region);
						mergers_possible = true;
					}
					else
						printf("\n\t Failed merge [%u]->[%u]", i, nearest_region);
				}
				
			}
		}
	}
	map_sync_region_ids(map);
	for (ushort_t i = 0; i < map->region_count; i++)
	{
		map_print_region(map, i);
	}
}

