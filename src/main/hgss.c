#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "../c_codebase/src/raytiles.h"
#include "../c_codebase/src/common.h"
#include "../c_codebase/src/ascui.h"
#include "../map.h"
#include "../mapgen.h"
#include "../world.h"

#include "hgss_ui.c"

#define DEF_COLOR (Color){20, 40, 29, 255}


void printbincharpad(char c)
{
    for (int i = 7; i >= 0; --i)
    {
        putchar( (c & (1 << i)) ? '1' : '0' );
    }
    putchar('\n');
}


typedef struct
{
	world_t *w;
	uint16_t player_realm_id;
	uint16_t c_realm_id;
	uint8_t game_state;
}game_t;

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
	uint16_t tile_width = 0;
	char input[50];
 
   	printf("Screen width:\n");
   	fgets(input, sizeof(input), stdin);
   	screensize_x = atoi(input);

   	if (screensize_x == 0)
   	{
   		// Defaults
   		screensize_x = 1250;
   		screensize_y = 650;
   		tile_width = 12;
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
    InitAudioDevice();
	Sound click_sound = LoadSound("Resources/Audio/click.wav");
	Sound scroll_sound = LoadSound("Resources/Audio/scroll.wav");
	Sound paint_sound = LoadSound("Resources/Audio/paint.wav");
    
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

	// map_t *map = map_create_map(100,100);
	// map->mapg_data.seed = 0;
	// map->mapg_data.sea_level = 127;
	// mapgen_assign_heightmap(map, 30, 30);
	
	map_t *map = mapgen_gen_from_heightmap(LoadImage("Resources/Heightmaps/aaland.png"), 15, 0);
	mapgen_place_rivers(map);
	mapgen_place_rivers(map);
	mapgen_place_rivers(map);
	
	mapgen_populate_map(map, 0.20f, 0.75f, 20.0f);

	map_rebake_cells(map);
	
	// map_t *map = mapgen_gen_continent(400, 255, 80, 4380343);

	Image hmap_img = ImageCopy(map->mapg_data.heightmap);
	Texture2D hmap_tex = LoadTextureFromImage(hmap_img);
	
	pos16_t map_view_camera = pos16(map->width/2,map->height/2);
	pos16_t map_view_composite;
	
	cursor_t cursor; 

	ascui_print_ui(top_container);

	pos16_t mouse_delta_start;
	int subg_mouse_delta_x;
	int subg_mouse_delta_y;

	game_t game = {.w = w_create(0, map), 0,0,0};
	w_disperse_realms(game.w, 50);
	
	ascui_get_button_data(end_turn_button)->domain = &game.c_realm_id; // Bind end turn button to c_realm_id
	ascui_get_button_data(end_turn_button)->function_data = &game.player_realm_id; // give player ID to confirm that c_realm_id++; is allowed
	
    while (!WindowShouldClose())
    {
		pos16_t mouse_scr_pos = pos16(GetMouseX(), GetMouseY());
		pos16_t mouse_subgrid_pos;
		pos16_t mouse_map_pos;
		
    	tick++;
		frame_time = -GetTime();

        BeginDrawing();
        ClearBackground(BLACK);

		if(IsKeyDown(45))
		{
			uint16_t new_tile_size = main_grid->tile_p_w + 1;
			tl_resize_grid(main_grid, 0, 0, screensize_x, screensize_y, new_tile_size);
			tl_center_grid_on_screen(main_grid, screensize_x, screensize_y);
		}
		else if(IsKeyDown(47))
		{
			uint16_t new_tile_size = main_grid->tile_p_w - 1;
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
		
		/// UI INTERACTIONS: MAIN UI	-----------------------------------------------------
		pos16_t mouse_grid_pos = tl_screen_to_grid_coords(main_grid, mouse_scr_pos);
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
			if(cursor.left_button_pressed && (cursor.hovered_container->container_type == BUTTON || cursor.hovered_container->selectability == SELECTABLE))
			{
				PlaySound(click_sound);
			}
			
			if (!(cursor.scroll > 0 && cursor.hovered_container->scroll_offset == 0) && cursor.scroll != 0)
			{
				cursor.hovered_container->scroll_offset -= cursor.scroll;
				if (!IsSoundPlaying(scroll_sound))
					PlaySound(scroll_sound);
			}
				
			if (cursor.hovered_container->container_type == BUTTON)
			{
				button_data_t *bt_data = ascui_get_button_data(cursor.hovered_container);
				if(bt_data->side_effect_func)
					bt_data->side_effect_func(bt_data->domain, bt_data->function_data, &cursor);
			}

			// Select
			if(cursor.left_button_pressed && cursor.hovered_container->selectability == SELECTABLE)
			{
				cursor.selected_container = cursor.hovered_container;
				
			}
			// Deselect
			if(cursor.hovered_container == cursor.selected_container && cursor.right_button_pressed)
				cursor.selected_container = NULL;
		}

		/// TURN HANDLING	----------------------------------------------------------------

		if(game.w->realm_count == 0)
			goto skip_turn_handling;
			
		    
    	if (game.c_realm_id >= game.w->realm_count) { game.c_realm_id = 0; }
    		
		printf("\n Realm turn: %u / %u", game.c_realm_id, game.w->realm_count);
		
		if(!game.w->realms[game.c_realm_id].is_active)
			continue;
			
		if(game.c_realm_id != game.player_realm_id)
		{
			if(make_ai_move(game.c_realm_id))
				game.c_realm_id++;
		}
		else if(cursor.hovered_container == subgrid_container) // Player's turn and map-view hovered
		{
			// handle player moves
			
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
				{
					mapgen_plot_river(map, mouse_map_pos.x, mouse_map_pos.y);
					if (!IsSoundPlaying(paint_sound))
						PlaySound(paint_sound);
				}
			}
			else if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			{
				if (!IsSoundPlaying(paint_sound))
					PlaySound(paint_sound);
				if(cursor.selected_container == cut_woods_container)
					unwoodify_cell(map, mouse_map_pos.x, mouse_map_pos.y);
				else if(selected_env == WOODLANDS)
					woodify_cell(map, mouse_map_pos.x, mouse_map_pos.y);
				else if(selected_env != NONE)
					map_get_cell_p(map, pos16(mouse_map_pos.x, mouse_map_pos.y))->env = selected_env;
			}
			else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			{
				if(selected_env == NONE) // one click to disable painting, second to select cell
				{
					if (!IsSoundPlaying(click_sound))
						PlaySound(click_sound);
					selected_cell = map_get_cell_p(map, pos16(mouse_map_pos.x, mouse_map_pos.y));
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
		}

		skip_turn_handling:
    	
		/// UI INTERACTIONS: MAP-VIEW	----------------------------------------------------

		if(cursor.hovered_container == subgrid_container) // Not neccessarily the players turn, but map-view is hovered
		{
			int key_cam_delta_x = 0;
			int key_cam_delta_y = 0;
			float delta_mult = max(1, 30/subgrid->tile_p_w); //ms
			if (IsKeyDown(KEY_RIGHT)) key_cam_delta_x = 1.0f * delta_mult;
	        if (IsKeyDown(KEY_LEFT)) key_cam_delta_x = -1.0f * delta_mult;
	        if (IsKeyDown(KEY_UP)) key_cam_delta_y = -1.0f * delta_mult;
	        if (IsKeyDown(KEY_DOWN)) key_cam_delta_y = 1.0f * delta_mult;

			map_view_camera.x = clamp(0, (int)map_view_camera.x + key_cam_delta_x, map->width - 1);
			map_view_camera.y = clamp(0, (int)map_view_camera.y + key_cam_delta_y, map->height - 1);
			
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
				subg_mouse_delta_x = 0;
				subg_mouse_delta_y = 0;
			}
			else
			{
				map_view_composite = map_view_camera;
			}
			// Map drawing
			map_drawing_time = -GetTime();
			map_draw_map_onto_grid(subgrid, map, map_view_composite, 2, map_vis, selected_cell);
			map_drawing_time += GetTime();


			tl_plot_smbl_w_bg(subgrid, mouse_subgrid_pos.x, mouse_subgrid_pos.y, 'A', WHITE8B, BLACK8B, 1);
		}
		else // Not neccessarily player's turn, map-view not hovered 
		{
			if(subg_mouse_delta_x != 0 || subg_mouse_delta_y != 0)
			{
				map_view_composite.x = clamp(0, (int)map_view_composite.x, map->width - 1);
				map_view_composite.y = clamp(0, (int)map_view_composite.y, map->height - 1);
				map_view_camera = map_view_composite;
				subg_mouse_delta_x = 0;
				subg_mouse_delta_y = 0;
			}
		
			map_drawing_time = -GetTime();
			map_draw_map_onto_grid(subgrid, map, map_view_camera, 2, map_vis, selected_cell);
			map_drawing_time += GetTime();
			tl_plot_smbl_w_bg(main_grid, mouse_grid_pos.x, mouse_grid_pos.y, 'A', WHITE8B, BLACK8B, 0);
		}

		/// HGSS SIMULATION ///

		map_rebake_cells(map);

		//-------------------
		
		tl_rendering_time = -GetTime();
		pos16_t main_grid_dcalls = tl_render_grid(main_grid);
		pos16_t sub_grid_dcalls = tl_render_grid(subgrid);
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
    CloseAudioDevice();
    CloseWindow();
    ascui_destroy(top_container);
	tl_deinit_grid(main_grid);
	UnloadTexture(hmap_tex);
    return 0;
}
