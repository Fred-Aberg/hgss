#include "world.h"
#include "map.h"

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



uint16_t w_add_realm(world_t *w, ui32_list_t cell_id_list)
{
	// Search for a gap
	for (uint16_t i = 0; i < w->realm_count; i++)
	{
		if(!w->realms[i].is_active)
		{
			w->realms[i].realm.cell_id_list = cell_id_list;
			w->realms[i].is_active = true;
			return i;
		}
	}

	// New allocation
	if (w->realm_capacity <= w->realm_count)
	{
		w->realm_capacity = w->realm_capacity * REALM_REALLOC_INCREASE + 1; 
		w->realms = (realm_entry_t *)realloc(w->realms, (w->realm_capacity * sizeof(realm_entry_t)));
	}

	w->realms[w->realm_count].realm.cell_id_list = cell_id_list ;
	w->realms[w->realm_count].is_active = true;
	w->realm_count++;
	return w->realm_count - 1;
}
