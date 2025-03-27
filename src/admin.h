// #pragma once
// #include <stdbool.h>
// #include "religion.h"
// #include "culture.h"
// 
// typedef enum
// {
	// NATIONSTATE,	// legitimacy = cultural dominance?
	// THEOCRACY,		// legitimacy = control over holy sites etc.
	// MONARCHY,		// legitimacy = loyalty of substates? + clergy?
	// FEDERATION,		// legitimacy = loyalty of substates
	// ANARCHIC		// legitimacy = ? wellbeing?
// }motivation_type_e;
// 
// typedef struct
// {
	// motivation_type_e type;
	// void *motivation_data; // NATIONSTATE => culture, THEOCRACY => religion, ROYAL_REALM => ?
// }motivation_t;
// 
// typedef enum
// {
	// NO_FOCUS,
	// POP_GROWTH,
	// ASSIMILATION,
	// EXTRACTION
// }focus_t;
// 
// typedef struct
// {
	// bool military_allowed;
	// motivation_t motivation;
	// focus_t focus;
	// uint8_t legitimacy;
	// uint8_t parent_extraction_rate;
	// uint8_t loyalty_to_parent;
// 
	// uint8_t capital_tax;
	// uint8_t clergical_tax;
	// 
	// // int16_t b_production_pt;
	// // int16_t b_food_pt;
	// // int16_t b_gold_pt;
	// 
	// int16_t production_pt;
	// int16_t food_pt;
	// int16_t gold_pt;
	// 
	// uint16_t production_stored;
	// uint16_t food_stored;
	// int16_t gold_stored;
	// 
	// uint8_t sub_realm_count;
	// uint8_t sub_realm_capacity;
	// 
	// struct realm_t *child_realms;
	// struct realm_t *parent_realm;
// }admin_t;
// 
// typedef struct
// {
	// int16_t production_pt;
	// int16_t food_pt;
	// int16_t gold_pt;
	// 
	// uint16_t production_stored;
	// uint16_t food_stored;
	// int16_t gold_stored;
// }market_t;
// 
// typedef enum
// {
	// NO_STATUS,
// 
// }status_type_e;
// 
// typedef struct
// {
	// status_type_e type;
	// uint8_t duration;			// 255 -> permanent
// }status_t;
// 
// #define MAX_STATUSES 16
// typedef struct
// {
	// int16_t baked_food_yield;
	// int16_t baked_production_yield;
	// int16_t baked_gold_yield;
	// status_t statuses[MAX_STATUSES];
	// ui32_list_t cell_id_list;
	// admin_t admin;
	// market_t *mrkt;
// }realm_t;
