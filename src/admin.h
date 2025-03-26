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
	uint8_t parent_extraction_rate;
	uint8_t loyalty_to_parent;
	uint8_t sub_admin_count;
	uint8_t sub_admin_capacity;

	uint8_t capital_tax;
	int16_t b_production_pt;
	int16_t b_food_pt;
	int16_t b_gold_pt;
	
	int16_t production_pt;
	int16_t food_pt;
	int16_t gold_pt;
	
	int16_t production_stored;
	int16_t food_stored;
	int16_t gold_stored;
	
	uint16_t region_count;
	uint16_t region_capacity;
	uint16_t *region_ids;
	struct administration_t *child_admins;
	struct administration_t *parent_admin;
}administration_t;
