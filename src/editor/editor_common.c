#pragma once
#include <string.h>
#include <stdio.h>
#include "../c_codebase/src/raytiles.h"
#include "../c_codebase/src/common.h"
#include "../c_codebase/src/ascui.h"
#include "../map.h"

#define MAIN_MENU 0
#define NEW_WORLD 1
#define LOAD_WORLD 2
#define EDITOR 3

intptr_t editor_state = 0;


void dropdown_button(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	container_t *container = *(container_t **)domain;
	container->open = !container->open;
}

void editor_state_button(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	*(intptr_t *)function_data = (intptr_t)domain;
}
