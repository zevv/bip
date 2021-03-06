
#include <stdio.h>
//#include <math.h>
#include <sys/poll.h>
#include <time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <pty.h>
#include <fcntl.h>

#include "api.h"
#include "bip.h"
#include "osc.h"
#include "biquad.h"

#define DOC(...) #__VA_ARGS__

const char *default_bip_lua = DOC(

   function on_line(line)
      local l = #line
      local freq = 110 * math.pow(1.05946309, (l-10) % 60)
      pan = (l % 16) / 8 - 1.0
      bip(freq, 0.05, 1.0, pan)
   end

);


struct channel;

typedef float (*generator)(struct channel *channel);


static void init_lua(struct bip *bip);
static void init_audio(struct bip *bip);
static void handle_stdin(struct bip *bip);
static void handle_char(struct bip *bip, char c);
static void handle_line(struct bip *bip);
static void on_audio(void *userdata, uint8_t *stream, int len);
static bool load_lua(struct bip *bip);
static bool call_lua(struct bip *bip, int nargs);


int main(int argc, char **argv)
{
	// Init new bip

	struct bip *bip = calloc(1, sizeof(struct bip));
	bip->gain = 1.0;
	bip->pan = 0.0;
	for(size_t i=0; i<CHANNELS; i++) {
		struct channel *ch = &bip->channel_list[i];
		osc_init(&ch->osc, SRATE);
	}

	if(argc >= 2) {
		snprintf(bip->fname_lua, sizeof(bip->fname_lua), "%s", argv[1]);
	} else {
		const char *home = getenv("HOME");
		if(home == NULL) home = ".";
		snprintf(bip->fname_lua, sizeof(bip->fname_lua), "%s/.bip.lua", home);
	}

	init_lua(bip);
	init_audio(bip);

	// All done, run main loop

	handle_stdin(bip);

	// Cleanup

	SDL_CloseAudioDevice(bip->adev);
	lua_close(bip->L);
	free(bip);

	return 0;
}


static void init_lua(struct bip *bip)
{
	bip->L = luaL_newstate();
	luaL_openlibs(bip->L);

	lua_pushlightuserdata(bip->L, bip);
	lua_setfield(bip->L, LUA_REGISTRYINDEX, "bip");

	init_api(bip->L);

	// Load simple built-in script
	int r = luaL_loadstring(bip->L, default_bip_lua);
	if(r != 0) {
		fprintf(stderr, "error: %s\n", lua_tostring(bip->L, -1));
		return;
	}
	call_lua(bip, 0);

	// Try to load user provided bip.lua
	load_lua(bip);
}


static void init_audio(struct bip *bip)
{
	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec want = {
		.freq = SRATE,
		.format = AUDIO_F32,
		.channels = 2,
		.samples = FRAGSIZE,
		.callback = on_audio,
		.userdata = bip,
	};

	SDL_AudioSpec have;

	bip->adev = SDL_OpenAudioDevice(NULL, false, &want, &have, 0);

	if(bip->adev == 0) {
		fprintf(stderr, "Failed to open audio: %s", SDL_GetError());
		exit(1);
	}

	SDL_PauseAudioDevice(bip->adev, 0);
}


static void handle_stdin(struct bip *bip)
{
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

	struct pollfd pfd = {
		.fd = 0,
		.events = POLLIN
	};

	char buf[4096];

	for(;;) {
		int r = poll(&pfd, 1, -1);
		if(r < 0) {
			fprintf(stderr, "poll(): %s\n", strerror(errno));
			exit(1);
		}

		r = read(0, buf, sizeof(buf));
		if(r < 0) {
			fprintf(stderr, "read(): %s\n", strerror(errno));
			exit(1);
		} else if(r == 0) {
			break;
		} else {
			write(1, buf, r);
			for(int i=0; i<r; i++) {
				handle_char(bip, buf[i]);
			}
		}
	}
}


static void handle_char(struct bip *bip, char c)
{
	if(c == '\n' || c == '\r') {
		bip->line[bip->len] = '\0';
		handle_line(bip);
		bip->len = 0;
	} else {
		if(bip->len < sizeof(bip->line)-1) {
			bip->line[bip->len] = c;
			bip->len ++;
		}
	}
}


static bool load_lua(struct bip *bip)
{
	struct stat st;
	int r = lstat(bip->fname_lua, &st);
	if(r < 0) {
		fprintf(stderr, "%s: %s\n", bip->fname_lua, strerror(errno));
		return false;
	}

	if(st.st_mtime == bip->mtime_lua) {
		return true;
	}

	bip->mtime_lua = st.st_mtime;

	r = luaL_loadfile(bip->L, bip->fname_lua);
	if(r != 0) {
                fprintf(stderr, "error: %s\n", lua_tostring(bip->L, -1));
                return false;
        }

	return call_lua(bip, 0);
}


static bool call_lua(struct bip *bip, int nargs)
{
	lua_pushcfunction(bip->L, l_traceback);
	int traceback_idx = 1;
	lua_insert(bip->L, traceback_idx);

	int r = lua_pcall(bip->L, nargs, 0, traceback_idx);
	if(r == 0) {
		return true;
	} else {
                fprintf(stderr, "error: %s\n", lua_tostring(bip->L, -1));
		l_traceback(bip->L);
		return false;
	}
}


static void handle_line(struct bip *bip)
{
	time_t t_now = time(NULL);

	if(t_now != bip->t_last_line) {
		load_lua(bip);
		bip->t_last_line = t_now;
	}

	lua_getglobal(bip->L, "on_line");
	lua_pushlstring(bip->L, bip->line, bip->len);

	SDL_LockAudioDevice(bip->adev);
	call_lua(bip, 1);
	SDL_UnlockAudioDevice(bip->adev);
}


static float clamp(float f, float min, float max)
{
	if(f < min) f = min;
	if(f > max) f = max;
	return f;
}


struct channel *alloc_channel(struct bip *bip, float gain, float pan)
{
	size_t t_max = 0;
	struct channel *rv = NULL;

	for(size_t i=0; i<CHANNELS; i++) {
		struct channel *ch = &bip->channel_list[i];
		if(!ch->busy) {
			rv = ch;
			break;
		}
		if(ch->t >= t_max) {
			t_max = ch->t;
			rv = ch;
		}
	}

	rv->gain[0] = clamp(gain - pan, -1.0, 1.0) / CHANNELS;
	rv->gain[1] = clamp(gain + pan, -1.0, 1.0) / CHANNELS;
	rv->t = 0.0;
	rv->window = 0.0;
	rv->busy = true;

	return rv;
}



static float window(struct channel *ch)
{
	float dw = 1.0 / (SRATE * 0.005);
	if(ch->t < ch->duration) {
		ch->window += dw;
	} else {
		ch->window -= dw;
	}
	ch->window = clamp(ch->window, 0.0, 1.0);
	return ch->window;
}


static void on_audio(void *userdata, uint8_t *stream, int len)
{
	struct bip *bip = userdata;
	float *sout = (float *)stream;

	memset(sout, 0, len);

	for(size_t i=0; i<CHANNELS; i++) {
		struct channel *ch = &bip->channel_list[i];
		for(size_t j=0; j<FRAGSIZE*2; j+=2) {
			if(ch->busy) {
				float w = window(ch);
				float v = osc_gen(&ch->osc);

				v = biquad_run(&ch->biquad, v);

				sout[j+0] += v * w * ch->gain[0];
				sout[j+1] += v * w * ch->gain[1];
				ch->t += 1.0/SRATE;

				if(w == 0.0) {
					ch->busy = false;
				}
			}
		}
	}
}




