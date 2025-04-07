#include <string.h>
#include <stdio.h>
#include "../c_codebase/src/raytiles.h"
#include "../c_codebase/src/common.h"
#include "../c_codebase/src/ascui.h"
#include "../map.h"

#include "editor_common.c"

// Global container variables
container_t *main_menu_cntr;
container_t *new_world_cntr;
container_t *load_world_cntr;

void bind_main_menu_ui()
{
	container_style_t s_0 = style(0, col8bt(2,0,1), col8bt(5,0,2), col8bt(7,7,3), '=', '|', '+');
	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_ortho = style(3, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');

	main_menu_cntr = ascui_container(true, PERCENTAGE, 100, VERTICAL, 3,
		ascui_container(true, PERCENTAGE, 33, VERTICAL, 1,
			ascui_text(true, STATIC, TILES, 1, strlen(" "), " ", s_1)
		),
		ascui_box(true, STATIC, PERCENTAGE, 33, HORIZONTAL, s_1, 4,
			ascui_button(true, STATIC, TILES, 9, strlen("<#-HGSS EDITOR-#>"), "<#-HGSS EDITOR-#>", s_1, NULL, NULL, NULL),
			new_world_cntr = ascui_button(true, HOVERABLE, TILES, 5, strlen(">New World"), ">New World", 
											s_0, editor_state_button, (void *)NEW_WORLD, &editor_state),
			load_world_cntr = ascui_button(true, HOVERABLE, TILES, 5, strlen(">Load World"), ">Load World", 
											s_0, editor_state_button, (void *)LOAD_WORLD, &editor_state),
			ascui_text(true, STATIC, TILES, 1, strlen("AsKaND!R W!s 'ab"), "AsKaND!R W!s 'ab", s_ortho)
		),
		ascui_container(true, PERCENTAGE, 33, VERTICAL, 1,
			ascui_text(true, STATIC, TILES, 1, strlen(" "), " ", s_1)
		)
	);
}

