/*
   the game manager

   a game is composed of tiles and a ball
   when a ball hits a tile:
     - the tile can be removed
     - the ball can be killed
     - the ball can change color
     - a key can be acquired
     - nothing

     - if hitting the top/bottom of a tile, change up/down direction
     - if hitting the side, "bounce" off the tile for a bit of time
*/
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define GAME_MAX_SIZE 12

typedef enum _game_direction {
  GAME_LEFT,
  GAME_RIGHT,
  GAME_STOP,
} game_direction;

typedef enum _game_tile_type {
  EMPTY_TILE        = 0,
  COLOR_TILE        = 1,
  DIAMOND_TILE      = 2,
  REVERSER_TILE     = 3,
  KILL_TILE         = 4,
  KEY_TILE          = 5,
  LOCK_TILE         = 6,
  PAINT_TILE        = 7,
  BRICK_TILE        = 8,
  /*
     KILL_IF_COLOR_TILE
     KILL_IFN_COLOR_TILE
     PASS_IF_COLOR_TILE
     PASS_IFN_COLOR_TILE
  */
} game_tile_type;

typedef struct _game_tile {
  int color;
  game_tile_type type;
} game_tile;

typedef struct _game_ball {
  int color;
  float x,y;
  int going_up; 
  game_direction dir; // Which they ball is being moved
  game_direction bounce; // Which way the ball is bouncing to (overrides dir if not stop)
  float bounce_remaining; // How much longer to bounce for in seconds
} game_ball;

// This struct containts values that stay constant during the game
typedef struct _game_config {
  // Defined by interface
  float lr_speed, ud_speed; // Units per second left/right ; up/down
  float aspect; // = (x pixels / y pixels)
  float bounce_time; // Duration of a bounce in seconds
  float ball_size; // Radius of the ball

  // Defined by the level
  char name[50]; // Name cannot have spaces in it
  int x,y; // Number of tiles in each direction
  int ix,iy; // The tile the ball should start in
  int init_color; // What color the ball starts as
  int going_up_init;
  int colored_tiles;
  int diamond_tiles;
  game_tile* tiles;
} game_config;

typedef struct _game_state {
  game_tile* tiles;
  game_ball ball;
  game_config config;
  float time;
  int lives;
  int reversed;
  int have_key;
  int key_color;
  int color_tile_count; // Used to quickly test if we can remove diamond tiles
  int diamond_tile_count;
} game_state;

// Move the game forward in time by dt seconds
extern void game_update(game_state* gs, float dt);

// Set the ball moving left, right, or stop the ball from sideways movement
extern void game_change_direction(game_state* gs, game_direction dir);

// Load a game file
extern game_state* game_load(FILE* f, float lr, float ud, float aspect, float bounce, float bsize);

// Initalize a game level for play
extern void game_init(game_state* gs, int lives);

// Close a game, deallocate all memory being used by the game
extern void game_close(game_state* gs);

#endif /* GAME_H */
