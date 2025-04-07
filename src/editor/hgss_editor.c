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

#include "editor_ui_main_menu.c"
#include "editor_ui_new_world.c"
#include "editor_ui_load_world.c"
// #include "editor_ui_editor.c"

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

   	int screensize_x = 1250;
	int screensize_y = 650;
	uint16_t tile_width = 12;

    InitWindow(screensize_x, screensize_y, "HGSS");
    
    InitAudioDevice();
	Sound click_sound = LoadSound("Resources/Audio/click.wav");
	Sound scroll_sound = LoadSound("Resources/Audio/scroll.wav");
	Sound paint_sound = LoadSound("Resources/Audio/paint.wav");
    
    Font unscii = LoadFontEx("Resources/Fonts/unscii-8-alt.ttf", 32, 0, 256);
    Font unscii_ortho = LoadFontEx("Resources/Fonts/unscii8x8orthov_v4.ttf", 64, 0, 90);
    Font unscii_fantasy = LoadFontEx("Resources/Fonts/unscii-8-fantasy.ttf", 32, 0, 256);
    Font icons_font = LoadFontEx("Resources/Fonts/hgss.ttf", 64, NULL, 0);

	Font fonts[4] = {unscii, unscii_fantasy, icons_font, unscii_ortho};
    
    SetTargetFPS(60);
    SetWindowMinSize(200, 200);

	// Main Grid
	grid_t *main_grid = tl_init_grid(0, 0, screensize_x, screensize_y, tile_width, 1.0f, fonts, 1500);
	grid_t *subgrid = tl_init_grid(0, 0, 100, 100, 40, 1.0f, fonts, 3000);
	
	// UI Bindings
	bind_main_menu_ui();
	bind_new_world_menu_ui();
	bind_load_world_menu_ui();
	cursor_t cursor; 

	// ascui_print_ui(editor_cntr);
	ascui_print_ui(main_menu_cntr);

	
    while (!WindowShouldClose())
    {
    	tick++;
		frame_time = -GetTime();

        BeginDrawing();
        ClearBackground(BLACK);

    	ascui_drawing_time = 0;

		switch (editor_state)
		{
			case MAIN_MENU: 
				ascui_run_ui(main_grid, main_menu_cntr, &ascui_drawing_time, &click_sound, &scroll_sound, 45, 47, &cursor);
			break;
			case NEW_WORLD:
				ascui_run_ui(main_grid, new_world_menu_cntr, &ascui_drawing_time, &click_sound, &scroll_sound, 45, 47, &cursor);
			break;
			case LOAD_WORLD:
				ascui_run_ui(main_grid, load_world_menu_cntr, &ascui_drawing_time, &click_sound, &scroll_sound, 45, 47, &cursor);
			break;
			case EDITOR:
			break;
			default:
			break;
		}
		
		//-------------------

		tl_plot_smbl_w_bg(main_grid, cursor.x, cursor.y, '^', BLACK8B, WHITE8B, 0);
		
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
			sprintf(buf, "(%u, %u)", cursor.x, cursor.y);
			DrawText(buf, 0, 0, 24, c(0, 255, 0));

			DrawFPS(0, 100);
		}

        EndDrawing();
    }
    UnloadFont(unscii);
    UnloadFont(unscii_ortho);
    UnloadFont(unscii_fantasy);
    UnloadFont(icons_font);
    CloseAudioDevice();
    CloseWindow();
    // ascui_destroy(editor_cntr);
    ascui_destroy(main_menu_cntr);
	tl_deinit_grid(main_grid);
    return 0;
}
