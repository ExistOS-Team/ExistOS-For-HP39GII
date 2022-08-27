// #include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

#include "sys_llapi.h"

unsigned int frames;

#include "keyboard_gii39.h"
#include <string.h>

static int button_start, button_select;
static int button_a, button_b;
static int button_down, button_up, button_left, button_right;
static int button_debug, button_quit;

/*
struct keymap
{
	SDL_Scancode code;
	int *key;
	void (*f)(void);
	int prev;
};
*/
static void debug()
{
	cpu_print_debug();
}

/*
static struct keymap keys[] =
{
	{SDL_SCANCODE_A,     &button_a,      NULL, 0},
	{SDL_SCANCODE_S,     &button_b,      NULL, 0},
	{SDL_SCANCODE_D,     &button_select, NULL, 0},
	{SDL_SCANCODE_F,     &button_start,  NULL, 0},
	{SDL_SCANCODE_LEFT,  &button_left,   NULL, 0},
	{SDL_SCANCODE_RIGHT, &button_right,  NULL, 0},
	{SDL_SCANCODE_UP,    &button_up,     NULL, 0},
	{SDL_SCANCODE_DOWN,  &button_down,   NULL, 0},
	{SDL_SCANCODE_ESCAPE,   &button_quit,   NULL, 0},
	{SDL_SCANCODE_F1,    &button_debug, debug, 0}
};
*/

int sdl_init(void)
{
	/*
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(
		"Fer is an ejit",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640, 576,
		SDL_WINDOW_INPUT_FOCUS
	);

	surface = SDL_GetWindowSurface(window);
	*/

	return 0;
}

int sdl_update(void)
{
	uint32_t keys = ll_vm_check_key();
	uint32_t key = keys & 0xFFFF;
	uint32_t press = keys >> 16;

/*
	button_up =    (key == KEY_UP)    & (press);
	button_down =  (key == KEY_DOWN)  & (press);
	button_left =  (key == KEY_LEFT)  & (press);
	button_right = (key == KEY_RIGHT) & (press);
*/
	button_up    = ((key == KEY_8) | (key == KEY_UP)    ) & (press);
	button_down  = ((key == KEY_2) | (key == KEY_DOWN)  ) & (press);
	button_left  = ((key == KEY_4) | (key == KEY_LEFT)  ) & (press);
	button_right = ((key == KEY_6) | (key == KEY_RIGHT) ) & (press);

	button_a = ((key == KEY_SYMB) || (key == KEY_5)) & (press);

	button_b = (key == KEY_HOME) & (press);
	button_start = (key == KEY_NUM) & (press);
	button_select = (key == KEY_VIEWS) & (press);


	if((key == KEY_9) & (press))
	{
		button_a = 1;
		button_right = 1;
	}

	if((key == KEY_7) & (press))
	{
		button_a = 1;
		button_left = 1;
	}

/*

	SDL_Event e;
	const unsigned char *keystates;
	size_t i;

	keystates = SDL_GetKeyboardState(NULL);

	while(SDL_PollEvent(&e))
	{
		if(e.type == SDL_QUIT)
			return 1;
	}

	for(i = 0; i < sizeof (keys) / sizeof (struct keymap); i++)
	{
		if(!keystates[keys[i].code])
		{
			if(keys[i].key)
				*(keys[i].key) = 0;
			continue;
		}

		if(keys[i].f && keys[i].prev == 0)
		{
			*(keys[i].key) = 1;
			keys[i].f();
		}

		keys[i].prev = *(keys[i].key);
		*(keys[i].key) = keystates[keys[i].code];
	}

	if(button_quit)
	{
		printf("frames: %d\n", frames);
		return 1;
	}
*/
	return 0;
}

unsigned int sdl_get_buttons(void)
{
	return (button_start*8) | (button_select*4) | (button_b*2) | button_a;
}

unsigned int sdl_get_directions(void)
{
	return (button_down*8) | (button_up*4) | (button_left*2) | button_right;
}

uint8_t gb_frame_buffer[256 * 127];

unsigned int *sdl_get_framebuffer(void)
{
	//return surface->pixels;
	return (unsigned int *)gb_frame_buffer;
}

void sdl_frame(void)
{
	frames++;
	if(frames % 5 == 0)
		ll_disp_put_area(gb_frame_buffer, 0, 0, 255, 126);
	//SDL_UpdateWindowSurface(window);
}

void sdl_frame_i(void)
{
		ll_disp_put_area(gb_frame_buffer, 0, 0, 255, 126);
	//SDL_UpdateWindowSurface(window);
}

void sdl_frame_clr(uint8_t c)
{
	memset(gb_frame_buffer, c, sizeof(gb_frame_buffer));
	ll_disp_put_area(gb_frame_buffer, 0, 0, 255, 126);
	//SDL_UpdateWindowSurface(window);
}

void sdl_quit()
{
	//SDL_Quit();
}
