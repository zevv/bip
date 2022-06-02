
#include "api.h"
#include "bip.h"


void init_api(lua_State *L)
{
	lua_pushcfunction(L, l_bip); lua_setglobal(L, "bip");
	lua_pushcfunction(L, l_hash); lua_setglobal(L, "hash");
}


int l_traceback(lua_State *L)
{
	fprintf(stderr, ">>> ");
        lua_getglobal(L, "debug");
        lua_getfield(L, -1, "traceback");
        lua_pushvalue(L, 1);
        lua_pushinteger(L, 2);
        lua_call(L, 2, 1);
        return 1;
}


int l_bip(lua_State *L)
{
	float freq = 0;
	float gain = 0;
	float duration = 0;
	float pan = 0;
	float filter = 0;
	float dutycycle = 0.5;
	enum osc_type osc_type = OSC_TYPE_SIN;
	
	lua_getfield(L, LUA_REGISTRYINDEX, "bip");
	struct bip *bip = lua_touserdata(L, -1);
	lua_pop(L, 1);

	if(lua_istable(L, 1)) {
		const char *typename;
		lua_getfield(L, 1, "freq"); freq = luaL_optnumber(L, -1, 1000);
		lua_getfield(L, 1, "duration"); duration = luaL_optnumber(L, -1, 0.01);
		lua_getfield(L, 1, "gain"); gain = luaL_optnumber(L, -1, bip->gain);
		lua_getfield(L, 1, "pan"); pan = luaL_optnumber(L, -1, bip->pan);
		lua_getfield(L, 1, "filter"); filter = luaL_optnumber(L, -1, 1.0);
		lua_getfield(L, 1, "type"); typename = luaL_optstring(L, -1, "sin");
		lua_getfield(L, 1, "dc"); dutycycle = luaL_optnumber(L, -1, 0.5);

		if(strncmp(typename, "saw", 3) == 0) osc_type = OSC_TYPE_SAW;
		if(strncmp(typename, "pul", 3) == 0) osc_type = OSC_TYPE_PULSE;
		if(strncmp(typename, "tri", 3) == 0) osc_type = OSC_TYPE_TRIANGLE;;

	} else {
		freq = luaL_optnumber(L, 1, 1000);
		duration = luaL_optnumber(L, 2, 0.01);
		gain = luaL_optnumber(L, 3, 1.0);
		pan = luaL_optnumber(L, 4, 0.0);
		filter = luaL_optnumber(L, 5, 1.0);
	}

	// Allocate channel

	struct channel *ch = alloc_channel(bip, gain, pan);
	if(ch == NULL) {
		return 0;
	}
	ch->duration = duration;

	// Setup osc and filter

	osc_set_type(&ch->osc, osc_type);
	osc_set_freq(&ch->osc, freq);
	osc_set_dutycycle(&ch->osc, dutycycle);

	biquad_init(&ch->biquad, SRATE);
	biquad_config(&ch->biquad, BIQUAD_TYPE_LP, freq * filter, 1.0);

	return 0;
}


int l_hash(lua_State *L)
{
	size_t len;
	const char *buf = luaL_checklstring(L, 1, &len);

	uint8_t a = 0;
	uint8_t b = 0;

	for(size_t i=0; i<len; i++) {
		a += (uint8_t)buf[i];
		b += a;
	}
	
	lua_pushinteger(L, (a << 8) | b);
	return 1;
}

