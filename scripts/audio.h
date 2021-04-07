#ifndef AUDIO_PREDEF
#define AUDIO_PREDEF

#include "classes.h"

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

class AudioContext { public:
	ALCdevice* device;
	ALCcontext* context;
	
	Entity* listener = nullptr;
	
	map<string,Sound*> library;
	vector<PlayingSound*> onetime_sounds;
	vector<PlayingSound*> repeat_sounds;
	
	ALuint source;
	ALuint buffer;
	
	virtual ~AudioContext() {};
	
	virtual void init_context() = 0;
	virtual void load_sounds() = 0;
	
	virtual PlayingSound* play_sound(string name, vec3 pos, float gain = 1, float pitch = 1) = 0;
	virtual void play_onetime(PlayingSound* sound) = 0;
	virtual void play_repeat(PlayingSound* sound) = 0;
	
	virtual void tick() = 0;
	virtual void check_sounds() = 0;
	
	virtual void status(stringstream& debugstream) = 0;
	void geterr();
};

class AudioMainContext : public AudioContext { public:
	AudioMainContext();
	virtual ~AudioMainContext();
	
	virtual void init_context();
	virtual void load_sounds();
	
	virtual PlayingSound* play_sound(string name, vec3 pos, float gain = 1, float pitch = 1);
	virtual void play_onetime(PlayingSound* sound);
	virtual void play_repeat(PlayingSound* sound);
	
	virtual void tick();
	virtual void check_sounds();
	
	virtual void status(stringstream& debugstream);
};

class AudioNullContext : public AudioContext { public:
	AudioNullContext() {}
	virtual ~AudioNullContext() {}
	
	virtual void init_context() {}
	virtual void load_sounds() {}
	
	virtual PlayingSound* play_sound(string name, vec3 pos, float gain = 1, float pitch = 1) {return nullptr;}
	virtual void play_onetime(PlayingSound* sound) {}
	virtual void play_repeat(PlayingSound* sound) {}
	
	virtual void tick() {}
	virtual void check_sounds() {}
	
	virtual void status(stringstream& debugstream) {}
};



#endif
