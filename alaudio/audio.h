#ifndef AUDIO_PREDEF
#define AUDIO_PREDEF

#include "scrolls/classes.h"
#include "scrolls/audio.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <map>
using std::map;

class Sound { public:
	AudioContext* context;
	string name;
	ALuint* buffers;
	int num_buffers;
	double duration;
	double range = 30;
	Sound(AudioContext* newcontext, string filepath);
	~Sound();
	ALuint get_buffer();
};

class PlayingSound { public:
	AudioContext* context;
	Sound* sound;
	ALuint source;
	double start_time;
	PlayingSound(Sound* newsound, vec3 pos, bool repeat = false);
	~PlayingSound();
	void play();
	void position(vec3 newpos);
	void vel(vec3 newvel);
	void pitch(float newval);
	void gain(float newval);
	void repeat(bool newval);
};

class ALAudioContext : public AudioContext { public:
	ALCdevice* device;
	ALCcontext* context;
	
	vec3* listenerpos;
	vec3* listenervel;
	vec2* listenerdir;
	
	map<string,Sound*> library;
	vector<PlayingSound*> onetime_sounds;
	vector<PlayingSound*> repeat_sounds;
	
	ALuint source;
	ALuint buffer;
	
	ALAudioContext();
	virtual ~ALAudioContext();
	
	virtual void init_context();
	virtual void load_sounds();
	
	virtual void set_listener(vec3* pos, vec3* vel, vec2* dir);
	
	virtual void play_sound(string name, vec3 pos, float gain = 1, float pitch = 1);
	virtual void play_onetime(PlayingSound* sound);
	virtual void play_repeat(PlayingSound* sound);
	
	virtual void tick();
	virtual void check_sounds();
	
	virtual void status(stringstream& debugstream);
	virtual void geterr();
};

class AudioNullContext : public AudioContext { public:
	AudioNullContext() {}
	virtual ~AudioNullContext() {}
	
	virtual void init_context() {}
	virtual void load_sounds() {}
	
	virtual void play_sound(string name, vec3 pos, float gain = 1, float pitch = 1) {}
	// virtual void play_onetime(PlayingSound* sound) {}
	// virtual void play_repeat(PlayingSound* sound) {}
	
	virtual void tick() {}
	virtual void check_sounds() {}
	
	virtual void status(stringstream& debugstream) {}
};



#endif
