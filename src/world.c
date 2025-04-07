#include "world.h"
#include "map.h"
#include <stdio.h>

#define REALM_REALLOC_INCREASE 1.5f

bool make_ai_move(uint16_t realm_id) // false if AI isn't finnished, true if AI is finnished
{
	return 1;
}

world_t *w_create(uint16_t init_realm_capacity, map_t *map)
{
	world_t *w = calloc(1, sizeof(world_t));
	w->realm_capacity = init_realm_capacity;
	w->w_map = map;

	return w;
}

// Takes ownership of cell_id_list.items
uint16_t w_add_realm(world_t *w, ui32_list_t cell_id_list)
{
	// Search for a gap
	for (uint16_t i = 1; i < w->realm_count; i++)
	{
		if(!w->realms[i].is_active)
		{
			w->realms[i].realm.cell_id_list = cell_id_list;
			w->realms[i].is_active = true;
			
			for (uint16_t i = 0; i < cell_id_list.count; i++)
				map_get_cell_i(w->w_map, cell_id_list.items[i])->realm_id = i;
				
			return i;
		}
	}

	// New allocation
	if (w->realm_capacity <= w->realm_count)
	{
		w->realm_capacity = w->realm_capacity * REALM_REALLOC_INCREASE + 1; 
		w->realms = (realm_entry_t *)realloc(w->realms, (w->realm_capacity * sizeof(realm_entry_t)));
	}

	w->realms[w->realm_count].realm.cell_id_list = cell_id_list;
	w->realms[w->realm_count].is_active = true;
	for (uint16_t i = 0; i < cell_id_list.count; i++)
		map_get_cell_i(w->w_map, cell_id_list.items[i])->realm_id = w->realm_count;
	w->realm_count++;
	return w->realm_count - 1;
}

void w_disperse_realms(world_t *w, uint16_t rlm_count)
{
	uint16_t c_rlm_count = 0;
	pos16_t rlm_p;

	cell_t *c;
	ui32_list_t c_list = (ui32_list_t){0,0,0, true}; // empty ordered list
	while (c_rlm_count < rlm_count)
	{
		rlm_p = pos16(GetRandomValue(1, w->w_map->width - 2), GetRandomValue(1, w->w_map->height - 2));

		c = map_get_cell_p(w->w_map, rlm_p);
		if (c->env == WATER || c->env == RIVER || c->realm_id != NO_REALM_ID)
			continue; // Unsuitable location

		ui32_list_add(&c_list, map_p_to_i(w->w_map, rlm_p));
		printf("\n new realm: (%u, %u)", rlm_p.x, rlm_p.y);
		
		c = map_get_cell_p(w->w_map, pos16(rlm_p.x + 1, rlm_p.y));
		if (c->env != WATER && c->env != RIVER && c->realm_id == NO_REALM_ID)
			ui32_list_add(&c_list, map_p_to_i(w->w_map, pos16(rlm_p.x + 1, rlm_p.y)));
			
		c = map_get_cell_p(w->w_map, pos16(rlm_p.x - 1, rlm_p.y));
		if (c->env != WATER && c->env != RIVER && c->realm_id == NO_REALM_ID)
			ui32_list_add(&c_list, map_p_to_i(w->w_map, pos16(rlm_p.x - 1, rlm_p.y)));
			
		c = map_get_cell_p(w->w_map, pos16(rlm_p.x, rlm_p.y + 1));
		if (c->env != WATER && c->env != RIVER && c->realm_id == NO_REALM_ID)
			ui32_list_add(&c_list, map_p_to_i(w->w_map, pos16(rlm_p.x, rlm_p.y + 1)));
			
		c = map_get_cell_p(w->w_map, pos16(rlm_p.x, rlm_p.y - 1));
		if (c->env != WATER && c->env != RIVER && c->realm_id == NO_REALM_ID)
			ui32_list_add(&c_list, map_p_to_i(w->w_map, pos16(rlm_p.x, rlm_p.y - 1)));

		w_add_realm(w, c_list); // new realm takes ownership of c_list.items
		c_list.items = NULL;
		c_list.capacity = 0;
		c_list.count = 0;
		c_rlm_count++;
	}
}
