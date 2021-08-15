#ifndef BASE_AUDIO_H
#define BASE_AUDIO_H

#include "classes.h"
#include "plugins.h"

class AudioContext { public:
	PLUGIN_HEAD(AudioContext, ());
	
	virtual ~AudioContext() {};
	
	virtual void init_context() = 0;
	virtual void load_sounds() = 0;
	
	virtual void set_listener(vec3* pos, vec3* vel, vec2* pointing) = 0;
	
	virtual void play_sound(string name, vec3 pos, float gain = 1, float pitch = 1) = 0;
	// virtual void play_onetime(PlayingSound* sound) = 0;
	// virtual void play_repeat(PlayingSound* sound) = 0;
	
	virtual void tick() = 0;
	virtual void check_sounds() = 0;
	
	virtual void status(stringstream& debugstream) = 0;
	virtual void geterr() = 0;
};

extern Plugin<AudioContext> audio;

#endif
