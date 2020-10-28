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
	
	AudioContext();
	~AudioContext();
	
	void init_context();
	void load_sounds();
	
	PlayingSound* play_sound(string name, vec3 pos, float gain = 1, float pitch = 1);
	void play_onetime(PlayingSound* sound);
	void play_repeat(PlayingSound* sound);
	
	void tick();
	void check_sounds();
	
	void status(stringstream& debugstream);
	void geterr();
};



#endif
