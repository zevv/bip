#ifndef osc_h
#define osc_h


enum osc_type {
	OSC_TYPE_SIN,
	OSC_TYPE_SAW,
	OSC_TYPE_SAW_NAIVE,
	OSC_TYPE_TRIANGLE,
	OSC_TYPE_TRIANGLE_NAIVE,
	OSC_TYPE_PULSE,
	OSC_TYPE_PULSE_NAIVE,
};

struct osc {
	float phase;
	float srate_inv;
	float dphase;
	float dutycycle;
	float prev;
	enum osc_type type;
};

void osc_init(struct osc *osc, float srate);
void osc_set_freq(struct osc *osc, float freq);
void osc_set_type(struct osc *osc, enum osc_type type);
void osc_set_dutycycle(struct osc *osc, float dt);
float osc_gen(struct osc *osc);

#endif
