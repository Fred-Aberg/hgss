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

// pos16(food, prod)
pos16_t map_env_base_yields(environment_e env)
{
	switch (env)
	{
		case WATER:
			return pos16(0, 0);
		case RIVER:
			return pos16(8, 2);
		case MARSH:
			return pos16(2, 2);
		case FLOODPLAINS:
			return pos16(16, 4);
		case BEACH:
			return pos16(8, 2);
		case URBAN:
			return pos16(0, 20);
		case RUINS:
			return pos16(2, 2);
		case GRASSLAND:
			return pos16(12, 4);
		case WOODLANDS:
			return pos16(6, 8);
		case HILLS:
			return pos16(4, 10);
		case WOODLAND_HILLS:
			return pos16(2, 8);
		case HIGHLANDS:
			return pos16(2, 4);
		case HIGHLANDS_WOODS:
			return pos16(0, 6);
		case MOUNTAINS:
			return pos16(0, 1);
		default:
			return pos16(0, 0);
	}
}

uint32_t map_p_to_i(map_t *map, pos16_t p)
{
	return p.x + p.y * map->width;
}


pos16_t map_i_to_p(map_t *map, uint32_t i)
{
	uint16_t y = i / map->width;
	return pos16(i - y * map->width, y);
}

cell_t *map_get_cell_p(map_t *map, pos16_t p)
{
	if(p.x >= map->width && p.y >= map->height)
	{
    fprintf(stderr, "map.c: get cell overflow (x,y)=(%u, %u) [w,h]=[%u, %u]\n", 
    		p.x, p.y, map->width, map->height);
    assert(false); // kys	
	}
    return &map->cells[p.x + p.y * map->width];
}

cell_t *map_get_cell_i(map_t *map, uint32_t i)
{
	if(i > map->width * map->height)
	{
    fprintf(stderr, "map.c: get cell overflow i=%u [w,h]=[%u, %u]\n", 
    		i, map->width, map->height);
    assert(false); // kys	
	}
    return &map->cells[i];
}

map_t *map_create_map(uint16_t width, uint16_t height)
{
	map_t *map = calloc(1, sizeof(map_t));
	map->width = width;
	map->height = height;
	map->cells = calloc(width*height, sizeof(cell_t));
	map->pop_data = calloc(width*height, sizeof(pop_data_t));
	map->b_cell_graphics = calloc(width*height, sizeof(cell_graphics_t));

	// optimize with memset?
	uint32_t n_c = width * height;
	for (uint32_t i = 0; i < n_c; i++)
		map->cells[i].realm_id = NO_REALM_ID;

	return map;
}

// int comp_cells_qs(const void *id_a, const void *id_b)
// {
	// return (int)(*(signed long long *)id_a - *(signed long long *)id_b);
// }
// 
// llong_t comp_cells(uint32_t a, uint32_t b)
// {
	// return (int)((signed long long)a - (signed long long)b);
// }
// 
// #define REALLOC_PERCENTAGE_INCREASE 1.5f

void map_rebake_cell_yields(map_t *map, uint32_t cell_id)
{
	uint16_t pop_multiplier = (map->cells[cell_id].pop_lvl>>2);
	
	pos16_t base_env_yields = map_env_base_yields(map->cells[cell_id].env);
	map->cells[cell_id].baked_food_yield = base_env_yields.x * pop_multiplier;
	map->cells[cell_id].baked_production_yield = base_env_yields.y * pop_multiplier;
}

void map_rebake_cell_gfx(map_t *map, uint32_t cell_id)
{
	map->b_cell_graphics[cell_id] = map_calc_cell_graphics(map->cells[cell_id], 1);
}

void map_rebake_yields(map_t *map)
{
	uint32_t n_cells = map->width * map->height;
	for(uint32_t i = 0; i < n_cells; i++)
	{
		map_rebake_cell_yields(map, i);
	}
}

void map_rebake_graphics(map_t *map)
{
	uint32_t n_cells = map->width * map->height;
	for(uint32_t i = 0; i < n_cells; i++)
	{
		map_rebake_cell_gfx(map, i);
	}
}

void map_rebake_cells(map_t *map)
{
	map_rebake_yields(map);
	map_rebake_graphics(map);
}


#define cg(icon, bg_col, char_col) (cell_graphics_t){bg_col, char_col, icon}

cell_graphics_t map_calc_cell_graphics(cell_t cell, int rand)
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

color8b_t realm_id_to_col(uint16_t id)
{
	id = 8 + id * 37;
	uint8_t id_8bt = ((id>>8) & 255) ^ (id & 255);
	return id_8bt;
}

#define col_hmap(c) tl_Color_to_color8b((Color){min((int)c + 32, 255), min((int)c + 32, 255), 0, 0})
#define col_pop(c) tl_Color_to_color8b((Color){min((int)c + 32, 255), min((int)c * 2 + 32, 255), 0,0})
#define col_char(c) tl_Color_to_color8b((Color){c, c, c, c})
#define p_val(image, x, y) GetImageColor(image, x, y).r

bool border_cell(map_t *map, uint16_t realm_id, uint16_t x, uint16_t y)
{
	if(realm_id == 0)
		return false;
	pos16_t neighbours[4] = {pos16(x, min((int)y + 1, map->height - 1)), pos16(x, max((int)y - 1, 0)), pos16(min((int)x + 1, map->width - 1), y), pos16(max((int)x - 1, 0), y)};

	for (uint8_t i = 0; i < 4; i++)
		if(map_get_cell_p(map, neighbours[i])->realm_id != realm_id)
			return true;
	return false;
}

void draw_map_default(grid_t *grid, map_t *map, uint8_t g_x0, uint8_t g_y0, uint8_t g_x1, uint8_t g_y1, uint16_t map_x0, uint16_t map_y0, char map_font, cell_t *selected_cell,
						color8b_t t_delta_col, color8b_t t_delta_col_invert)
{
	// uint16_t selected_region_id = (selected_cell != NULL)? selected_cell->region_id : 0;
	
	int map_display_width = g_x1 - g_x0;
	int map_display_height = g_y1 - g_y0;

	cell_graphics_t cg;
	cell_t *c_cell;
	const color8b_t WATER_A = col8bt(0, 1, 3);
	int t = GetTime();
	bool rand = t & 1;
	
	for (int _y = 0; _y < map_display_height; _y++)
		{
			color8b_t c_interval_bg;
			uint16_t c_interval_x0 = 0;
			uint16_t c_interval_x1 = 0;
			for (int _x = 0; _x < map_display_width; _x++)
			{
				c_cell = map_get_cell_p(map, pos16(map_x0 + _x, map_y0 + _y));
				
				cg = map->b_cell_graphics[map_p_to_i(map, pos16(map_x0 + _x, map_y0 + _y))];

				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;
				if(c_cell->realm_id != NO_REALM_ID)
					cg.bg_col = realm_id_to_col(c_cell->realm_id);
					
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
					cg = map->b_cell_graphics[map_p_to_i(map, pos16(map_x0 + _x, map_y0 + _y))];
					if(cg.icon == '0' || cg.icon == '2') // Sea or River
						cg.icon += rand;
					
					
					tl_plot_smbl(grid, g_x0 + _x, g_y0 + _y, cg.icon, cg.char_col, map_font);
				}
		}
}

void draw_map_terrain(grid_t *grid, map_t *map, uint8_t g_x0, uint8_t g_y0, uint8_t g_x1, uint8_t g_y1, uint16_t map_x0, uint16_t map_y0, char map_font, cell_t *selected_cell,
						color8b_t t_delta_col, color8b_t t_delta_col_invert)
{
	// uint16_t selected_region_id = (selected_cell != NULL)? selected_cell->region_id : 0;
	
	int map_display_width = g_x1 - g_x0;
	int map_display_height = g_y1 - g_y0;

	cell_graphics_t cg;
	cell_t *c_cell;
	const color8b_t WATER_A = col8bt(0, 1, 3);
	int t = GetTime();
	bool rand = t & 1;
	
	for (int _y = 0; _y < map_display_height; _y++)
		{
			color8b_t c_interval_bg;
			uint16_t c_interval_x0 = 0;
			uint16_t c_interval_x1 = 0;
			for (int _x = 0; _x < map_display_width; _x++)
			{
				// c_cell = map_get_cell_p(map, pos16(map_x0 + _x, map_y0 + _y));
				

				cg = map->b_cell_graphics[map_p_to_i(map, pos16(map_x0 + _x, map_y0 + _y))];
				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;
				// else if(selected_region_id != 0 && selected_region_id == c_cell->region_id)
					// cg.bg_col = t_delta_col;
					
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
					cg = map->b_cell_graphics[map_p_to_i(map, pos16(map_x0 + _x, map_y0 + _y))];
					if(cg.icon == '0' || cg.icon == '2') // Sea or River
						cg.icon += rand;
					tl_plot_smbl(grid, g_x0 + _x, g_y0 + _y, cg.icon, cg.char_col, map_font);
				}
		}
}

// Draw at the same time as rebaking? Looping through the cells only once?
void map_draw_map_onto_grid(grid_t *grid, map_t *map, pos16_t cam_map_pos16, char map_font, map_visualisation_e vis, cell_t *selected_cell)
{
	
	// uint16_t selected_region_id = (selected_cell != NULL)? selected_cell->region_id : 0;

	uint8_t t_char = ((sin(4 * GetTime()) + 1.0f)/2.0f) * 255.0f;
	color8b_t t_delta_col = col_char(t_char);
	color8b_t t_delta_col_invert = col_char(255 - t_char);
	

	pos16_t grid_dimensions = tl_grid_get_dimensions(grid);
	int g_cam_x = (int)grid_dimensions.x / 2;
	int g_cam_y = (int)grid_dimensions.y / 2;


	int g_x0 = max(g_cam_x - cam_map_pos16.x, 0);
	int g_y0 = max(g_cam_y - cam_map_pos16.y, 0);
	int g_x1 = min(g_cam_x + (map->width - cam_map_pos16.x), grid_dimensions.x);
	int g_y1 = min(g_cam_y + (map->height - cam_map_pos16.y), grid_dimensions.y);

	// int cam_up = min(cam_map_pos16.y, g_cam_y);
	// int cam_down = min(map->height - cam_map_pos16.y, g_cam_y);
	// int cam_right = min(map->width - cam_map_pos16.x, g_cam_x);
	// int cam_left = g_cam_x - g_x0;
	
	int map_x0 = cam_map_pos16.x - (g_cam_x - g_x0);
	int map_y0 = cam_map_pos16.y - (g_cam_y - g_y0);

	// int map_x0 = max(0, (int)cam_map_pos16.x - g_cam_x);
	// int map_y0 = max(0, (int)cam_map_pos16.y - g_cam_y);
	// int map_x1 = min(map->width, cam_map_pos16.x + g_cam_x);
	// int map_y1 = min(map->height, cam_map_pos16.y + g_cam_y);

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
		default:
			break;
	}
	

	int map_display_width = g_x1 - g_x0;
	int map_display_height = g_y1 - g_y0;

	int t = GetTime();
	bool rand = t & 1;
	cell_graphics_t cg;
	tl_draw_rect_bg(grid,g_x0, g_y0, g_x1, g_y1, WATER_A);

	cell_t *c_cell;

	for (int _y = 0; _y < map_display_height; _y++)
	{
		color8b_t c_interval_bg;
		uint16_t c_interval_x0 = 0;
		uint16_t c_interval_x1 = 0;
		for (int _x = 0; _x < map_display_width; _x++)
		{
			c_cell = map_get_cell_p(map, pos16(map_x0 + _x, map_y0 + _y));
			
			if(c_cell->env == WATER || c_cell->env == RIVER)
			{
				cg = map->b_cell_graphics[map_p_to_i(map, pos16(map_x0 + _x, map_y0 + _y))];
				if(cg.icon == 'A' || cg.icon == '1') // Sea or River
					cg.icon += rand;
			}
			else if(vis == POPULATION)
			{
				cg = cg('\0', col_pop(c_cell->pop_lvl), BLACK8B);
				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;

			}
			else if(vis == HEIGHTMAP)
			{
				cg = cg('\0', col_hmap(GetImageColor(map->mapg_data.heightmap, map_x0 + _x, map_y0 + _y).r), BLACK8B);
				if(c_cell == selected_cell)
					cg.bg_col = t_delta_col_invert;
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
		for (int _x = 0; _x < map_display_width; _x++)
			if(grid->tile_p_w >= SYMBOL_CULL_P_W)
			{
				cg = map->b_cell_graphics[map_p_to_i(map, pos16(map_x0 + _x, map_y0 + _y))];
				if(cg.icon == '0' || cg.icon == '2') // Sea or River
					cg.icon += rand;
					
				tl_plot_smbl(grid, g_x0 + _x, g_y0 + _y, cg.icon, BLACK8B, map_font);
			}
	}
}

pos16_t map_grid_pos_to_map_pos(grid_t *grid, map_t *map, pos16_t grid_pos16, pos16_t camera_pos16ition)
{
	pos16_t grid_dimensions = tl_grid_get_dimensions(grid);
	int g_cam_x = (int)grid_dimensions.x / 2;
	int g_cam_y = (int)grid_dimensions.y / 2;
	return pos16(clamp(0, (int)camera_pos16ition.x - (g_cam_x - (int)grid_pos16.x), map->width - 1), 
			   clamp(0, (int)camera_pos16ition.y - (g_cam_y - (int)grid_pos16.y), map->height - 1));
}
