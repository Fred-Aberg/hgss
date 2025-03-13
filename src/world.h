#pragma once
#include "c_codebase/src/common.h"
#include "map.h"
#include "religion.h"
#include "culture.h"
#include "admin.h"

typedef struct
{
	ushort_t admin_count;
	ushort_t admin_capacity;
	administration_t *top_lvl_admins;
	religion_t *living_religions[MAX_RELIGIONS];
	religion_tree_t religion_tree;
	culture_t *living_cultures[MAX_CULTURES];
	culture_tree_t culture_tree;
	religion_relation_matrix_t *religion_relation_matrix;
	culture_relation_matrix_t *culture_relation_matrix;
	ushort_t *unclaimed_region_ids;
	map_t w_map;
}world_t;
