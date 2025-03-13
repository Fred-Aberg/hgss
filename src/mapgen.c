#include <stdio.h>
#include <math.h>
#include "mapgen.h"
#include "raylib.h"
#include "c_codebase/src/common.h"

// Divide seed into two 16 bit numbers
#define seed_x ((int *)(&seed))[0] 
#define seed_y ((int *)(&seed))[1] 

// Divide seed into four 8 bit numbers
#define seed_b1 ((char *)(&seed))[0]
#define seed_b2 ((char *)(&seed))[1]
#define seed_b3 ((char *)(&seed))[2]
#define seed_b4 ((char *)(&seed))[3]

#define val(v) c(v,v,v)
#define p_val(image, x, y) GetImageColor(image, x, y).r

void mapgen_plot_river(map_t *map, Image heightmap, ushort_t x, ushort_t y)
{
	// printf("\n x:%u y:%u", x, y);
	x = clamp(1, x, map->width - 2);
	y = clamp(1, y, map->height - 2);
	Pos_t river_candidates[4] = {pos(x, y + 1), pos(x, y - 1), pos(x + 1, y), pos(x - 1, y)};
	Pos_t river_winner;
	uchar_t lowest_elev = 255;
	
	cell_t *c_cell = map_get_cell(map, x, y);

	if(c_cell->env == WATER) // Ocean reached
		return;

	if(c_cell->env == RIVER) // Join or create lake ?
		return;
	
	uint_t i = x + y; // Randomizing the order in which we check N S W E

	for (uint_t _i = 0; _i < 4; _i++)
	{
		if (p_val(heightmap, river_candidates[i%4].x, river_candidates[i%4].y) < lowest_elev)
		{
			river_winner = river_candidates[i%4];
			lowest_elev = p_val(heightmap, river_candidates[i%4].x, river_candidates[i%4].y);
		}
		i++;
	}

	if(lowest_elev == 255) // Failure
		return;
		
	c_cell->env = RIVER;
	c_cell->pop_lvl = 0;
	mapgen_plot_river(map, heightmap, river_winner.x, river_winner.y);
}

map_t *mapgen_gen_from_heightmap(char *file_path, uchar_t sea_level, long seed)
{
	Image heightmap = LoadImage(file_path);
	ushort_t w = heightmap.width;
	ushort_t h = heightmap.height;
	
	Image perlin_woods = GenImagePerlinNoise(w, h, seed_x, seed_y, 50.0f);
	
	uchar_t mountain_level = 255 - (255 - sea_level)*0.50f;
	uchar_t highlands_level = 255 - (255 - sea_level)*0.60f;
	uchar_t hills_level = 255 - (255 - sea_level)*0.80f;
	uchar_t flatlands_level = 255 - (255 - sea_level)*0.98f;

	map_t *map = map_create_map(w, h);
	map->heightmap = heightmap;
	
	for (uint_t y = 0; y < h; y++)
		for (uint_t x = 0; x < w; x++)
		{
			uchar_t cell_height = p_val(heightmap, x, y);
			cell_t *c_cell = map_get_cell(map, x, y);
			
			uchar_t wood_val = p_val(perlin_woods, x, y);
			c_cell->pop_lvl = (cell_height > sea_level) * clamp(0, 255 - cell_height - sea_level, 255) * 0.5f;

			if(cell_height > mountain_level)
			{
				c_cell->env = MOUNTAINS;
				c_cell->pop_lvl *= 0.10f;
			}
			else if(cell_height > highlands_level)
				if(wood_val > 200)
				{
					c_cell->env = HIGHLANDS_WOODS;
					c_cell->pop_lvl *= 0.20f;
				}
				else
				{
					c_cell->env = HIGHLANDS;
					c_cell->pop_lvl *= 0.40f;
				}
			else if(cell_height > hills_level)
			{
				if(wood_val > 140)
				{
					c_cell->env = WOODLAND_HILLS;
					c_cell->pop_lvl *= 0.35f;
				}
				else
				{
					c_cell->env = HILLS;
					c_cell->pop_lvl *= 0.70f;
				}
			}
			else if(cell_height > flatlands_level)
			{
				if(wood_val > 127)
				{
					c_cell->env = GRASSLAND;
					c_cell->pop_lvl *= 0.80f;
				}
				else
				{
					c_cell->env = WOODLANDS;
					c_cell->pop_lvl *= 0.45f;
				}
			}
			else if(cell_height > sea_level)
			{
				if(wood_val > 200)
				{
					c_cell->env = MARSH;
					c_cell->pop_lvl *= 0.35f;
				}
				else if(wood_val > 160)
				{
					c_cell->env = FLOODPLAINS;
				}
				else
				{
					c_cell->env = BEACH;
					c_cell->pop_lvl *= 0.5f;
				}
			}
			else if(cell_height <= sea_level)
				c_cell->env = WATER;
			
				
		}
	UnloadImage(perlin_woods);
	return map;
}

void mapgen_hmap_ridges(Image *heightmap, float scale, float amp, long seed)
{
	Image perlin_ridges = GenImagePerlinNoise(heightmap->width, heightmap->height, seed_x, seed_y, scale);
	for (int y = 0; y < heightmap->height; y++)
		for (int x = 0; x < heightmap->width; x++)
			ImageDrawPixel(heightmap, x, y, val(255 - abs(((int)p_val(perlin_ridges, x, y) - 127) * 2) * amp));
	UnloadImage(perlin_ridges);
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
map_t *mapgen_gen_continent(ushort_t w, ushort_t h, uchar_t sea_level, long seed)
{
	map_t *map = map_create_map(w, h);

	Image heightmap = GenImageColor(w, h, BLACK);
	
	Image perlin_ridges = GenImagePerlinNoise(w, h, seed_x, seed_y, 3.0f);
	Image perlin_macro = GenImagePerlinNoise(w, h, seed_x, seed_y, 0.5f);
	Image perlin_micro = GenImagePerlinNoise(w, h, seed_x, seed_y, 15.0f);
	Image perlin_woods = GenImagePerlinNoise(w, h, seed_x, seed_y, 50.0f);
	Image cell = GenImageCellular(w, h, 10.0f);
	Image cutoff = GenImageGradientSquare(w, h, 0.35f, WHITE, BLACK);
	
	map->heightmap = heightmap;

	uchar_t mountain_level = 255 - (255 - sea_level)*0.35f;
	uchar_t highlands_level = 255 - (255 - sea_level)*0.50f;
	uchar_t hills_level = 255 - (255 - sea_level)*0.60f;
	uchar_t flatlands_level = 255 - (255 - sea_level)*0.90f;


	for (uint_t y = 0; y < h; y++)
		for (uint_t x = 0; x < w; x++)
		{
			ImageDrawPixel(&heightmap, x, y, val(
				clamp( 0,(
					  (int)p_val(perlin_macro, x, y)
					+ ridgeify(p_val(perlin_ridges, x, y), 10)
					+ (int)p_val(perlin_micro, x, y)
					+ islands(-p_val(cell, x, y), 8)
					) / 2, 255) 
				* ((float)p_val(cutoff, x , y) / 255.0f)
			));
			
			uchar_t cell_height = p_val(heightmap, x, y);
			uchar_t wood_val = p_val(perlin_woods, x, y);

			cell_t *c_cell = map_get_cell(map, x, y);
			
			c_cell->pop_lvl = (cell_height > sea_level) * min(0, (int)p_val(cell, x, y) - cell_height - sea_level);

			if(cell_height > mountain_level)
			{
				c_cell->env = MOUNTAINS;
				c_cell->pop_lvl *= 0.10f;
			}
			else if(cell_height > highlands_level)
				if(wood_val > 200)
				{
					c_cell->env = HIGHLANDS_WOODS;
					c_cell->pop_lvl *= 0.20f;
				}
				else
				{
					c_cell->env = HIGHLANDS;
					c_cell->pop_lvl *= 0.40f;
				}
			else if(cell_height > hills_level)
			{
				if(wood_val > 140)
				{
					c_cell->env = WOODLAND_HILLS;
					c_cell->pop_lvl *= 0.35f;
				}
				else
				{
					c_cell->env = HILLS;
					c_cell->pop_lvl *= 0.70f;
				}
			}
			else if(cell_height > flatlands_level)
			{
				if(wood_val > 127)
				{
					c_cell->env = GRASSLAND;
					c_cell->pop_lvl *= 0.80f;
				}
				else
				{
					c_cell->env = WOODLANDS;
					c_cell->pop_lvl *= 0.45f;
				}
			}
			else if(cell_height > sea_level)
			{
				if(wood_val > 200)
				{
					c_cell->env = MARSH;
					c_cell->pop_lvl *= 0.35f;
				}
				else if(wood_val > 160)
				{
					c_cell->env = FLOODPLAINS;
				}
				else
				{
					c_cell->env = BEACH;
					c_cell->pop_lvl *= 0.5f;
				}
			}
			else if(cell_height <= sea_level)
				c_cell->env = WATER;
			
				
		}
	for (uint_t y = 0; y < h; y++)
		for (uint_t x = 0; x < w; x++)
			if (map_get_cell(map, x, y)->env == MOUNTAINS && GetRandomValue(0, 200) > 195)
				mapgen_plot_river(map, heightmap, x, y);
			else if (map_get_cell(map, x, y)->env == HIGHLANDS && GetRandomValue(0, 200) > 198)
				mapgen_plot_river(map, heightmap, x, y);
			else if ((map_get_cell(map, x, y)->env == HILLS || map_get_cell(map, x, y)->env == WOODLAND_HILLS) && GetRandomValue(0, 200) > 199)
				mapgen_plot_river(map, heightmap, x, y);
			else if ((map_get_cell(map, x, y)->env == GRASSLAND || map_get_cell(map, x, y)->env == WOODLANDS) && GetRandomValue(0, 200) > 199)
				mapgen_plot_river(map, heightmap, x, y);


			
	UnloadImage(perlin_ridges);
	UnloadImage(perlin_macro);
	UnloadImage(perlin_micro);
	UnloadImage(cell);
	UnloadImage(perlin_woods);
	UnloadImage(cutoff);

	return map;
}
