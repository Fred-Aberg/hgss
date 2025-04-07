#include <string.h>
#include <stdio.h>
#include "../c_codebase/src/raytiles.h"
#include "../c_codebase/src/common.h"
#include "../c_codebase/src/ascui.h"
#include "../map.h"

#include "editor_common.c"

// Global container variables
container_t *load_world_menu_cntr;

void bind_load_world_menu_ui()
{
	container_style_t s_0 = style(0, col8bt(2,0,1), col8bt(5,0,2), col8bt(7,7,3), '=', '|', '+');
	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');

	load_world_menu_cntr = ascui_container(true, PERCENTAGE, 100, VERTICAL, 3,
		ascui_container(true, PERCENTAGE, 10, HORIZONTAL, 2,
			ascui_button(true, HOVERABLE, TILES, 5, strlen("< BACK"), "< BACK", s_0, editor_state_button, MAIN_MENU, &editor_state),
			ascui_text(true, STATIC, TILES, 1, strlen(" "), " ", s_1)
		),
		ascui_box(true, STATIC, PERCENTAGE, 80, HORIZONTAL, s_1, 4,
			ascui_button(true, STATIC, TILES, 9, strlen("<#-LOAD WORLD-#>"), "<#-LOAD WORLD-#>", s_1, NULL, NULL, NULL),
			ascui_button(true, STATIC, TILES, 3, strlen("<#-LOAD FROM SAVE-#>"), "<#-LOAD FROM SAVE-#>", s_1, NULL, NULL, NULL),
			ascui_button(true, STATIC, TILES, 3, strlen("<#-LOAD FROM TEMPLATES-#>"), "<#-LOAD FROM TEMPLATES-#>", s_1, NULL, NULL, NULL),
			ascui_text(true, STATIC, TILES, 1, strlen(" "), " ", s_1)
		),
		ascui_container(true, PERCENTAGE, 10, VERTICAL, 1,
			ascui_text(true, STATIC, TILES, 1, strlen(" "), " ", s_1)
		)
	);
}

