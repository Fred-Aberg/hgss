#pragma once
#include "raylib.h"
#include "stdlib.h"
#include "common.h"

#define NO_BG (Color) {0,0,0,0}
#define NO_SMBL 0

int tl_init_grid(uint res_x, int scr_size_x, int scr_size_y, Color def_col, Font *def_font);

void tl_deinit_grid();

Pos_t tl_grid_get_size();

void tl_render_grid();

void tl_draw_tile(uint x, uint y, char smbl, Color char_col, Color bg_col, Font *font);

void tl_draw_rect(uint x, uint y, uint width, uint height, char smbl, Color char_col, Color bg_col, Font *font);

void tl_draw_line(uint x_start, uint y_start, uint x_end, uint y_end, char smbl, Color char_col, Color bg_col, Font *font);

uint tl_draw_text(uint x, uint y, uint wrap, char *text, uint len, Color char_col, Color bg_col, Font *font);

void tl_draw_prose(uint x, uint y, char *wrapped_text, uint len, Color char_col, Color bg_col, Font *font);

Pos_t tl_get_grid_coords(Pos_t xy);

void tl_set_tile_bg(uint x, uint y, Color bg_col);


