#ifndef bip_h
#define bip_h

#include <SDL.h>
#include <time.h>

#include "osc.h"
#include "biquad.h"

#define CHANNELS 16
#define FRAGSIZE 256
#define SRATE 48000


struct channel {
	float t;
	float duration;
	float gain[2];
	float window;
	struct osc osc;
	struct biquad biquad;
};


struct bip {
	time_t t_last_line;
	char fname_lua[256];
	time_t mtime_lua;
	lua_State *L;
	SDL_AudioDeviceID adev;
	char line[256];
	int len;
	struct channel channel_list[CHANNELS];

	float gain;
	float pan;
};

void init_api(lua_State *L);
struct channel *alloc_channel(struct bip *bip, float gain, float pan);

#endif
