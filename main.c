#include "iface.h"

#define WAITMS 300 
Uint32 lct = 0;

Uint32 chill() {
  Uint32 ct = SDL_GetTicks();
  if (ct < lct + WAITMS) {
    SDL_Delay(WAITMS - (ct - lct));
    ct = SDL_GetTicks();
  }
  Uint32 ret = ct - lct;
  lct = ct;
  return ret;  
}

Uint32 reds[] = {0xa4,0x4e,0xce,0x20,0x5c,0x8f,0xc4};
Uint32 greens[] = {0x00,0x9a,0x5c,0x4a,0x35,0x59,0xa0};
Uint32 blues[] = {0x00,0x06,0x00,0x87,0x66,0x02,0x00};

void drawbox(SDL_Surface *screen, int x, int y, int w, int h, Uint32 r, Uint32 g, Uint32 b) {
  SDL_Rect rt;
  rt.x = x;
  rt.y = y;
  rt.w = w;
  rt.h = h;
  SDL_FillRect(screen,&rt,(r<<16)|(g<<8)|(b));
}

int main(int argc, char** argv) {
  game_state* gs;
  gs = game_load(stdin,3.0,3.0,1.6,.07,.1);
  game_init(gs,6);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  SDL_Surface *screen;
  SDL_Event event;
  screen = SDL_SetVideoMode(600,400,32,SDL_HWSURFACE|SDL_DOUBLEBUF);
  if (screen == NULL) {
    fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_WM_SetCaption("Diamonds","");

  int i,t;
  int x,y;
  for (;;) {
    drawbox(screen,0,0,600,360,186,189,182);
    drawbox(screen,0,360,600,10,136,138,133);
    for (i=1;i<12;i++) {
      drawbox(screen,50*i,0,1,360,136,138,133);
      drawbox(screen,0,30*i,600,1,136,138,133);
    }
    drawbox(screen,0,370,300,30,reds[gs->ball.color],greens[gs->ball.color],blues[gs->ball.color]);
    if (gs->have_key) {
      drawbox(screen,300,370,300,30,reds[gs->key_color],greens[gs->key_color],blues[gs->key_color]);
    } else {
      drawbox(screen,300,370,300,30,186,189,182);
    }

    for (y=0;y<gs->config.y;y++) {
      for (x=0;x<gs->config.x;x++) {
        game_tile* tile;
        tile = &gs->tiles[gs->config.x*y + x];
        switch (tile->type) {
          case EMPTY_TILE:
            break;
          case COLOR_TILE:
            drawbox(screen,50*x,30*y,50,30,reds[tile->color],greens[tile->color],blues[tile->color]);
            break;
          case DIAMOND_TILE:
            drawbox(screen,50*x,30*y,50,30,255,255,255);
            drawbox(screen,50*x+10,30*y+10,30,10,0xd2,0xd7,0xcf);
            break;
          case REVERSER_TILE:
            drawbox(screen,50*x,30*y,50,30,0,0,0);
            drawbox(screen,50*x+10,30*y+10,30,10,reds[1]/2,greens[1]/2,blues[1]/2);
            break;
          case KILL_TILE: 
            drawbox(screen,50*x,30*y,50,30,0,0,0);
            drawbox(screen,50*x+10,30*y+10,30,10,reds[0]/2,greens[0]/2,blues[0]/2);
            break;
          case KEY_TILE:
            drawbox(screen,50*x,30*y,50,30,reds[tile->color],greens[tile->color],blues[tile->color]);
            drawbox(screen,50*x+10,30*y+10,30,10,255,255,255);
            break;
          case LOCK_TILE:
            drawbox(screen,50*x,30*y,50,30,255,255,255);
            drawbox(screen,50*x+10,30*y+10,30,10,reds[tile->color],greens[tile->color],blues[tile->color]);
            break;
          case PAINT_TILE:
            drawbox(screen,50*x,30*y,50,30,reds[tile->color],greens[tile->color],blues[tile->color]);
            drawbox(screen,50*x+10,30*y+10,30,10,reds[tile->color]/2,greens[tile->color]/2,blues[tile->color]/2);
            break;
          case BRICK_TILE:
            drawbox(screen,50*x,30*y,50,30,0x2e,0x34,0x36);
            break;
          default:
            break;
        }
      }
    }

    drawbox(screen,gs->ball.x*50-5,gs->ball.y*30-5,10,10,0x2e,0x34,0x36);
    SDL_Flip(screen);
    t = chill();
    
    if (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_LEFT:
              game_change_direction(gs,GAME_LEFT);
              break;
            case SDLK_RIGHT:
              game_change_direction(gs,GAME_RIGHT);
              break;
            case SDLK_ESCAPE:
              exit(0);
            default:
              break;
          }
          break;
        case SDL_KEYUP:
          switch (event.key.keysym.sym) {
            case SDLK_LEFT:
            case SDLK_RIGHT:
              game_change_direction(gs,GAME_STOP);
              break;
            default:
              break;
          }
          break;
        case SDL_QUIT:
          exit(0);
        default:
          break;
      }
    }

    game_update(gs,t/1000.0);

    if (gs->color_tile_count == 0 && gs->diamond_tile_count == 0) {
      printf("Victory!\n");
      break;
    }
    if (gs->lives == 0) {
      printf("You lose.\n");
      break;
    }
  }

  printf("Total time: %f\n",gs->time);

  game_close(gs);
  return 0;
}
