#ifndef __STM32F4_UB_MAZE_GEN_H
#define __STM32F4_UB_MAZE_GEN_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "maze.h"
#include "skin.h"

//--------------------------------------------------------------
// define for "maze_generate_searchandset"
//--------------------------------------------------------------
#define SEARCH_ROOMS_ALL   0 // set data of all rooms
#define SEARCH_DOORS_YES   1 // set typ+skin of all rooms with a door
#define SEARCH_SKIN_PATH   2 // set skin of all "path"
#define SEARCH_SKIN_UNDEF  3 // set skin of all "undef" rooms

//--------------------------------------------------------------
// function prototypes
//--------------------------------------------------------------

/**
 * @brief Initialize Maze.Room[][] as a random map (using your random logic)
 *        Replaces the old fixed digpath code.
 */
void maze_generate_init_random_map(void);

/**
 * @brief Mark & set rooms data (type, special, door, skin, points, etc)
 * @param mark : SEARCH_ROOMS_ALL, SEARCH_DOORS_YES, ...
 * @param room : the Room_t struct to be assigned
 */
void maze_generate_searchandset(uint32_t mark, Room_t room);

uint32_t maze_generate_setportals(uint32_t x,uint32_t y,Room_t room);
uint32_t maze_generate_setgate(uint32_t x,uint32_t y,Room_t room, uint32_t door);

/**
 * @brief Auto set outside walls' skin (top row, bottom row, left col, right col).
 */
void maze_generate_setwallskin_outside(void);

/**
 * @brief Auto set inside walls' skin.
 */
void maze_generate_setwallskin_inside(void);

/**
 * @brief Manually set horizontal skin for length l.
 */
uint32_t maze_generate_setskin_h(uint32_t x,uint32_t y,uint32_t l,uint32_t s);

/**
 * @brief Manually set vertical skin for length l.
 */
uint32_t maze_generate_setskin_v(uint32_t x,uint32_t y,uint32_t l,uint32_t s);

/**
 * @brief Check the maze (borders, path doors, etc.)
 * @return 0=OK, otherwise error code
 */
uint32_t maze_generate_check(void);

/**
 * @brief Count the rooms with points
 */
uint32_t maze_generate_count_dots(void);

#endif // __STM32F4_UB_MAZE_GEN_H
