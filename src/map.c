#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "map.h"
#include "c_codebase/src/common.h"

#define SYMBOL_CULL_P_W 8

// Om buf e liten så kys
void map_env_to_str(char *buf, environment_e env)
{
	switch (env)
	{
		case WATER:
			strcpy(buf, "Ocean");
			return;
		case RIVER:
			strcpy(buf, "River");
			return;
		case MARSH:
			strcpy(buf, "Marsh");
			return;
		case FLOODPLAINS:
			strcpy(buf, "Floodplains");
			return;
		case BEACH:
			strcpy(buf, "Beach");
			return;
		case URBAN:
			strcpy(buf, "Urban");
			return;
		case RUINS:
			strcpy(buf, "Ruins");
			return;
		case GRASSLAND:
			strcpy(buf, "Grassland");
			return;
		case WOODLANDS:
			strcpy(buf, "Woodlands");
			return;
		case HILLS:
			strcpy(buf, "Hills");
			return;
		case WOODLAND_HILLS:
			strcpy(buf, "Wooded Hills");
			return;
		case HIGHLANDS:
			strcpy(buf, "Highlands");
			return;
		case HIGHLANDS_WOODS:
			strcpy(buf, "Wooded Highlands");
			return;
		case MOUNTAINS:
			strcpy(buf, "Mountains");
			return;
		default:
			strcpy(buf, "???");
			return;
	}
}

// pos(food, prod)
Pos_t map_env_base_yields(environment_e env)
{
	switch (env)
	{
		case WATER:
			return pos(0, 0);
		case RIVER:
			return pos(8, 2);
		case MARSH:
			return pos(2, 2);
		case FLOODPLAINS:
			return pos(16, 4);
		case BEACH:
			return pos(8, 2);
		case URBAN:
			return pos(0, 20);
		case RUINS:
			return pos(2, 2);
		case GRASSLAND:
			return pos(12, 4);
		case WOODLANDS:
			return pos(6, 8);
		case HILLS:
			return pos(4, 10);
		case WOODLAND_HILLS:
			return pos(2, 8);
		case HIGHLANDS:
			return pos(2, 4);
		case HIGHLANDS_WOODS:
			return pos(0, 6);
		case MOUNTAINS:
			return pos(0, 1);
		default:
			return pos(0, 0);
	}
}

Pos_t map_cell_id_to_xy(map_t *map, ulong_t i)
{
	ushort_t y = i / map->width;
	return pos(i - y * map->width, y);
}

cell_t *map_get_cell(map_t *map, ushort_t x, ushort_t y)
{
	if(x >= map->width && y >= map->height)
	{
    fprintf(stderr, "map.c: get cell overflow (x,y)=(%u, %u) [w,h]=[%u, %u]\n", 
    		x, y, map->width, map->height);
    assert(false); // kys	
	}
    return &map->cells[x + y * map->width];
}

void map_print_region(map_t *map, ushort_t region_id)
{
	printf("\n\nR[%u] c: %u/%u", region_id, map->regions[region_id].cell_count, map->regions[region_id].cell_capacity);
	for (ushort_t i = 0; i < map->regions[region_id].cell_count; i++)
	{
		cell_t c = map->cells[map->regions[region_id].cell_ids[i]];
		Pos_t c_pos = map_cell_id_to_xy(map, map->regions[region_id].cell_ids[i]);
		printf("\n\tr[%u], id[%lu], (%u, %u)", c.region_id, map->regions[region_id].cell_ids[i], c_pos.x, c_pos.y);
	}
}

map_t *map_create_map(uint_t width, uint_t height)
{
	map_t *map = calloc(1, sizeof(map_t));
	map->width = width;
	map->height = height;
	map->cells = calloc(width*height, sizeof(cell_t));
	map->region_count = 1;
	map->region_capacity = 10;
	map->regions = calloc(10, sizeof(region_t));

	return map;
}

int comp_cells_qs(const void *id_a, const void *id_b)
{
	return (int)(*(signed long long *)id_a - *(signed long long *)id_b);
}

llong_t comp_cells(ulong_t a, ulong_t b)
{
	return (int)((signed long long)a - (signed long long)b);
}

#define REALLOC_PERCENTAGE_INCREASE 1.5f
region_t *map_add_region(map_t *map, ulong_t *cell_ids, uchar_t cell_count)
{
	// if(cell_count == 0)
		// return NULL;
	
	if (map->region_capacity <= map->region_count)
	{
		map->region_capacity = map->region_capacity * REALLOC_PERCENTAGE_INCREASE + 1; 
		map->regions = (region_t *)realloc(map->regions, (map->region_capacity * sizeof(region_t)));
	}
	
	region_t region;
	region.cell_count = cell_count;
	region.cell_capacity = cell_count;
	if(cell_count != 0)
	{
		region.cell_ids = calloc(cell_count, sizeof(ulong_t));
		memcpy(region.cell_ids, cell_ids, cell_count * sizeof(ulong_t));
		qsort(region.cell_ids, cell_count, sizeof(ulong_t), comp_cells_qs);
		
		// ulong_t first_id = region.cell_ids[0];
		// 
		// llong_t comparison;
		// for (ushort_t i = 0; i < map->region_count; i++)
		// {
			// comparison = comp_cells(first_id, map->regions[i].cell_ids[0]);
			// if(comparison > 0)
			// {
				// // Move all regions from and including i one step forward
				// memmove(&map->regions[i + 1], &map->regions[i], (map->region_count - i) * sizeof(region_t));
				// 
				// map->regions[i] = region;
				// map->region_count++;
				// return &map->regions[i];
			// }
		// }
	}

	// Add emoty region -> append
	map->regions[map->region_count] = region;
	map->region_count++;
	return &map->regions[map->region_count - 1];
}

void map_sync_region_ids(map_t *map)
{
	for (ushort_t i = 0; i < map->region_count; i++)
		for (ushort_t j = 0; j < map->regions[i].cell_count; j++)
		{
			map->cells[map->regions[i].cell_ids[j]].region_id = i;
		}
}

void map_remove_region(map_t *map, ushort_t region_id, bool sync_region_ids)
{
	free(map->regions[region_id].cell_ids);
	memmove(&map->regions[region_id], &map->regions[region_id + 1], (map->region_count - region_id) * sizeof(region_t));
	map->region_count--;

	if(sync_region_ids)
		map_sync_region_ids(map);
}

void map_rebake_cell_yields(cell_t *cell_array, ulong_t cell_id)
{
	ushort_t pop_multiplier = (cell_array[cell_id].pop_lvl>>2);
	
	Pos_t base_env_yields = map_env_base_yields(cell_array[cell_id].env);
	cell_array[cell_id].baked_food_yield = base_env_yields.x * pop_multiplier;
	cell_array[cell_id].baked_production_yield = base_env_yields.y * pop_multiplier;
}

void map_rebake_cell_yields_propagate(region_t *region_array, cell_t *cell_array, ulong_t cell_id)
{
	float pop_multiplier = 1.0f + (cell_array[cell_id].pop_lvl / 255.0f);
	
	Pos_t base_env_yields = map_env_base_yields(cell_array[cell_id].env);
	cell_array[cell_id].baked_food_yield = base_env_yields.x * pop_multiplier;
	cell_array[cell_id].baked_production_yield = base_env_yields.y * pop_multiplier;
	

	// TODO: Subtract consumption
	region_array[cell_array[cell_id].region_id].baked_food_yield += base_env_yields.x * pop_multiplier;
	region_array[cell_array[cell_id].region_id].baked_production_yield += base_env_yields.y * pop_multiplier;
	region_array[cell_array[cell_id].region_id].baked_gold_yield += 20 * pop_multiplier;
}

void map_rebake_region_yields(region_t *region_array, cell_t *cell_array, ushort_t region_id)
{
	region_array[region_id].baked_food_yield = 0;
	region_array[region_id].baked_production_yield = 0;
	region_array[region_id].baked_gold_yield = 0;

	ulong_t *cell_ids = region_array[region_id].cell_ids;
	cell_t c;

	for(uchar_t i = 0; i < region_array[region_id].cell_count; i++)
	{
		c = cell_array[cell_ids[i]];
		
		region_array[region_id].baked_food_yield += (short)c.baked_food_yield - (c.pop_lvl>>4);
		region_array[region_id].baked_production_yield += (short)c.baked_production_yield - (c.pop_lvl>>4);
		region_array[region_id].baked_gold_yield += (c.pop_lvl>>4);	
	}
}

void map_rebake_yields(map_t *map)
{
	ulong_t n_cells = map->width * map->height;
	for(ulong_t i = 0; i < n_cells; i++)
	{
		map_rebake_cell_yields(map->cells, i);
	}

	for(ushort_t i = 0; i < map->region_count; i++)
	{
		map_rebake_region_yields(map->regions, map->cells, i);
	}
}

void map_add_cell_to_region(map_t *map, ushort_t region_id, ulong_t cell_id)
{
	region_t *region = &map->regions[region_id];
	if(region->cell_count == 255)
	{
		WARNINGF("Region [%u] is at cell capacity!", region_id)
		return;
	}
	if (region->cell_capacity <= region->cell_count)
	{
		region->cell_capacity = min(region->cell_capacity * REALLOC_PERCENTAGE_INCREASE + 1, MAX_REGION_CELL_CAP);
		region->cell_ids = (ulong_t *)realloc(region->cell_ids, (region->cell_capacity * sizeof(ulong_t)));
	}
	region->cell_ids[region->cell_count] = cell_id;
	region->cell_count++;
}

void map_add_cells_to_region(map_t *map, ushort_t region_id, ulong_t *cell_ids, uchar_t cell_count)
{
	region_t *region = &map->regions[region_id];
	if((int)region->cell_count + (int)cell_count >= MAX_REGION_CELL_CAP)
	{
		WARNINGF("Region [%u] is at cell capacity!", region_id)
		return;
	}
	if (region->cell_capacity <= region->cell_count + cell_count)
	{
		region->cell_capacity = min(region->cell_capacity * REALLOC_PERCENTAGE_INCREASE + 1, MAX_REGION_CELL_CAP);
		region->cell_ids = (ulong_t *)realloc(region->cell_ids, (region->cell_capacity * sizeof(ulong_t)));
	}

	for (ushort_t i = 0; i < cell_count; i++)
	{
		map->cells[cell_ids[i]].region_id = region_id; // set region incase it hasn't already been done
		region->cell_ids[region->cell_count] = cell_ids[i];
		region->cell_count++;
	}
}

bool map_merge_region_a_with_b(map_t *map, ushort_t region_id_a, ushort_t region_id_b, bool sync_region_ids)
{
	region_t *region_a = &map->regions[region_id_a];
	region_t *region_b = &map->regions[region_id_b];
	if((int)region_a->cell_count + (int)region_b->cell_count >= MAX_REGION_CELL_CAP)
	{
		WARNINGF("Regions [%u] & [%u] Cannot be merged! cell counts to high.", region_id_a, region_id_b)
		return false;
	}

	// if(region_b->cell_capacity < MAX_REGION_CELL_CAP)
	// {
		// region_b->cell_capacity = MAX_REGION_CELL_CAP;
		// region_b->cell_ids = (ulong_t *)realloc(region_b->cell_ids, (region_b->cell_capacity * sizeof(ulong_t)));
	// }
	if (region_b->cell_capacity < region_a->cell_count + region_b->cell_count)
	{
		region_b->cell_capacity += region_a->cell_count;
		region_b->cell_ids = (ulong_t *)realloc(region_b->cell_ids, (region_b->cell_capacity * sizeof(ulong_t)));
	}

	for (ushort_t i = 0; i < region_a->cell_count; i++)
	{
		map->cells[region_a->cell_ids[i]].region_id = region_id_b; // set region incase it hasn't already been done
		region_b->cell_ids[region_b->cell_count] = region_a->cell_ids[i];
		region_b->cell_count++;
	}

	map_remove_region(map, region_id_a, sync_region_ids);
}

void map_remove_cell_from_region(ushort_t region_id, ulong_t cell_id);

void map_remove_cells_from_region(ushort_t region_id, ulong_t *cell_ids);

void map_remove_cells_from_region(ushort_t region_id, ulong_t *cell_id);

ushort_t map_get_region_pop(map_t *map, ushort_t region_id)
{
	ushort_t tot_pop = 0;
	region_t region = map->regions[region_id];

	for (ushort_t i = 0; i < region.cell_count; i++)
	{
		map->cells[region.cell_ids[i]].region_id = region_id;
		tot_pop += (ushort_t)map->cells[region.cell_ids[i]].pop_lvl;
	}

	return tot_pop;
}

color8b_t region_id_to_col(ushort_t n_regions, ushort_t region_id)
{
	if(region_id == 0)
		return BLACK8B;

	return ((float)region_id / (float)n_regions) * 255;
	// uchar_t r = (((char *)&region_id)[0] * 3) % 8;
	// uchar_t g = (((char *)&region_id)[0] * 7) % 8;
	// uchar_t b = (((char *)&region_id)[0] * 5) % 4;
	// color8b_t col = col8bt(r, g, b);
	// return (col == 0)? col8bt(0, 0, 1) : col;
}

#define cg(icon, bg_col, char_col) (cell_graphics_t){bg_col, char_col, icon}

cell_graphics_t map_calc_cell_graphics(cell_t cell)
{
	const color8b_t WATER_A = col8bt(0, 1, 3);
	const color8b_t WATER_B = col8bt(0, 2, 3);

	const color8b_t RIVER_A = col8bt(0, 2, 3);
	const color8b_t RIVER_B = col8bt(0, 3, 3);

	const color8b_t BEACH_A = col8bt(6, 6, 0);
	const color8b_t BEACH_B = col8bt(5, 5, 0);

	const color8b_t CLEAR_GREEN_A = col8bt(1, 5, 0);
	const color8b_t CLEAR_GREEN_B = col8bt(1, 7, 1);

	const color8b_t WOODED_A = col8bt(0, 3, 0);
	const color8b_t WOODED_B = col8bt(0, 5, 0);
	
	const color8b_t HILLS_A = col8bt(2, 4, 1);
	const color8b_t HILLS_B = col8bt(2, 6, 1);
	const color8b_t WOODLAND_HILLS_B = col8bt(2, 6, 1);

	const color8b_t HIGHLANDS_A = col8bt(3, 4, 1);
	const color8b_t HIGHLANDS_B = col8bt(4, 6, 2);

	const color8b_t MOUNTAINS_A = col8bt(3, 3, 1);
	const color8b_t MOUNTAINS_B = col8bt(4, 4, 2);

	const color8b_t URBAN_A = col8bt(4, 4, 2);
	const color8b_t URBAN_B = col8bt(2, 2, 1);
		
	bool rand = (long)(&cell) % 2 == 0;
	switch (cell.env)
	{
		case WATER:
			if(rand)
				return cg('0', WATER_A, WATER_B);
			else
				return cg('1', WATER_A, WATER_B);
		case RIVER:
			if(rand)
				return cg('2', RIVER_A, RIVER_B);
			else
				return cg('3', RIVER_A, RIVER_B);
		case MARSH:
			return cg('8', CLEAR_GREEN_B, WOODED_A);
		case FLOODPLAINS:
			return cg('7', CLEAR_GREEN_B, CLEAR_GREEN_A);
		case BEACH:
			return cg('4', BEACH_A, BEACH_B);
		case URBAN:
			if (rand)
				return cg('#', URBAN_A, URBAN_B);
			else
				return cg('$', URBAN_A, URBAN_B);
		case RUINS:
			return cg('%', URBAN_A, URBAN_B);
		case GRASSLAND:
			return cg('7', CLEAR_GREEN_A, CLEAR_GREEN_B);
		case WOODLANDS:
			return cg('5', WOODED_A, WOODED_B);
		case HILLS:
			return cg('4', HILLS_A, HILLS_B);
		case WOODLAND_HILLS:
			return cg('6', HILLS_A, WOODED_A);
		case HIGHLANDS:
			return cg('9', HIGHLANDS_A, HIGHLANDS_B);
		case HIGHLANDS_WOODS:
			return cg('!', HIGHLANDS_A, HIGHLANDS_B);
		case MOUNTAINS:
			return cg('"', MOUNTAINS_A, MOUNTAINS_B);
		default:
			return cg('E', BLACK8B, WHITE8B);
	}
}

#define col_hmap(c) tl_Color_to_color8b((Color){min((int)c + 32, 255), min((int)c + 32, 255), 0, 0})
#define col_pop(c) tl_Color_to_color8b((Color){min((int)c + 32, 255), min((int)c * 2 + 32, 255), 0,0})
#define col_char(c) tl_Color_to_color8b((Color){c, c, c, c})
#define p_val(image, x, y) GetImageColor(image, x, y).r

bool border_cell(map_t *map, ushort_t region_id, ushort_t x, ushort_t y)
{
	if(region_id == 0)
		return false;
	Pos_t neighbours[4] = {pos(x, min((int)y + 1, map->height - 1)), pos(x, max((int)y - 1, 0)), pos(min((int)x + 1, map->width - 1), y), pos(max((int)x - 1, 0), y)};

	for (uchar_t i = 0; i < 4; i++)
		if(map_get_cell(map, neighbours[i].x, neighbours[i].y)->region_id != region_id)
			return true;
	return false;
}

void draw_map_default(grid_t *grid, map_t *map, uchar_t g_x0, uchar_t g_y0, uchar_t g_x1, uchar_t g_y1, ushort_t map_x0, ushort_t map_y0, char map_font, cell_t *selected_cell,
						color8b_t t_delta_col, color8b_t t_delta_col_invert)
{
	ushort_t selected_region_id = (selected_cell != NULL)? selected_cell->region_id : 0;
	
	int map_display_width = g_x1 - g_x0;
	int map_display_height = g_y1 - g_y0;

	cell_graphics_t cg;
	cell_t *c_cell;
	const color8b_t WATER_A = col8bt(0, 1, 3);
	
	for (int _y = 0; _y < map_display_height; _y++)
		{
			color8b_t c_interval_bg;
			ushort_t c_interval_x0 = 0;
			ushort_t c_interval_x1 = 0;
			for (int _x = 0; _x < map_display_width; _x++)
			{
				c_cell = map_get_cell(map, map_x0 + _x, map_y0 + _y);
	
				
				if(c_cell->env == WATER || c_cell->env == RIVER)
					cg = map_calc_cell_graphics(*c_cell);
				

				cg = map_calc_cell_graphics(*c_cell);
				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;
				else if(border_cell(map, c_cell->region_id, map_x0 + _x,  map_y0 + _y))
				{
					if(c_cell->region_id != selected_region_id)
						cg.bg_col = region_id_to_col(map->region_count, c_cell->region_id);
					else if(selected_region_id != 0)
						cg.bg_col = t_delta_col;
				}
					
				if(_x == 0)
				{
					c_interval_bg = cg.bg_col;
				}
				else
				{
					if(c_interval_bg == cg.bg_col)
					{
						c_interval_x1++;
					}
					else
					{
						if(c_interval_bg != WATER_A) // Cull water backgrounds
							tl_draw_rect_bg(grid, g_x0 + c_interval_x0, g_y0 + _y, g_x0 + c_interval_x1, g_y0 + _y, c_interval_bg);
						c_interval_bg = cg.bg_col;
						c_interval_x0 = _x;
						c_interval_x1 = _x;
					}
	
				}
			}
			// Draw last interval
			if(c_interval_bg != WATER_A) // Cull water backgrounds
				tl_draw_rect_bg(grid, g_x0 + c_interval_x0, g_y0 + _y, g_x0 + c_interval_x1, g_y0 + _y, c_interval_bg);

			// Draw symbols
			if(grid->tile_p_w >= SYMBOL_CULL_P_W)
				for (int _x = 0; _x < map_display_width; _x++)
				{
					cg = map_calc_cell_graphics(*map_get_cell(map, map_x0 + _x, map_y0 + _y));
					tl_plot_smbl(grid, g_x0 + _x, g_y0 + _y, cg.icon, cg.char_col, map_font);
				}
		}
}

void draw_map_terrain(grid_t *grid, map_t *map, uchar_t g_x0, uchar_t g_y0, uchar_t g_x1, uchar_t g_y1, ushort_t map_x0, ushort_t map_y0, char map_font, cell_t *selected_cell,
						color8b_t t_delta_col, color8b_t t_delta_col_invert)
{
	ushort_t selected_region_id = (selected_cell != NULL)? selected_cell->region_id : 0;
	
	int map_display_width = g_x1 - g_x0;
	int map_display_height = g_y1 - g_y0;

	cell_graphics_t cg;
	cell_t *c_cell;
	const color8b_t WATER_A = col8bt(0, 1, 3);
	
	for (int _y = 0; _y < map_display_height; _y++)
		{
			color8b_t c_interval_bg;
			ushort_t c_interval_x0 = 0;
			ushort_t c_interval_x1 = 0;
			for (int _x = 0; _x < map_display_width; _x++)
			{
				c_cell = map_get_cell(map, map_x0 + _x, map_y0 + _y);
	
				
				if(c_cell->env == WATER || c_cell->env == RIVER)
					cg = map_calc_cell_graphics(*c_cell);
				

				cg = map_calc_cell_graphics(*c_cell);
				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;
				else if(selected_region_id != 0 && selected_region_id == c_cell->region_id)
					cg.bg_col = t_delta_col;
				
					
				if(_x == 0)
				{
					c_interval_bg = cg.bg_col;
				}
				else
				{
					if(c_interval_bg == cg.bg_col)
					{
						c_interval_x1++;
					}
					else
					{
						if(c_interval_bg != WATER_A) // Cull water backgrounds
							tl_draw_rect_bg(grid, g_x0 + c_interval_x0, g_y0 + _y, g_x0 + c_interval_x1, g_y0 + _y, c_interval_bg);
						c_interval_bg = cg.bg_col;
						c_interval_x0 = _x;
						c_interval_x1 = _x;
					}
	
				}
			}
			// Draw last interval
			if(c_interval_bg != WATER_A) // Cull water backgrounds
				tl_draw_rect_bg(grid, g_x0 + c_interval_x0, g_y0 + _y, g_x0 + c_interval_x1, g_y0 + _y, c_interval_bg);

			// Draw symbols
			if(grid->tile_p_w >= SYMBOL_CULL_P_W)
				for (int _x = 0; _x < map_display_width; _x++)
				{
					cg = map_calc_cell_graphics(*map_get_cell(map, map_x0 + _x, map_y0 + _y));
					tl_plot_smbl(grid, g_x0 + _x, g_y0 + _y, cg.icon, cg.char_col, map_font);
				}
		}
}

// Draw at the same time as rebaking? Looping through the cells only once?
void map_draw_map_onto_grid(grid_t *grid, map_t *map, Pos_t cam_map_pos, char map_font, map_visualisation_e vis, cell_t *selected_cell)
{
	
	ushort_t selected_region_id = (selected_cell != NULL)? selected_cell->region_id : 0;

	uchar_t t_char = ((sin(4 * GetTime()) + 1.0f)/2.0f) * 255.0f;
	color8b_t t_delta_col = col_char(t_char);
	color8b_t t_delta_col_invert = col_char(255 - t_char);
	

	Pos_t grid_dimensions = tl_grid_get_dimensions(grid);
	int g_cam_x = (int)grid_dimensions.x / 2;
	int g_cam_y = (int)grid_dimensions.y / 2;


	int g_x0 = max(g_cam_x - cam_map_pos.x, 0);
	int g_y0 = max(g_cam_y - cam_map_pos.y, 0);
	int g_x1 = min(g_cam_x + (map->width - cam_map_pos.x), grid_dimensions.x);
	int g_y1 = min(g_cam_y + (map->height - cam_map_pos.y), grid_dimensions.y);

	// int cam_up = min(cam_map_pos.y, g_cam_y);
	// int cam_down = min(map->height - cam_map_pos.y, g_cam_y);
	// int cam_right = min(map->width - cam_map_pos.x, g_cam_x);
	// int cam_left = g_cam_x - g_x0;
	
	int map_x0 = cam_map_pos.x - (g_cam_x - g_x0);
	int map_y0 = cam_map_pos.y - (g_cam_y - g_y0);

	// int map_x0 = max(0, (int)cam_map_pos.x - g_cam_x);
	// int map_y0 = max(0, (int)cam_map_pos.y - g_cam_y);
	// int map_x1 = min(map->width, cam_map_pos.x + g_cam_x);
	// int map_y1 = min(map->height, cam_map_pos.y + g_cam_y);

	const color8b_t WATER_A = col8bt(0, 1, 3);
	tl_draw_rect_bg(grid,g_x0, g_y0, g_x1, g_y1, WATER_A);

	switch (vis)
	{
		case DEFAULT:
		draw_map_default(grid, map, g_x0, g_y0, g_x1, g_y1, map_x0, map_y0, map_font, selected_cell, t_delta_col, t_delta_col_invert);
		return;
		case TERRAIN:
		draw_map_terrain(grid, map, g_x0, g_y0, g_x1, g_y1, map_x0, map_y0, map_font, selected_cell, t_delta_col, t_delta_col_invert);
		return;
	}
	

	int map_display_width = g_x1 - g_x0;
	int map_display_height = g_y1 - g_y0;

	
	cell_graphics_t cg;
	tl_draw_rect_bg(grid,g_x0, g_y0, g_x1, g_y1, WATER_A);

	cell_t *c_cell;

	for (int _y = 0; _y < map_display_height; _y++)
	{
		color8b_t c_interval_bg;
		ushort_t c_interval_x0 = 0;
		ushort_t c_interval_x1 = 0;
		for (int _x = 0; _x < map_display_width; _x++)
		{
			c_cell = map_get_cell(map, map_x0 + _x, map_y0 + _y);

			
			if(c_cell->env == WATER || c_cell->env == RIVER)
				cg = map_calc_cell_graphics(*c_cell);
			else if(vis == POPULATION)
			{
				cg = cg('\0', col_pop(c_cell->pop_lvl), BLACK8B);
				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;
				else if(c_cell->region_id == selected_region_id)
					cg.bg_col = t_delta_col;
			}
			else if(vis == REGIONS)
			{
				cg = cg('\0', region_id_to_col(map->region_count, c_cell->region_id), BLACK8B);
				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;
				else if(c_cell->region_id == selected_region_id)
					cg.bg_col = t_delta_col;
			}
			else if(vis == HEIGHTMAP)
			{
				cg = cg('\0', col_hmap(GetImageColor(map->mapg_data.heightmap, map_x0 + _x, map_y0 + _y).r), BLACK8B);
				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;
				else if(c_cell->region_id == selected_region_id)
					cg.bg_col = t_delta_col;
			}
			if(_x == 0)
			{
				c_interval_bg = cg.bg_col;
			}
			else
			{
				if(c_interval_bg == cg.bg_col)
				{
					c_interval_x1++;
				}
				else
				{
					if(c_interval_bg != WATER_A) // Cull water backgrounds
						tl_draw_rect_bg(grid, g_x0 + c_interval_x0, g_y0 + _y, g_x0 + c_interval_x1, g_y0 + _y, c_interval_bg);
					c_interval_bg = cg.bg_col;
					c_interval_x0 = _x;
					c_interval_x1 = _x;
				}

			}
		}
		// Draw last interval
		tl_draw_rect_bg(grid, g_x0 + c_interval_x0, g_y0 + _y, g_x0 + c_interval_x1, g_y0 + _y, c_interval_bg);
		if(vis == DEFAULT)
			for (int _x = 0; _x < map_display_width; _x++)
				if(grid->tile_p_w >= SYMBOL_CULL_P_W)
				{
					cg = map_calc_cell_graphics(*map_get_cell(map, map_x0 + _x, map_y0 + _y));
					tl_plot_smbl(grid, g_x0 + _x, g_y0 + _y, cg.icon, cg.char_col, map_font);
				}
	}
	tl_plot_smbl_w_bg(grid, g_cam_x, g_cam_y, 'O', BLACK8B, WHITE8B, 1);
	tl_plot_smbl_w_bg(grid, g_x0, g_y0, 'X', BLACK8B, WHITE8B, 1);
	tl_plot_smbl_w_bg(grid, g_x1 - 1, g_y1 - 1, 'X', BLACK8B, WHITE8B, 1);
}

Pos_t map_grid_pos_to_map_pos(grid_t *grid, map_t *map, Pos_t grid_pos, Pos_t camera_position)
{
	Pos_t grid_dimensions = tl_grid_get_dimensions(grid);
	int g_cam_x = (int)grid_dimensions.x / 2;
	int g_cam_y = (int)grid_dimensions.y / 2;
	return pos(clamp(0, (int)camera_position.x - (g_cam_x - (int)grid_pos.x), map->width - 1), 
			   clamp(0, (int)camera_position.y - (g_cam_y - (int)grid_pos.y), map->height - 1));
}

// Pos_t map_map_pos_to_grid_pos(map_t *map, Pos_t grid_pos, Pos_t camera_position)
// {
	// 
// }

/*

char set_bit(char byte, uint_t bit, bool bitv)
{
	if(bitv)
		return byte | (1<<bit);
	else
		return byte ^ (1<<bit);
}

#define set_byte(bmap, byte_x, y, b) bmap->flag_bytes[y][byte_x] = b

bmap_t *bmap_create(map_t *map)
{
	bmap_t *bmap = calloc(1, sizeof(bmap_t));
	bmap->map = map;
	bmap->b_w = (map->width / 8) + 1;
	bmap->b_h = map->height;
	
	bmap->flag_bytes = calloc(bmap->b_h, sizeof(char *));
	for (ushort_t y = 0; y < bmap->b_h; y++)
		bmap->flag_bytes[y] = calloc(bmap->b_w, sizeof(1));
	return bmap;
}

void bmap_bake(bmap_t *bmap)
{
	ushort_t c_bmap_byte = 0;
	ushort_t c_bit = 0;
	uchar_t c_byte;
	for (ushort_t _y = 0; _y < bmap->map->height; _y++)
		{
			color8b_t c_interval_bg = map_calc_cell_graphics(*map_get_cell(bmap->map, 0, _y)).bg_col;
			for (ushort_t _x = 1; _x < bmap->map->height; _x++)
			{
				if(c_bit == 8)
				{
					set_byte(bmap, _y, c_bmap_byte, c_byte);
					c_bit = 0; 
					c_bmap_byte++;
				}
				color8b_t new_bg = map_calc_cell_graphics(*map_get_cell(bmap->map, _x, _y)).bg_col;

				if(c_interval_bg == new_bg)
					c_byte = set_bit(c_byte, c_bit, 0);
				else
				{
					c_byte = set_bit(c_byte, c_bit, 1);
					c_interval_bg = new_bg;
				}
				c_bit++;
			}
		}
}

void bmap_rebake(ulong_t i, bmap_t *bmap);

void bmap_rebake_at(ushort_t x, ushort_t y, bmap_t *bmap);

void bmap_get_instruction(bmap_t *bmap, ulong_t *i, color8b_t *interval_col, ushort_t *interval_len);

ushort_t bmap_get_interval_at(bmap_t *bmap, ushort_t *x, ushort_t y)
{
	ushort_t interval = 0;
	bmap->flag_bytes[y][(*x / 8)]
	for (ushort_t byte_x = *x / 8; byte_x < bmap->b_w)
	{
		if(bmap->flag_bytes[y][*x])
	}
	ushort_t cell_index = *x + y * map->width;
}
*/
