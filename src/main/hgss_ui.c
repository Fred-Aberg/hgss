#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../c_codebase/src/raytiles.h"
#include "../c_codebase/src/common.h"
#include "../c_codebase/src/ascui.h"
#include "../map.h"

Sound click_sound;

environment_e selected_env;

void env_tool_button(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	selected_env = (environment_e)function_data;
}

map_visualisation_e map_vis = DEFAULT;

void map_vis_button(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	map_vis = (map_visualisation_e)function_data;
}

						// c_realm_id		player_realm_id
void end_turn_button_func(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed || *(uint16_t *)domain != *(uint16_t *)function_data )
		return;
	*(uint16_t *)domain += 1;
}

void woodify_cell(map_t *map, uint16_t x, uint16_t y)
{
	cell_t *c = map_get_cell_p(map, pos16(x, y));

	switch (c->env)
	{
		case FLOODPLAINS:
			c->env = MARSH;
			break;
		case GRASSLAND:
			c->env = WOODLANDS;
			break;
		case HILLS:
			c->env = WOODLAND_HILLS;
			break;
		case HIGHLANDS:
			c->env = HIGHLANDS_WOODS;
			break;
		default:
		return;
	}
}

void unwoodify_cell(map_t *map, uint16_t x, uint16_t y)
{
	cell_t *c = map_get_cell_p(map, pos16(x, y));

	switch (c->env)
	{
		case MARSH:
			c->env = FLOODPLAINS;
			break;
		case WOODLANDS:
			c->env = GRASSLAND;
			break;
		case WOODLAND_HILLS:
			c->env = HILLS;
			break;
		case HIGHLANDS_WOODS:
			c->env = HIGHLANDS;
			break;
		default:
		return;
	}
}


container_t *top_container;
container_t *subgrid_container;
container_t *tool_box_container;



container_t *river_container;
container_t *cut_woods_container;
container_t *plot_region_container;

container_t *end_turn_button;

#define CELL_INFO_BUF_LEN 256
cell_t *selected_cell = NULL;
char cell_info_buf[CELL_INFO_BUF_LEN];
char *buf_ptr = &cell_info_buf[0];
container_t *cell_display;

void cell_to_str(map_t *map, cell_t *c, uint16_t x, uint16_t y)
{
	char env_buf[32];
	map_env_to_str(env_buf, c->env);
	sprintf(cell_info_buf, "\n\n[%u, %u]: %s\n  Pop: %u\n  \b002\a070{\b:%u\a \b002\a640|\b:%u\a\n  Realm ID: %u",
	x, y, env_buf, c->pop_lvl, c->baked_food_yield, c->baked_production_yield, c->realm_id);
}

void update_main_ui()
{
	ascui_get_display_data(cell_display)->baked_available_width = 0;
	ascui_get_display_data(cell_display)->text_len = strlen(buf_ptr);
}

void main_ui()
{

	container_style_t s_0 = style(0, col8bt(0,0,0), col8bt(5,0,2), col8bt(7,7,3), '=', '|', '+');
	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_2 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,0,3), '-', '|', 'O');
	container_style_t s_3 = style(1, col8bt(2,1,1), col8bt(2,1,1), col8bt(7,1,3), '-', '|', '+');
	container_style_t s_v_div = style(1, col8bt(2,1,1), col8bt(2,1,1), col8bt(7,1,3), '|', '|', '|');
	container_style_t s_h_div = style(1, col8bt(2,1,1), col8bt(2,1,1), col8bt(7,1,3), '=', '=', '=');
	top_container = ascui_container(true, PERCENTAGE, 100, VERTICAL, 2,
		 ascui_box(true, STATIC, TILES, 20, HORIZONTAL, s_0, 2, 
			ascui_dropdown_button(TILES, 3, "V Env. Tool Box ", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1),
			tool_box_container = ascui_container(true, TILES, 1, HORIZONTAL, 11, 
				river_container = ascui_button(true, SELECTABLE, TILES, 3, "River gen", ALIGN_MIDDLE, ALIGN_MIDDLE, s_2, NULL, NULL, NULL),
				ascui_button(true, SELECTABLE, TILES, 3, "Plant Woods", ALIGN_MIDDLE, ALIGN_MIDDLE, s_2, env_tool_button, NULL, (void *)WOODLANDS),
				cut_woods_container = ascui_button(true, SELECTABLE, TILES, 3, "Cut Woods", ALIGN_MIDDLE, ALIGN_MIDDLE, s_2, NULL, NULL, NULL),
				ascui_button(true, SELECTABLE, TILES, 3, "Sea", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, env_tool_button, NULL, (void *)WATER),
				ascui_button(true, SELECTABLE, TILES, 3, "River", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, env_tool_button, NULL, (void *)RIVER),
				ascui_button(true, SELECTABLE, TILES, 3, "Beach", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, env_tool_button, NULL, (void *)BEACH),
				ascui_button(true, SELECTABLE, TILES, 3, "Floodplains", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, env_tool_button, NULL, (void *)FLOODPLAINS),
				ascui_button(true, SELECTABLE, TILES, 3, "Grasslands", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, env_tool_button, NULL, (void *)GRASSLAND),
				ascui_button(true, SELECTABLE, TILES, 3, "Hills", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, env_tool_button, NULL, (void *)HILLS),
				ascui_button(true, SELECTABLE, TILES, 3, "Highlands", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, env_tool_button, NULL, (void *)HIGHLANDS),
				ascui_button(true, SELECTABLE, TILES, 3, "Mountains", ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, env_tool_button, NULL, (void *)MOUNTAINS)
			)
		),
		ascui_container(true, TILES, 1, HORIZONTAL, 5,
			ascui_divider(s_h_div),
			ascui_divider(s_h_div),
			ascui_container(true, TILES, 11, VERTICAL, 2,
				ascui_container(true, TILES, 38, VERTICAL, 4,
					end_turn_button = ascui_button(true, HOVERABLE, TILES, 10, "End Turn", ALIGN_MIDDLE, ALIGN_MIDDLE, s_3, end_turn_button_func, NULL, NULL),
					ascui_divider(s_v_div),
					cell_display = ascui_display(true, STATIC, TILES, 26, &buf_ptr, ALIGN_LEFT, ALIGN_TOP, s_3),
					ascui_divider(s_v_div)
				),
				ascui_box(true, HOVERABLE, TILES, 1, VERTICAL, s_3, 7, 
					ascui_button(true, HOVERABLE, PERCENTAGE, 25, "Default", ALIGN_MIDDLE, ALIGN_MIDDLE, s_3, map_vis_button, NULL, (void *)DEFAULT),
					ascui_divider(s_v_div),
					ascui_button(true, HOVERABLE, PERCENTAGE, 25, "Terrain", ALIGN_MIDDLE, ALIGN_MIDDLE, s_3, map_vis_button, NULL, (void *)TERRAIN),
					ascui_divider(s_v_div),
					ascui_button(true, HOVERABLE, PERCENTAGE, 25, "Population", ALIGN_MIDDLE, ALIGN_MIDDLE, s_3, map_vis_button, NULL, (void *)POPULATION),
					ascui_divider(s_v_div),
					ascui_button(true, HOVERABLE, PERCENTAGE, 25, "Heightmap", ALIGN_MIDDLE, ALIGN_MIDDLE, s_3, map_vis_button, NULL, (void *)HEIGHTMAP)
				)
			),
			ascui_divider(s_h_div),
			subgrid_container = ascui_subgrid(true, TILES, 1, NULL)
		)
	);
}
