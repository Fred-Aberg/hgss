#pragma once
#include <stdbool.h>
#include "religion.h"
#include "culture.h"

typedef enum
{
	NATIONSTATE,
	THEOCRACY,
	ROYAL_REALM,
	FEDERATION,
	ANARCHIC
}motivation_type_e;

typedef struct
{
	motivation_type_e type;
	void *motivation_data; // NATIONSTATE => culture, THEOCRACY => religion, ROYAL_REALM => ? 
}motivation_t;

typedef enum
{
	NO_FOCUS,
	POP_GROWTH,
	ASSIMILATION,
	EXTRACTION
}focus_e;

typedef struct
{
	bool military_allowed;
	motivation_t motivation;
	focus_e focus;
	uchar_t parent_extraction_rate;
	uchar_t loyalty_to_parent;
	uchar_t sub_admin_count;
	uchar_t sub_admin_capacity;

	uchar_t capital_tax;
	short b_production_pt;
	short b_food_pt;
	short b_gold_pt;
	
	short production_pt;
	short food_pt;
	short gold_pt;
	
	short production_stored;
	short food_stored;
	short gold_stored;
	
	ushort_t region_count;
	ushort_t region_capacity;
	ushort_t *region_ids;
	struct administration_t *child_admins;
	struct administration_t *parent_admin;
}administration_t;
