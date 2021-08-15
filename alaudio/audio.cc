#include "audio.h"
#include "base/cross-platform.h"
#include "base/libraries.h"

#include <stdlib.h>


Sound::Sound(AudioContext* newcontext, string filename): context(newcontext) {
	string parentdir = filename.substr(0, filename.find_last_of('/')) + '/';
	ifstream ifile(filename);
	vector<string> wavfiles;
	string buff;
  ifile >> buff;
  if (buff != "audio") {
    cout << "ERR: this file is not a audio data file " << endl;
  } else {
    ifile >> name;
    getline(ifile, buff, '{');
    string varname;
    getline(ifile, buff, ':');
    getline(ifile, varname, ':');
    while (!ifile.eof() and varname != "") {
      if (varname == "range") {
        ifile >> range;
      } else if (varname == "file") {
				string path;
				ifile >> path;
				wavfiles.push_back(PathLib::find_path("sounds/" + path));
			} else if (varname == "files") {
				string path;
				char lett;
				ifile >> lett >> path;
				while (path.back() != ']') {
					wavfiles.push_back(PathLib::find_path("sounds/" + path));
					ifile >> path;
				}
				if (path != "]") {
					path.pop_back();
					wavfiles.push_back(PathLib::find_path("sounds/" + path));
				}
			} else if (varname == "dir") {
				string dirpath;
				ifile >> dirpath;
				PathLib newfiles ("sounds/" + dirpath);
				for (string path : newfiles) {
					wavfiles.push_back(path);
				}
			}
      getline(ifile, buff, ':');
      getline(ifile, varname, ':');
    }
		
		if (wavfiles.size() == 0) {
			wavfiles.push_back(PathLib::find_path(name + ".wav"));
		}
		
		duration = 0;
		num_buffers = wavfiles.size();
		
		buffers = new ALuint[num_buffers];
		alGenBuffers(num_buffers, buffers);
		
		for (int i = 0; i < num_buffers; i ++) {
			string fullname = wavfiles[i];
			ALsizei size;
			ALfloat freq;
			ALenum format;
			
			ALvoid *data = alutLoadMemoryFromFile(fullname.c_str(), &format, &size, &freq);
			
			alBufferData(buffers[i], format, data, size, freq);
			double newdur = double(size) / freq;
			if (newdur > duration) {
				duration = newdur;
			}
			
			free(data);
		}
	}
}

Sound::~Sound() {
	alDeleteBuffers(num_buffers, buffers);
	delete[] buffers;
}

ALuint Sound::get_buffer() {
	return buffers[rand()%num_buffers];
}



PlayingSound::PlayingSound(Sound* newsound, vec3 pos, bool newrepeat): context(newsound->context), sound(newsound) {
	alGenSources((ALuint)1, &source);
	
	position(pos);
	repeat(newrepeat);
	
	alSourcei(source, AL_BUFFER, sound->get_buffer());
	alSourcef(source, AL_REFERENCE_DISTANCE, sound->range/4); context->geterr();
	alSourcef(source, AL_MAX_DISTANCE, sound->range); context->geterr();
}

PlayingSound::~PlayingSound() {
	alDeleteSources(1, &source);
}

void PlayingSound::play() {
	start_time = getTime();
	alSourcePlay(source);
}

void PlayingSound::position(vec3 newpos) {
	alSource3f(source, AL_POSITION, newpos.x, newpos.y, newpos.z); context->geterr();
}

void PlayingSound::vel(vec3 newvel) {
	alSource3f(source, AL_VELOCITY, newvel.x, newvel.y, newvel.z);
}

void PlayingSound::pitch(float newval) {
	alSourcef(source, AL_PITCH, newval);
}

void PlayingSound::gain(float newval) {
	alSourcef(source, AL_GAIN, newval);
}

void PlayingSound::repeat(bool newval) {
	alSourcei(source, AL_LOOPING, newval);
}








ALAudioContext::ALAudioContext() {
	init_context();
	load_sounds();
}

void ALAudioContext::set_listener(vec3* pos, vec3* vel, vec2* dir) {
	listenerpos = pos;
	listenervel = vel;
	listenerdir = dir;
}


void ALAudioContext::init_context() {
	
	device = alcOpenDevice(NULL);
	if (device != nullptr) {
	context = alcCreateContext(device, NULL);
		if (!alcMakeContextCurrent(context)) {
			cout << "ERR: openal context not made current" << endl;
		}
		
		alutInitWithoutContext(nullptr, nullptr);
		
		ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

		alListener3f(AL_POSITION, 0, 0, 1.0f);
		alListener3f(AL_VELOCITY, 0, 0, 0);
		alListenerfv(AL_ORIENTATION, listenerOri);
	}
}

void ALAudioContext::load_sounds() {
	PathLib paths("sounds");
	string xx = LibEntry("Sfsd", "sss");
	for (string path : paths) {
		if (path.find(".txt") != string::npos) {
			Sound* sound = new Sound(this, path);
			library[sound->name] = sound;
		}
	}
	cout << "loaded in " << paths.size() << " sound files " << endl;
}

void ALAudioContext::play_sound(string name, vec3 pos, float gain, float pitch) {
	if (device != nullptr and name != "null" and listenerpos != nullptr and glm::length(pos - *listenerpos) < library[name]->range) {
		PlayingSound* newsound = new PlayingSound(library[name], pos);
		newsound->gain(gain);
		newsound->pitch(pitch);
		play_onetime(newsound);
		//return newsound;
	}
	//return nullptr;
}

void ALAudioContext::play_onetime(PlayingSound* sound) {
	if (device != nullptr) {
		sound->play();
		onetime_sounds.push_back(sound);
	}
}

void ALAudioContext::play_repeat(PlayingSound* sound) {
	if (device != nullptr) {
		sound->play();
		repeat_sounds.push_back(sound);
	}
}

void ALAudioContext::tick() {
	if (device != nullptr) {
		check_sounds();
		if (listenerpos != nullptr) {
			vec3 direction (
				cos(listenerdir->y) * sin(listenerdir->x),
				sin(listenerdir->y),
				cos(listenerdir->y) * cos(listenerdir->x)
			);
			vec3 right (
				sin(listenerdir->x - 3.14f/2.0f),
				0,
				cos(listenerdir->x - 3.14f/2.0f)
			);
			vec3 up = glm::cross(right, direction);
			ALfloat listenerOri[] = { direction.x, direction.y, direction.z, up.x, up.y, up.z };
			alListener3f(AL_POSITION, listenerpos->x, listenerpos->y, listenerpos->z); geterr();
			alListener3f(AL_VELOCITY, listenervel->x, listenervel->y, listenervel->z); geterr();
			alListenerfv(AL_ORIENTATION, listenerOri); geterr();
		}
	}
}

void ALAudioContext::check_sounds() {
	if (device != nullptr) {
		double time = getTime();
		for (int i = onetime_sounds.size() - 1; i >= 0; i --) {
			PlayingSound* sound = onetime_sounds[i];
			if (sound->start_time + sound->sound->duration < time) {
				delete sound;
				onetime_sounds.erase(onetime_sounds.begin() + i);
			}
		}
	}
}

void ALAudioContext::status(stringstream& debugstream) {
	debugstream << "----- audio status --------" << endl;
	for (PlayingSound* sound : onetime_sounds) {
		debugstream << sound->sound->name << ' ';
	}
	debugstream << endl;
	debugstream << onetime_sounds.size() << " sounds playing" << endl;
}

void ALAudioContext::geterr() {
	ALCenum error;
	error = alGetError();
	if (error != AL_NO_ERROR) {
		cout << "ERR: openal error " << error << endl;
	}
}


ALAudioContext::~ALAudioContext() {
	for (PlayingSound* sound : onetime_sounds) {
		delete sound;
	}
	for (pair<string,Sound*> kv : library) {
		delete kv.second;
	}
	if (device != nullptr) {
		device = alcGetContextsDevice(context);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
}



EXPORT_PLUGIN(ALAudioContext);
