#pragma once
#include "c_codebase/src/common.h"
#define MAX_CULTURES 255

typedef struct
{
	uint16_t id;	// useful as long as the culture is alive, corresponds to index in w_living_cultures[]
	uint16_t point_of_divergence;
}culture_t;

typedef struct
{
	uint8_t child_count;
	uint8_t child_capacity;
	culture_t culture;	
	struct culture_node_t *parent_culture;
	struct culture_node_t *child_culture;
}culture_node_t;

typedef struct
{
	culture_node_t *root_culture;
}culture_tree_t;

typedef struct 
{
	uint8_t matrix[MAX_CULTURES][MAX_CULTURES];
}culture_relation_matrix_t;

void clt_calculate_relation_matrix(culture_relation_matrix_t *matrix, culture_tree_t culture_tree, culture_t *living_cultures[MAX_CULTURES]);

uint8_t clt_calculate_relation(culture_relation_matrix_t *matrix, culture_node_t clt_a, culture_node_t clt_b);
