#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "map.h"
#include "c_codebase/src/common.h"

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

map_t *map_create_map(uint_t width, uint_t height)
{
	map_t *map = calloc(1, sizeof(map_t));
	map->width = width;
	map->height = height;
	map->cells = calloc(width*height, sizeof(cell_t));
	map->region_count = 0;
	map->region_capacity = 10;
	map->regions = calloc(10, sizeof(region_t));

	// environment_e envs[] = {WATER, RIVER, MARSH,FLOODPLAINS,URBAN,RUINS,GRASSLAND,WOODLANDS,HILLS,WOODLAND_HILLS,HIGHLANDS,HIGHLANDS_WOODS,MOUNTAINS};
	// for (uint_t y = 0; y < width; y++)
		// for (uint_t x = 0; x < height; x++)
		// {
			// if(x * y == 0 || x == width - 1 || y == height - 1)
				// map_get_cell(map, x, y)->env = WATER;
			// else
				// map_get_cell(map, x, y)->env = envs[GetRandomValue(0, 13)];
				// 
		// }

	return map;
}
/*
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
	assert(cell_count != 0);
	if (map->region_capacity <= map->region_count)
	{
		map->region_capacity *= REALLOC_PERCENTAGE_INCREASE + 1; 
		map->regions = (region_t *)realloc(map->regions, (map->region_capacity * sizeof(region_t)));
	}
	
	region_t *region = calloc(1, sizeof(region_t));
	region->cell_count = cell_count;
	region->cell_capacity = cell_count;
	region->cell_ids = calloc(cell_count, sizeof(ulong_t));
	memcpy(region->cell_ids, cell_ids, cell_count * sizeof(ulong_t));
	qsort(region->cell_ids, cell_count, sizeof(ulong_t), comp_cells_qs);
	
	ulong_t first_id = region->cell_ids[0];

	
	llong_t comparison;
	for (ushort_t i = 0; i < map->region_count; i++)
	{
		comparison = comp_cells(first_id, map->regions[i].cell_ids[0]);
		if(comparison > 0)
		{
			// Move all tags from and including i one step forward
			memmove(&map->regions[i + 1], &map->regions[i], (map->region_count - i) * sizeof(Container_tag_t));
			
			list->tags[i].container = container;
			strcpy(list->tags[i].tag, tag);
			break;
		}
	}
	list->count++;
}

void map_remove_region(map_t *map, ushort_t region_id);

void map_rebake_cell_yields(ulong_t cell_id);

void map_add_cell_to_region(ushort_t region_id, ulong_t cell_id);

void map_add_cells_to_region(ushort_t region_id, ulong_t *cell_ids);

void map_remove_cell_from_region(ushort_t region_id, ulong_t cell_id);

void map_remove_cells_from_region(ushort_t region_id, ulong_t *cell_ids);

void map_remove_cells_from_region(ushort_t region_id, ulong_t *cell_id);
*/


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
#define p_val(image, x, y) GetImageColor(image, x, y).r

// Draw at the same time as rebaking? Looping through the cells only once?
void map_draw_map_onto_grid(grid_t *grid, map_t *map, Pos_t cam_map_pos, char map_font, map_visualisation_e vis)
{
	#define SYMBOL_CULL_P_W 8

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

	int map_display_width = g_x1 - g_x0;
	int map_display_height = g_y1 - g_y0;

	const color8b_t WATER_A = col8bt(0, 1, 3);
	
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
			if(c_cell->env == WATER)
				cg = map_calc_cell_graphics(*c_cell);
			else if(vis == DEFAULT)
				cg = map_calc_cell_graphics(*c_cell);
			else if(vis == POPULATION)
				cg = cg('\0', col_pop(c_cell->pop_lvl), BLACK8B);
			else if(vis == HEIGHTMAP)
				cg = cg('\0', col_hmap(GetImageColor(map->heightmap, map_x0 + _x, map_y0 + _y).r), BLACK8B);
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
