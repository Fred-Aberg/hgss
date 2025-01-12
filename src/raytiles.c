#include "raylib.h"
#include "raytiles.h"
#include "math.h"
#include "stdio.h"
#include "assert.h"
#include "string.h"

typedef struct
{
    Color bg_col;
    Color char_col;
    char symbol;
    Font *font;
}Tile_t;

typedef struct
{
    int scr_size_x;
    int scr_size_y;
    uint resolution_x;
    uint resolution_y;
    int tile_size_x;
    int tile_size_y;
    Color default_col;
    Font *default_font;

    Tile_t *tiles;
} Grid_t;

// Global address 
Grid_t *GRID = NULL;
#define X_RES GRID->resolution_x
#define Y_RES GRID->resolution_y
#define X_TL_SIZE GRID->tile_size_x
#define Y_TL_SIZE GRID->tile_size_y
#define FONT_SIZE GRID->tile_size_x * 1.2f
#define SIZE X_RES * Y_RES

#define MIN(a, b) (a < b)? a : b

Tile_t *get_tile(uint x, uint y)
{
    assert(x < X_RES && y < Y_RES); // kys
    return &GRID->tiles[x + y * X_RES];
}

// Convert array index of cell to cell's coordinates
static void i_to_pos(int i, uint *x, uint *y)
{
    *y = i / X_RES; // C autimatically "floors" the result 
    *x = i - (*y * X_RES);
}

int tl_init_grid(uint res_x, int scr_size_x, int scr_size_y, Color def_col, Font *def_font)
{
    GRID = calloc(1, sizeof(Grid_t));
    GRID->tiles = calloc(res_x, sizeof(Tile_t*));

    GRID->scr_size_x = scr_size_x;
    GRID->scr_size_y = scr_size_y;
    GRID->resolution_x = res_x;
    GRID->default_col = def_col;
    GRID->default_font = def_font;
    GRID->tile_size_x = scr_size_x / res_x;
    GRID->tile_size_y = (scr_size_x / res_x) * 1.2f;

    // Calculate allowed res_y based on scr_size_y
    GRID->resolution_y = floor(scr_size_y / GRID->tile_size_y);

    GRID->tiles = calloc(X_RES * Y_RES, sizeof(Tile_t));

    printf("tl_grid initialized:\n    size: %d x %d -> %d tiles\n    coords: (0-%d, 0-%d)", X_RES, Y_RES, SIZE, X_RES - 1, Y_RES - 1);
    
    return Y_RES;
}

Pos_t tl_grid_get_size()
{
    return (Pos_t){.x = X_RES, .y = Y_RES};
}

void tl_deinit_grid()
{
    free(GRID->tiles);
    free(GRID);
    GRID = NULL;
}

void tl_render_grid()
{
    uint x = 0;
    uint y = 0;
    Tile_t tile;
    Font *tile_font;
    for (uint i = 0; i < SIZE; i++)
    {
            tile = GRID->tiles[i];
            i_to_pos(i, &x, &y);

            if (tile.bg_col.a != 0) DrawRectangle(x * X_TL_SIZE, y * Y_TL_SIZE, X_TL_SIZE, Y_TL_SIZE, tile.bg_col);
            else                    DrawRectangle(x * X_TL_SIZE, y * Y_TL_SIZE, X_TL_SIZE, Y_TL_SIZE, GRID->default_col);

            char draw_text[2];
            draw_text[0] = tile.symbol;
            draw_text[1] = 0; 
            if (tile.symbol != 0 && tile.char_col.a != 0)
            {
                tile_font = (tile.font != NULL)? tile.font : GRID->default_font;
                DrawTextEx(*tile_font, draw_text, (Vector2){x * X_TL_SIZE, y * Y_TL_SIZE}, FONT_SIZE, 0, tile.char_col);
            }

    }
}

void tl_draw_tile(uint x, uint y, char smbl, Color char_col, Color bg_col, Font *font)
{
    Tile_t *tile = get_tile(x, y);
    tile->bg_col = bg_col;
    tile->char_col = char_col;
    tile->symbol = smbl;
    tile->font = font;
}

void tl_draw_rect(uint x0, uint y0, uint width, uint height, char smbl, Color char_col, Color bg_col, Font *font)
{
    for (uint _x = x0; _x <= x0 + width; _x++)
    {
        for (uint _y = y0; _y <= y0 + height; _y++)
        {
            tl_draw_tile(_x, _y, smbl, char_col, bg_col, font);
        }
    }
}

void tl_draw_line(uint x0, uint y0, uint x1, uint y1, char smbl, Color char_col, Color bg_col, Font *font)
{
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    int dx = abs(x1 - x0);
    int sx = (x0 < x1)? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = (y0 < y1)? 1 : -1;
    int error = dx + dy;
    
    while (true)
    {
        tl_draw_tile(x0, y0, smbl, char_col, bg_col, font);

        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * error;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            error += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            if (y0 == y1) break;
            error += dx;
            y0 += sy;
        }
    }
}

// wrap = 0 => no wrapping
uint tl_draw_text(uint x, uint y, uint wrap, char *text, uint len, Color char_col, Color bg_col, Font *font)
{
    uint _x = x;
    uint _y = y;

    for (uint i = 0; i < len; i++)
    {
        if(text[i] != '\n')
            tl_draw_tile(_x, _y, text[i], char_col, bg_col, font);

        // Wrap around to start of x, on new line (y) and continue
        _x++;
        if ((_x >= wrap && wrap != 0) || text[i] == '\n') {_x = x; _y++;}
    }

    return _y - y + 1; // Rows written
}

Pos_t tl_get_grid_coords(Pos_t xy)
{
    return pos(xy.x / GRID->tile_size_x, xy.y / GRID->tile_size_y);
}

void tl_set_tile_bg(uint x, uint y, Color bg_col)
{
    Tile_t *tile = get_tile(x, y);
    tile->bg_col = bg_col;
}
