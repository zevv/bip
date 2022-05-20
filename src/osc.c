

#include <math.h>
#include <stdlib.h>

#include "sintab.h"
#include "osc.h"
#include "tabread.h"

/*
 * polyblep: http://www.kvraudio.com/forum/viewtopic.php?t=375517
 */

float poly_blep(float t, float dt)
{
	if (t < dt)
	{
		t /= dt;
		return t+t - t*t - 1.;
	} else if (t > 1. - dt)
	{
		t = (t - 1.) / dt;
		return t*t + t+t + 1.;
	} else {
		return 0.;
	}
}


void osc_init(struct osc *osc, float srate)
{
	osc->phase = 0;
	osc->srate_inv = 1.0/srate;
	osc_set_type(osc, OSC_TYPE_SIN);
	osc_set_dutycycle(osc, 0.5);
	osc_set_freq(osc, 440);
}


void osc_set_freq(struct osc *osc, float freq)
{
	osc->dphase = freq * osc->srate_inv;
}


void osc_set_type(struct osc *osc, enum osc_type type)
{
	osc->type = type;
}


void osc_set_dutycycle(struct osc *osc, float dt)
{
	osc->dutycycle = dt;
}



/*
 * Linear interpolation sine table lookup
 */

float osc_gen(struct osc *osc)
{
	float val = 0;

	if(osc->type == OSC_TYPE_SIN) {
		val = read2(sintab, SINTAB_SIZE, osc->phase);
	}
	
	if(osc->type == OSC_TYPE_PULSE_NAIVE) {
		val = osc->phase > 1.0-osc->dutycycle ? +1.0 : -1.0;
	}
	
	if(osc->type == OSC_TYPE_PULSE) {
		val = osc->phase > 1.0-osc->dutycycle ? +1.0 : -1.0;
		val += poly_blep(fmodf(osc->phase + osc->dutycycle, 1.0), osc->dphase);
		val -= poly_blep(osc->phase, osc->dphase);
	}

	if(osc->type == OSC_TYPE_TRIANGLE) {
		/* Integrated pulse, leaky integrator */
		val = osc->phase > 0.5 ? +1.0 : -1.0;
		val += poly_blep(fmodf(osc->phase + 0.5, 1.0), osc->dphase);
		val -= poly_blep(osc->phase, osc->dphase);
		val = osc->dphase * val + (1 - osc->dphase) * osc->prev;
		osc->prev = val * 0.99;
		val *= 4;
	}
	
	if(osc->type == OSC_TYPE_TRIANGLE_NAIVE) {
		val = -1.0 + 2.0 * osc->phase;
                val = 2.0 * fabsf(val) - 1;
	}
	
	if(osc->type == OSC_TYPE_SAW_NAIVE) {
		val = osc->phase * 2.0 - 1.0;
	}

	if(osc->type == OSC_TYPE_SAW) {
		val = osc->phase * 2.0 - 1.0;
		val -= poly_blep(osc->phase, osc->dphase);
	}

	osc->phase += osc->dphase;
	while(osc->phase < 0.0) {
		osc->phase += 1.0;
	}
	while(osc->phase >= 1.0) {
		osc->phase -= 1.0;
	}

	return val;
}


#ifdef TEST

#include <stdio.h>

int main(void)
{
	static struct osc osc, lfo, osc2;
	osc_init(&lfo, SRATE);

	osc_init(&osc, SRATE);
	osc_init(&osc2, SRATE);

	osc_set_freq(&lfo, 0.3);

	osc_set_dutycycle(&osc, 0.3);
	osc_set_dutycycle(&osc2, 0.3);

	osc_set_type(&osc, OSC_TYPE_PULSE);
	osc_set_freq(&osc, 1005);
	osc_set_type(&osc2, OSC_TYPE_SIN);
	osc_set_freq(&osc2, 1005);
	int i;

	for(i=0; i<480; i++) {
	
		float out, out2;
		out = osc_gen(&osc);
		out2 = osc_gen(&osc2);
		printf("%f %f\n", out, out2);
	}
}

#endif

/*
 * End
 */

