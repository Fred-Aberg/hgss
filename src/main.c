#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "c_codebase/src/raytiles.h"
#include "c_codebase/src/common.h"
#include "c_codebase/src/ascui.h"
#include "map.h"
#include "mapgen.h"

#define DEF_COLOR (Color){20, 40, 29, 255}

void printbincharpad(char c)
{
    for (int i = 7; i >= 0; --i)
    {
        putchar( (c & (1 << i)) ? '1' : '0' );
    }
    putchar('\n');
}

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


void woodify_cell(map_t *map, ushort_t x, ushort_t y)
{
	cell_t *c = map_get_cell(map, x, y);

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

void unwoodify_cell(map_t *map, ushort_t x, ushort_t y)
{
	cell_t *c = map_get_cell(map, x, y);

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

#define CELL_INFO_BUF_LEN 256
container_t *cell_info_text;
cell_t *selected_cell = NULL;
char cell_info_buf[CELL_INFO_BUF_LEN];

void cell_to_str(map_t *map, cell_t *c, ushort_t x, ushort_t y)
{
	char env_buf[32];
	map_env_to_str(env_buf, c->env);
	sprintf(cell_info_buf, "[%u, %u]: %s\n  Pop: %u\n  F:%u P:%u\n  Region ID: %u\n    Pop: %u\n    F:%d P:%d G:%d\n    No. Cells: %u", 
	x, y, env_buf, c->pop_lvl, c->baked_food_yield, c->baked_production_yield, 
	c->region_id, map_get_region_pop(map, c->region_id), map->regions[c->region_id].baked_food_yield, map->regions[c->region_id].baked_production_yield,
	map->regions[c->region_id].baked_gold_yield, map->regions[c->region_id].cell_count);
}


void main_ui()
{
	container_style_t s_0 = style(0, col8bt(0,0,0), col8bt(5,0,2), col8bt(7,7,3), '=', '|', '+');
	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_2 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,0,3), '-', '|', 'O');
	container_style_t s_3 = style(1, col8bt(2,1,1), col8bt(2,1,1), col8bt(7,1,3), '=', '|', '=');
	top_container = ascui_container(true, PERCENTAGE, 100, VERTICAL, 2,
		 ascui_box(true, TILES, 20, HORIZONTAL, s_0, 2, 
			ascui_button(true, TILES, 3, strlen("Env. Tool Box >"), "Env. Tool Box >", s_1, dropdown_button, &tool_box_container, NULL),
			tool_box_container = ascui_container(true, TILES, 1, HORIZONTAL, 12, 
				river_container = ascui_button(true, TILES, 3, strlen("River gen"), "River gen", s_2, NULL, NULL, NULL),
				ascui_button(true, TILES, 3, strlen("Plant Woods"), "Plant Woods", s_2, env_tool_button, NULL, (void *)WOODLANDS),
				cut_woods_container = ascui_button(true, TILES, 3, strlen("Cut Woods"), "Cut Woods", s_2, NULL, NULL, NULL),
				plot_region_container = ascui_button(true, TILES, 3, strlen("Plot Region"), "Plot Region", s_2, NULL, NULL, NULL),
				ascui_button(true, TILES, 3, strlen("Sea"), "Sea", s_1, env_tool_button, NULL, (void *)WATER),
				ascui_button(true, TILES, 3, strlen("River"), "River", s_1, env_tool_button, NULL, (void *)RIVER),
				ascui_button(true, TILES, 3, strlen("Beach"), "Beach", s_1, env_tool_button, NULL, (void *)BEACH),
				ascui_button(true, TILES, 3, strlen("Floodplains"), "Floodplains", s_1, env_tool_button, NULL, (void *)FLOODPLAINS),
				ascui_button(true, TILES, 3, strlen("Grasslands"), "Grasslands", s_1, env_tool_button, NULL, (void *)GRASSLAND),
				ascui_button(true, TILES, 3, strlen("Hills"), "Hills", s_1, env_tool_button, NULL, (void *)HILLS),
				ascui_button(true, TILES, 3, strlen("Highlands"), "Highlands", s_1, env_tool_button, NULL, (void *)HIGHLANDS),
				ascui_button(true, TILES, 3, strlen("Mountains"), "Mountains", s_1, env_tool_button, NULL, (void *)MOUNTAINS)
				)
		),
		ascui_container(true, TILES, 1, HORIZONTAL, 2,
			ascui_container(true, TILES, 11, VERTICAL, 2,
				ascui_box(true, PERCENTAGE, 50, VERTICAL, s_3, 1, 
					cell_info_text = ascui_text(true, TILES, 1, 0, NULL, s_3)
				),
				ascui_box(true, TILES, 1, HORIZONTAL, s_3, 5, 
					ascui_button(true, TILES, 3, strlen("Default"), "Default", s_3, map_vis_button, NULL, (void *)DEFAULT),
					ascui_button(true, TILES, 3, strlen("Terrain"), "Terrain", s_3, map_vis_button, NULL, (void *)TERRAIN),
					ascui_button(true, TILES, 3, strlen("Population"), "Population", s_3, map_vis_button, NULL, (void *)POPULATION),
					ascui_button(true, TILES, 3, strlen("Heightmap"), "Heightmap", s_3, map_vis_button, NULL, (void *)HEIGHTMAP),
					ascui_button(true, TILES, 3, strlen("Regions"), "Regions", s_3, map_vis_button, NULL, (void *)REGIONS)
				)
			),
			subgrid_container = ascui_subgrid(true, TILES, 1, NULL)
		)
	);
}

int main(){
	double tl_rendering_time;
	double total_tl_rendering_time = 0;
	double ascui_drawing_time;
	double frame_time;
	double total_ascui_drawing_time = 0;	
	double map_drawing_time;
	double total_map_drawing_time = 0;
	double total_frame_time = 0;
	bool show_diagnostics = false;
	long n_smbl_draw_calls = 0;
	long n_bg_draw_calls = 0;
	long tick = 0;

   	int screensize_x = 0;
	int screensize_y = 0;
	uint_t tile_width = 0;
	char input[50];
 
   	printf("Screen width:\n");
   	fgets(input, sizeof(input), stdin);
   	screensize_x = atoi(input);

   	if (screensize_x == 0)
   	{
   		// Defaults
   		screensize_x = 1500;
   		screensize_y = 1000;
   		tile_width = 15;
   	}
   	else
   	{
	   	printf("Screen height:\n");
	   	fgets(input, sizeof(input), stdin);
	   	screensize_y = atoi(input);

	   	printf("Tile width:\n");
	   	fgets(input, sizeof(input), stdin);
	   	tile_width = atoi(input);

		if (screensize_x == 0 || screensize_y == 0 || tile_width == 0)
		{
			perror("Invalid inputs");
			return 0;
		}	
   	}
	


    InitWindow(screensize_x, screensize_y, "HGSS");
    Font unscii = LoadFontEx("Resources/Fonts/unscii-8-alt.ttf", 32, 0, 256);
    Font unscii_fantasy = LoadFontEx("Resources/Fonts/unscii-8-fantasy.ttf", 32, 0, 256);
    Font icons_font = LoadFontEx("Resources/Fonts/hgss.ttf", 64, NULL, 0);

	Font fonts[3] = {unscii, unscii_fantasy, icons_font};
    
    SetTargetFPS(60);
    // HideCursor();
    SetWindowMinSize(200, 200);

	// Main Grid
	grid_t *main_grid = tl_init_grid(0, 0, screensize_x, screensize_y, tile_width, 1.0f, fonts, 1500);
	grid_t *subgrid = tl_init_grid(0, 0, 100, 100, 40, 1.0f, fonts, 3000);
	// UI
	main_ui();
	ascui_get_subgrid_data(subgrid_container)->subgrid = subgrid;
	ascui_get_text_data(cell_info_text)->text = &cell_info_buf[0];
	ascui_get_text_data(cell_info_text)->text_len = CELL_INFO_BUF_LEN;

	// map_t *map = map_create_map(500,500);
	// map->mapg_data.seed = 0;
	// map->mapg_data.sea_level = 127;
	// mapgen_assign_heightmap(map, 10, 10);
	
	map_t *map = mapgen_gen_from_heightmap(LoadImage("Resources/Heightmaps/aaland.png"), 15, 0);
	mapgen_place_rivers(map);
	mapgen_place_rivers(map);
	mapgen_place_rivers(map);
	mapgen_clear_region_ids(map);
	mapgen_place_regions(map, 255, 100, 40);
	
	mapgen_populate_map(map, 0.20f, 0.75f, 20.0f);
	// map_t *map = mapgen_gen_continent(400, 255, 80, 4380343);

	Image hmap_img = ImageCopy(map->mapg_data.heightmap);
	Texture2D hmap_tex = LoadTextureFromImage(hmap_img);
	
	Pos_t map_view_camera = pos(map->width/2,map->height/2);
	Pos_t map_view_composite;
	
	cursor_t cursor; 

	ascui_print_ui(top_container);

	Pos_t mouse_delta_start;
	int subg_mouse_delta_x;
	int subg_mouse_delta_y;
	
    while (!WindowShouldClose()){
		Pos_t mouse_scr_pos = pos(GetMouseX(), GetMouseY());

    	tick++;
		frame_time = -GetTime();

        BeginDrawing();
        ClearBackground(BLACK);

		if(IsKeyDown(45))
		{
			uint_t new_tile_size = main_grid->tile_p_w + 1;
			tl_resize_grid(main_grid, 0, 0, screensize_x, screensize_y, new_tile_size);
			tl_center_grid_on_screen(main_grid, screensize_x, screensize_y);
		}
		else if(IsKeyDown(47))
		{
			uint_t new_tile_size = main_grid->tile_p_w - 1;
			tl_resize_grid(main_grid, 0, 0, screensize_x, screensize_y, new_tile_size);
			tl_center_grid_on_screen(main_grid, screensize_x, screensize_y);
		}
		

		if(IsWindowResized())
		{
			screensize_x = GetScreenWidth();
			screensize_y = GetScreenHeight();
			
			tl_resize_grid(main_grid, 0, 0, screensize_x, screensize_y, main_grid->tile_p_w);
			tl_center_grid_on_screen(main_grid, screensize_x, screensize_y);
		}
		
		// Main grid
		Pos_t mouse_grid_pos = tl_screen_to_grid_coords(main_grid, mouse_scr_pos);
		cursor.x = mouse_grid_pos.x;
		cursor.y = mouse_grid_pos.y;
		cursor.right_button_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
		cursor.left_button_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
		cursor.middle_button_pressed = IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
		cursor.scroll = GetMouseWheelMove();

		ascui_drawing_time = -GetTime();
		ascui_draw_ui(main_grid, top_container, &cursor);
		ascui_drawing_time += GetTime();

		// Container hovering and selecting

		if(cursor.hovered_container != NULL)
		{
			if (cursor.hovered_container->container_type == BUTTON)
			{
				button_data_t *bt_data = ascui_get_button_data(cursor.hovered_container);
				if(bt_data->side_effect_func)
					bt_data->side_effect_func(bt_data->domain, bt_data->function_data, &cursor);
			}
			else if (!(cursor.scroll > 0 && cursor.hovered_container->scroll_offset == 0))
				cursor.hovered_container->scroll_offset -= cursor.scroll;

			// Select
			if(cursor.hovered_container->container_type == BUTTON && cursor.left_button_pressed)
				cursor.selected_container = cursor.hovered_container;
			// Deselect
			if(cursor.hovered_container == cursor.selected_container && cursor.right_button_pressed)
				cursor.selected_container = NULL;
		}

		Pos_t mouse_subgrid_pos;
		Pos_t mouse_map_pos;
		// Vector2 mouse_delta = GetMouseDelta();
		if(cursor.hovered_container == subgrid_container)
		{
			int key_cam_delta_x = 0;
			int key_cam_delta_y = 0;
			if (IsKeyDown(KEY_RIGHT)) key_cam_delta_x = 1;
	        if (IsKeyDown(KEY_LEFT)) key_cam_delta_x = -1;
	        if (IsKeyDown(KEY_UP)) key_cam_delta_y = -1;
	        if (IsKeyDown(KEY_DOWN)) key_cam_delta_y = 1;

			map_view_camera.x = clamp(0, (int)map_view_camera.x + key_cam_delta_x, map->width - 1);
			map_view_camera.y = clamp(0, (int)map_view_camera.y + key_cam_delta_y, map->height - 1);
			
			// Pos_t subgrid_dims = tl_grid_get_dimensions(subgrid);
			mouse_subgrid_pos = tl_screen_to_grid_coords(subgrid, mouse_scr_pos);

			// map view camera navigation
			if(IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
				mouse_delta_start = mouse_subgrid_pos;
			else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
			{
				subg_mouse_delta_x = (int)mouse_delta_start.x - (int)mouse_subgrid_pos.x;
				subg_mouse_delta_y = (int)mouse_delta_start.y - (int)mouse_subgrid_pos.y;
				map_view_composite.x = clamp(0, (int)map_view_camera.x + subg_mouse_delta_x, map->width - 1);
				map_view_composite.y = clamp(0, (int)map_view_camera.y + subg_mouse_delta_y, map->height - 1);
			}
			else if(IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE))
			{
				map_view_composite.x = clamp(0, (int)map_view_composite.x, map->width - 1);
				map_view_composite.y = clamp(0, (int)map_view_composite.y, map->height - 1);
				map_view_camera = map_view_composite;
			}
			else
			{
				map_view_composite = map_view_camera;
			}
			// Map drawing
			map_drawing_time = -GetTime();
			map_draw_map_onto_grid(subgrid, map, map_view_composite, 2, map_vis, selected_cell);
			map_drawing_time += GetTime();

			// Painting
			mouse_map_pos = map_grid_pos_to_map_pos(subgrid, map, mouse_subgrid_pos, map_view_camera);
			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				if(selected_cell != NULL)
				{
					selected_cell = NULL;
					cell_info_buf[0] = 0;
				}
				if(cursor.selected_container == river_container)
					mapgen_plot_river(map, mouse_map_pos.x, mouse_map_pos.y);
				else if(cursor.selected_container == plot_region_container)
					mapgen_plot_region(map, mouse_map_pos.x, mouse_map_pos.y, map->region_count + 1, 255);
			}
			else if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			{
				if(cursor.selected_container == cut_woods_container)
					unwoodify_cell(map, mouse_map_pos.x, mouse_map_pos.y);
				else if(selected_env == WOODLANDS)
					woodify_cell(map, mouse_map_pos.x, mouse_map_pos.y);
				else if(selected_env != NONE)
					map_get_cell(map, mouse_map_pos.x, mouse_map_pos.y)->env = selected_env;
			}
			else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			{
				if(selected_env == NONE) // one click to disable painting, second to select cell
				{
					selected_cell = map_get_cell(map, mouse_map_pos.x, mouse_map_pos.y);
					cell_to_str(map, selected_cell, mouse_map_pos.x, mouse_map_pos.y);
				}
				else 
				{
					selected_cell = NULL;
					cell_info_buf[0] = 0;
					selected_env = NONE;
					cursor.selected_container = NULL;
				}
			}


			tl_plot_smbl_w_bg(subgrid, mouse_subgrid_pos.x, mouse_subgrid_pos.y, 'A', WHITE8B, BLACK8B, 1);
		}
		else
		{
			map_drawing_time = -GetTime();
			map_draw_map_onto_grid(subgrid, map, map_view_camera, 2, map_vis, selected_cell);
			map_drawing_time += GetTime();
			tl_plot_smbl_w_bg(main_grid, mouse_grid_pos.x, mouse_grid_pos.y, 'A', WHITE8B, BLACK8B, 0);
		}

		/// HGSS SIMULATION ///

		map_rebake_yields(map);

		//-------------------
		
		tl_rendering_time = -GetTime();
		Pos_t main_grid_dcalls = tl_render_grid(main_grid);
		Pos_t sub_grid_dcalls = tl_render_grid(subgrid);
		tl_rendering_time += GetTime();

		frame_time += GetTime();

		n_bg_draw_calls = main_grid_dcalls.x + sub_grid_dcalls.x;
		n_smbl_draw_calls = main_grid_dcalls.y + sub_grid_dcalls.y;

		total_frame_time += frame_time;
		total_ascui_drawing_time += ascui_drawing_time;
		total_map_drawing_time += map_drawing_time;
		total_tl_rendering_time += tl_rendering_time;


		char buf[100];
		if(IsKeyPressed(KEY_TAB))
			show_diagnostics = !show_diagnostics;

		if(IsKeyDown(KEY_H))
		{
			DrawTexture(hmap_tex, 50, 50, WHITE);
		}
		
		if (show_diagnostics)
		{
			DrawRectangle(0,0, 350, 700, c(40, 10, 40));
			sprintf(buf, "\n\tN draw calls: %li\n\n\t = [bg:%li + smbl:%li]", n_bg_draw_calls + n_smbl_draw_calls, n_bg_draw_calls, n_smbl_draw_calls);
			DrawText(buf, 0, 0, 24, c(255, 0, 255));
			sprintf(buf, "\n\tframe time [ms]:\n\n\t\t%f\n\n\t\tavg: %f", frame_time * 1000.0f, (total_frame_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 75, 24, c(255, 0, 255));
			sprintf(buf, "\n\tascui drawing time [ms] [%d\%]:\n\n\t\t%f\n\n\t\tavg: %f",(int)((total_ascui_drawing_time/total_frame_time) * 100.0f), ascui_drawing_time * 1000.0f, (total_ascui_drawing_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 200, 24, c(255, 0, 255));
			sprintf(buf, "\n\tmap drawing time [ms] [%d\%]:\n\n\t\t%f\n\n\t\tavg: %f",(int)((total_map_drawing_time/total_frame_time) * 100.0f), map_drawing_time * 1000.0f, (total_map_drawing_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 350, 24, c(255, 0, 255));
			sprintf(buf, "\n\ttl drawing time [ms] [%d\%]:\n\n\t\t%f\n\n\t\tavg: %f",(int)((total_tl_rendering_time/total_frame_time) * 100.0f), tl_rendering_time * 1000.0f, (total_tl_rendering_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 500, 24, c(255, 0, 255));
		}
		else
		{
			sprintf(buf, "(%u, %u)", mouse_grid_pos.x, mouse_grid_pos.y);
			DrawText(buf, 0, 0, 24, c(0, 255, 0));

			sprintf(buf, "(%u, %u)", mouse_subgrid_pos.x, mouse_subgrid_pos.y);
			DrawText(buf, 0, 48, 24, c(0, 255, 0));

			sprintf(buf, "(%u, %u)", mouse_map_pos.x, mouse_map_pos.y);
			DrawText(buf, 0, 72, 24, c(0, 255, 0));

			DrawFPS(0, 100);
		}

        EndDrawing();
    }
    UnloadFont(unscii);
    UnloadFont(unscii_fantasy);
    UnloadFont(icons_font);
    CloseWindow();
    // ascui_destroy(top_container);
	tl_deinit_grid(main_grid);
	UnloadTexture(hmap_tex);
    return 0;
}
