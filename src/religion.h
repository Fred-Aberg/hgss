#pragma once
#include "c_codebase/src/common.h"
#define MAX_RELIGIONS 255 // Maximum number of (living) religions

typedef struct
{
	uint_t point_of_divergence;	// used in determining "distance" between religions and traversal-optimization
}religion_t;

typedef struct
{
	uchar_t child_count;
	uchar_t child_capacity;
	religion_t religion;	
	struct religion_node_t *parent_religion;
	struct religion_node_t *child_religions;
}religion_node_t;

typedef struct
{
	religion_node_t *root_religion;
}religion_tree_t;

typedef struct 
{
	uchar_t matrix[MAX_RELIGIONS][MAX_RELIGIONS];
}religion_relation_matrix_t;
