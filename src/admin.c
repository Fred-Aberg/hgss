#include "admin.h"

realm_t *adm_rlm_create(ui32_list_t cell_id_list)
{
	region_t region;
	region.cell_count = cell_count;
	region.cell_capacity = MAX_REGION_CELL_CAP - 1;
	region.cell_ids = calloc(region.cell_capacity, sizeof(uint32_t));
	/*if(cell_count != 0)
	{
		memcpy(region.cell_ids, cell_ids, cell_count * sizeof(uint32_t));
		qsort(region.cell_ids, cell_count, sizeof(uint32_t), comp_cells_qs);
		
		// uint32_t first_id = region.cell_ids[0];
		// 
		// llong_t comparison;
		// for (uint16_t i = 0; i < map->region_count; i++)
		// {
			// comparison = comp_cells(first_id, map->regions[i].cell_ids[0]);
			// if(comparison > 0)
			// {
				// // Move all regions from and including i one step forward
				// memmove(&map->regions[i + 1], &map->regions[i], (map->region_count - i) * sizeof(region_t));
				// 
				// map->regions[i] = region;
				// map->region_count++;
				// return &map->regions[i];
			// }
		// }
	}*/

	// Add empty region -> append
	map->regions[map->region_count] = region;
	map->region_count++;
	return &map->regions[map->region_count - 1];
}
