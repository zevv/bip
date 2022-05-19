
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include <SDL.h>

#define MAX_CHANNELS 16
#define FRAGSIZE 256
#define SRATE 48000

enum channel_type {
	CHANNEL_TYPE_IDLE,
	CHANNEL_TYPE_BIP,
};

struct channel;

typedef float (*generator)(struct channel *channel);

struct channel {
	int id;
	enum channel_type type;
	float t;
	float duration;
	float gain[2];
	generator fn_gen;

	// BIP
	float bip_freq;
};


struct bip {
	SDL_AudioDeviceID adev;
	struct channel channel_list[MAX_CHANNELS];
};



void handle_line(struct bip *bip, const char *l);
struct channel *find_channel(struct bip *bip);
void play_bip(struct bip *bip, float freq, float duration);
static void on_audio(void *userdata, uint8_t *stream, int len);

int main(int argc, char **argv)
{
	struct bip *bip = calloc(1, sizeof(struct bip));
	for(size_t i=0; i<MAX_CHANNELS; i++) {
		struct channel *ch = &bip->channel_list[i];
		ch->id = i;
		ch->type = CHANNEL_TYPE_IDLE;
	}

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

	printf("%d\n", have.freq);

	SDL_PauseAudioDevice(bip->adev, 0);

	char buf[256];
	while(fgets(buf, sizeof(buf), stdin) != NULL) {
		SDL_LockAudioDevice(bip->adev);
		handle_line(bip, buf);
		SDL_UnlockAudioDevice(bip->adev);
	}

	SDL_CloseAudioDevice(bip->adev);
	
	free(bip);

	return 0;
}


void handle_line(struct bip *bip, const char *buf)
{
	printf("> %s", buf);
	int l = strlen(buf) % 60;
	float freq = 110 * powf(1.05946309, l);
	play_bip(bip, freq, 0.05);
}


struct channel *find_channel(struct bip *bip)
{
	size_t t_max = 0;
	struct channel *rv = NULL;

	for(size_t i=0; i<MAX_CHANNELS; i++) {
		struct channel *ch = &bip->channel_list[i];
		if(ch->type == CHANNEL_TYPE_IDLE) {
			rv = ch;
			break;
		}
		if(ch->t >= t_max) {
			t_max = ch->t;
			rv = ch;
		}
	}

	return rv;
}


float gen_idle(struct channel *ch)
{
	return 0.0;
}


float gen_bip(struct channel *ch)
{
	if(ch->t >= ch->duration) {
		ch->type = CHANNEL_TYPE_IDLE;
	}

	return cos(ch->t * ch->bip_freq * M_PI * 2);
}


void play_bip(struct bip *bip, float freq, float duration)
{
	struct channel *ch = find_channel(bip);
	ch->gain[0] = 0.5 / MAX_CHANNELS;
	ch->gain[1] = 0.5 / MAX_CHANNELS;
	ch->t = 0.0;
	ch->type = CHANNEL_TYPE_BIP;
	ch->bip_freq = freq;
	ch->duration = duration;
	ch->fn_gen = gen_bip;
	//printf("ch %d: bip %f %f\n", ch->id, freq, duration);
}


static float window(struct channel *ch)
{
	if(ch->duration > 0) {
		return cos(M_PI * 2 * ch->t / ch->duration) * -0.5 + 0.5;
	} else {
		return 0;
	}
}


static void on_audio(void *userdata, uint8_t *stream, int len)
{
	struct bip *bip = userdata;
	float sout[FRAGSIZE * 2] = { 0 };

	for(size_t i=0; i<MAX_CHANNELS; i++) {
		struct channel *ch = &bip->channel_list[i];

		for(size_t j=0; j<FRAGSIZE*2; j+=2) {
			float v = 0;

			if(ch->type == CHANNEL_TYPE_BIP) {
				v = ch->fn_gen(ch);
			}

			float w = window(ch);

			sout[j+0] += v * w * ch->gain[0];
			sout[j+1] += v * w * ch->gain[1];
			ch->t += 1.0/SRATE;
		}

	}

	memcpy(stream, sout, len);

}
