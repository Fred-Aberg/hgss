#include <string.h>
#include <stdio.h>
#include "../c_codebase/src/raytiles.h"
#include "../c_codebase/src/common.h"
#include "../c_codebase/src/ascui.h"
#include "../map.h"

Sound click_sound;

void dropdown_button(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	container_t *container = *(container_t **)domain;
	container->open = !container->open;
}

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
container_t *cell_info_text;
cell_t *selected_cell = NULL;
char cell_info_buf[CELL_INFO_BUF_LEN];

void cell_to_str(map_t *map, cell_t *c, uint16_t x, uint16_t y)
{
	char env_buf[32];
	map_env_to_str(env_buf, c->env);
	sprintf(cell_info_buf, "[%u, %u]: %s\n  Pop: %u\n  \b002\a070{\b:%u\a \b002\a640|\b:%u\a\n  Realm ID: %u",
	x, y, env_buf, c->pop_lvl, c->baked_food_yield, c->baked_production_yield, c->realm_id);
}


void main_ui()
{
	container_style_t s_0 = style(0, col8bt(0,0,0), col8bt(5,0,2), col8bt(7,7,3), '=', '|', '+');
	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_2 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,0,3), '-', '|', 'O');
	container_style_t s_3 = style(1, col8bt(2,1,1), col8bt(2,1,1), col8bt(7,1,3), '=', '|', '=');
	top_container = ascui_container(true, PERCENTAGE, 100, VERTICAL, 2,
		 ascui_box(true, STATIC, TILES, 20, HORIZONTAL, s_0, 2, 
			ascui_button(true, HOVERABLE, TILES, 3, strlen("Env. Tool Box >"), "Env. Tool Box >", s_1, dropdown_button, &tool_box_container, NULL),
			tool_box_container = ascui_container(true, TILES, 1, HORIZONTAL, 11, 
				river_container = ascui_button(true, SELECTABLE, TILES, 3, strlen("River gen"), "River gen", s_2, NULL, NULL, NULL),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("Plant Woods"), "Plant Woods", s_2, env_tool_button, NULL, (void *)WOODLANDS),
				cut_woods_container = ascui_button(true, SELECTABLE, TILES, 3, strlen("Cut Woods"), "Cut Woods", s_2, NULL, NULL, NULL),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("Sea"), "Sea", s_1, env_tool_button, NULL, (void *)WATER),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("River"), "River", s_1, env_tool_button, NULL, (void *)RIVER),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("Beach"), "Beach", s_1, env_tool_button, NULL, (void *)BEACH),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("Floodplains"), "Floodplains", s_1, env_tool_button, NULL, (void *)FLOODPLAINS),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("Grasslands"), "Grasslands", s_1, env_tool_button, NULL, (void *)GRASSLAND),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("Hills"), "Hills", s_1, env_tool_button, NULL, (void *)HILLS),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("Highlands"), "Highlands", s_1, env_tool_button, NULL, (void *)HIGHLANDS),
				ascui_button(true, SELECTABLE, TILES, 3, strlen("Mountains"), "Mountains", s_1, env_tool_button, NULL, (void *)MOUNTAINS)
				)
		),
		ascui_container(true, TILES, 1, HORIZONTAL, 2,
			ascui_container(true, TILES, 11, VERTICAL, 3,
				ascui_box(true, STATIC, TILES, 12, VERTICAL, s_3, 1, 
					end_turn_button = ascui_button(true, HOVERABLE, TILES, 3, strlen("End Turn"), "End Turn", s_3, end_turn_button_func, NULL, NULL)
				),
				ascui_box(true, STATIC, PERCENTAGE, 33, VERTICAL, s_3, 1, 
					cell_info_text = ascui_text(true, STATIC, TILES, 1, 0, NULL, s_3)
				),
				ascui_box(true, HOVERABLE, TILES, 1, HORIZONTAL, s_3, 4, 
					ascui_button(true, HOVERABLE, TILES, 3, strlen("Default"), "Default", s_3, map_vis_button, NULL, (void *)DEFAULT),
					ascui_button(true, HOVERABLE, TILES, 3, strlen("Terrain"), "Terrain", s_3, map_vis_button, NULL, (void *)TERRAIN),
					ascui_button(true, HOVERABLE, TILES, 3, strlen("Population"), "Population", s_3, map_vis_button, NULL, (void *)POPULATION),
					ascui_button(true, HOVERABLE, TILES, 3, strlen("Heightmap"), "Heightmap", s_3, map_vis_button, NULL, (void *)HEIGHTMAP)
				)
			),
			subgrid_container = ascui_subgrid(true, TILES, 1, NULL)
		)
	);
}
