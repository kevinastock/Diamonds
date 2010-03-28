#include "game.h"

game_direction game_actual_ball_direction(game_state* gs) {
  if (gs->ball.bounce == GAME_STOP) {
    return gs->ball.dir;
  } else {
    return gs->ball.bounce;
  }
}

void game_move_ball(game_state* gs, float dt) {
  if (gs->ball.going_up) {
    gs->ball.y += dt*gs->config.ud_speed;
  } else {
    gs->ball.y -= dt*gs->config.ud_speed;
  }

  if (game_actual_ball_direction(gs) == GAME_LEFT) {
    gs->ball.x -= dt*gs->config.lr_speed/gs->config.aspect;
  } else if (game_actual_ball_direction(gs) == GAME_RIGHT) {
    gs->ball.x += dt*gs->config.lr_speed/gs->config.aspect;
  }

  gs->time += dt;
  if (gs->ball.bounce != GAME_STOP && gs->ball.bounce_remaining >= 0) {
    gs->ball.bounce_remaining -= dt;
    if (gs->ball.bounce_remaining <= 0) {
      gs->ball.bounce = GAME_STOP;
    } else {
    }
  }
}

void game_bounce_ball(game_state* gs, game_direction d) {
  if (d == GAME_LEFT) {
    gs->ball.bounce = GAME_RIGHT;
  } else if (d == GAME_RIGHT) {
    gs->ball.bounce = GAME_LEFT;
  } else {
    // This shouldn't happen.
    printf("Uh oh, bounce without moving to side.\n");
    exit(1);
  }
  gs->ball.bounce_remaining = gs->config.bounce_time;
}

// Return 1 if the tile was hit, 0 if pass has been granted
// This is also responsible for actions caused by hitting the tile
// ie.  Tile removed, ball killed, key acquired, etc...
// Return 1 if bouncing off this tile, 0 if going through
int game_hit_tile(game_state* gs, int x, int y, int* react) {
  game_tile* t;
  t = &gs->tiles[gs->config.x * y + x];
  *react = 1;
  switch (t->type) {
    case EMPTY_TILE:
      return 0;
      break;
    case COLOR_TILE:
      if (t->color == gs->ball.color) {
        t->type = EMPTY_TILE;
        gs->color_tile_count--;
      }
      return 1;
      break;
    case DIAMOND_TILE:
      if (gs->color_tile_count == 0) {
        t->type = EMPTY_TILE;
        gs->diamond_tile_count--;
      }
      return 1;
      break;
    case REVERSER_TILE:
      t->type = EMPTY_TILE;
      gs->reversed = !gs->reversed;
      if (gs->ball.dir == GAME_LEFT) {
        gs->ball.dir = GAME_RIGHT;
      } else if (gs->ball.dir == GAME_RIGHT) {
        gs->ball.dir = GAME_LEFT;
      }
      return 1;
      break;
    case KILL_TILE:
      *react = 0; // To stop the new ball from bouncing
      gs->lives--;
      gs->reversed = 0;
      gs->ball.color = gs->config.init_color;
      gs->ball.x = gs->config.ix + 0.5;
      gs->ball.y = gs->config.iy + 0.5;
      gs->ball.going_up = gs->config.going_up_init;
      gs->ball.bounce = GAME_STOP;
      return 1;
      break;
    case KEY_TILE:
      if (!gs->have_key && gs->ball.color == t->color) {
        t->type = EMPTY_TILE;
        gs->key_color = t->color;
        gs->have_key = 1;
      }
      return 1;
      break;
    case LOCK_TILE:
      if (gs->have_key && gs->ball.color == t->color && gs->key_color == t->color) {
        t->type = EMPTY_TILE;
        gs->have_key = 0;
      }
      return 1;
      break;
    case PAINT_TILE:
      gs->ball.color = t->color;
      return 1;
      break;
    case BRICK_TILE:
      return 1;
      break;
    default:
      printf("Unknown tile type.\n");
      exit(1);
      break;
  }
  
  return t->type;
}

void game_update(game_state* gs, float dt) { // Note that this algorithm will break if the ball is moving too fast
  // This currently assumes the ball is actually a square for easier math
  // Could probably be changed to an actual round ball with some ease
  
  if (gs->ball.bounce != GAME_STOP && gs->ball.bounce_remaining < dt) {
    // Account for ball changing direction in the middle of the update
    dt -= gs->ball.bounce_remaining;
    game_update(gs,gs->ball.bounce_remaining);
  }

  // What direction is the ball moving (left/right/stop)?
  game_direction gd;
  gd = game_actual_ball_direction(gs);

  // Compute the time to reach the next x and y boundaries
  float ttxw, ttyw; 
  int ix, iy,tx,ty;
  tx = floor(gs->ball.x);
  ty = floor(gs->ball.y);
 
  if (gs->ball.going_up) {
    float t = gs->ball.y + gs->config.ball_size;
    ttyw = ceil(t) - t;
    iy = ceil(t);
  } else {
    float t = gs->ball.y - gs->config.ball_size;
    ttyw = t - floor(t);
    iy = floor(t) - 1;
  }
  ttyw /= gs->config.ud_speed;

  if (gd == GAME_LEFT) {
    float t = gs->ball.x - gs->config.ball_size/gs->config.aspect;
    ttxw = t - floor(t);
    ix = floor(t) - 1;
  } else if (gd == GAME_RIGHT) {
    float t = gs->ball.x + gs->config.ball_size/gs->config.aspect;
    ttxw = ceil(t) - t;
    ix = ceil(t);
  } else {
    ttxw = 1000;
    ix = tx;
  }
  ttxw /= gs->config.lr_speed/gs->config.aspect;

  // Based on the distance to the boundaries of this tile, pick an action

  if ((ttxw > dt && ttyw > dt)) { // || (ttxw <= 0 && ttyw > dt) || (ttyw <= 0 && ttxw > dt)) {
    // Just move the ball and return, we're not leaving this tile
    game_move_ball(gs,dt);
  } else if (ttxw < ttyw && ttxw >= 0) {
    // We're going to hit a side of this tile, do work then recurse
    game_move_ball(gs,ttxw);
    dt -= ttxw;
    ttyw -= ttxw;
    // Attempt hitting the tile we're closest to
    int ty1,ty2,react;
    ty1 = floor(gs->ball.y - gs->config.ball_size);
    ty2 = floor(gs->ball.y + gs->config.ball_size);
    game_direction td = gs->ball.dir;
    if (ix < 0 || ix == gs->config.x || game_hit_tile(gs,ix,ty,&react) || game_hit_tile(gs,ix,ty1,&react) || game_hit_tile(gs,ix,ty2,&react)) {
      // we hit a tile on the side, so bounce off
      // (or a boundary wall side)
      if (react) game_bounce_ball(gs,td);
    }
    if (dt <= ttyw) {
      game_move_ball(gs,dt);
      if (dt==ttyw) {
        gs->ball.going_up = !gs->ball.going_up;
      }
    } else {
      game_move_ball(gs,ttyw/2.0);
      game_update(gs,ttyw/2.0);
    }
  } else if (ttyw >= 0) {
    // We're going to hit the top or bottom of this tile, do work then recurse
    game_move_ball(gs,ttyw);
    dt -= ttyw;
    ttxw -= ttyw;
    int tx1,tx2,react;
    tx1 = floor(gs->ball.x - gs->config.ball_size/gs->config.aspect);
    tx2 = floor(gs->ball.x + gs->config.ball_size/gs->config.aspect);
    game_direction td = gs->ball.dir;
    if (iy < 0 || iy == gs->config.y || game_hit_tile(gs,tx,iy,&react) || game_hit_tile(gs,tx1,iy,&react) || game_hit_tile(gs,tx2,iy,&react)) {
      // we hit a tile on the top or bottom or a boundary top/bottom
      // reverse the direction of the ball
      if (react) gs->ball.going_up = !gs->ball.going_up;
    }
    if (dt <= ttxw || ttxw < 0) {
      game_move_ball(gs,dt);
      if (dt == ttxw) {
        game_bounce_ball(gs,td);
      }
    } else {
      game_move_ball(gs,ttxw/2.0);
      game_update(gs,ttxw/2);
    } 
  } else {
    game_move_ball(gs,dt);
  }
}

void game_change_direction(game_state* gs, game_direction dir) {
  if (gs->reversed && dir == GAME_LEFT) {
    dir = GAME_RIGHT;
  } else if (gs->reversed && dir == GAME_RIGHT) {
    dir = GAME_LEFT;
  }
  gs->ball.dir = dir;
}

game_state* game_load(FILE* f, float lr, float ud, float aspect, float bounce, float bsize) {
  game_state* gs;
  gs = malloc(sizeof(game_state));
  gs->config.colored_tiles = 0;
  gs->config.diamond_tiles = 0;
  gs->config.lr_speed = lr;
  gs->config.ud_speed = ud;
  gs->config.aspect = aspect;
  gs->config.bounce_time = bounce;
  gs->config.ball_size = bsize;

  int i;
  i = fscanf(f,"%50s %d %d %d %d %d %d",gs->config.name,&gs->config.x,&gs->config.y,&gs->config.ix,&gs->config.iy,&gs->config.init_color,&gs->config.going_up_init);
  if (i != 7) {
    free(gs);
    return NULL;
  }

  gs->tiles = malloc(gs->config.x * gs->config.y * sizeof(game_tile));
  gs->config.tiles = malloc(gs->config.x * gs->config.y * sizeof(game_tile));

  int x,y;
  for (y=0;y<gs->config.y;y++) {
    for (x=0;x<gs->config.x;x++) {
      int type;
      i = fscanf(f,"%d %d",&type,&(gs->config.tiles[gs->config.x * y + x].color));
      gs->config.tiles[gs->config.x * y + x].type = (game_tile_type) type;
      if (i != 2) {
        free(gs->config.tiles);
        free(gs->tiles);
        free(gs);
        return NULL;
      }
      if (gs->config.tiles[gs->config.x * y + x].type == COLOR_TILE) {
        gs->config.colored_tiles++;
      }
      if (gs->config.tiles[gs->config.x * y + x].type == DIAMOND_TILE) {
        gs->config.diamond_tiles++;
      }
    }
  }

  return gs;
}

void game_init(game_state* gs, int lives) {
  gs->time = 0.0;
  gs->lives = lives;
  gs->reversed = 0;
  gs->have_key = 0;
  gs->color_tile_count = gs->config.colored_tiles;
  gs->diamond_tile_count = gs->config.diamond_tiles;
  memcpy(gs->tiles,gs->config.tiles,gs->config.x * gs->config.y * sizeof(game_tile));
  gs->ball.color = gs->config.init_color;
  gs->ball.x = gs->config.ix + 0.5;
  gs->ball.y = gs->config.iy + 0.5;
  gs->ball.going_up = gs->config.going_up_init;
  gs->ball.dir = GAME_STOP;
  gs->ball.bounce = GAME_STOP;
}

void game_close(game_state* gs) {
  free(gs->config.tiles);
  free(gs->tiles);
  free(gs);
}
