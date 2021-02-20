#ifndef AUDIO
#define AUDIO

#include "audio.h"
#include "cross-platform.h"
#include "entity.h"

#include <stdlib.h>


Sound::Sound(AudioContext* newcontext, string filename): context(newcontext) {
	string dataname = "resources/sounds/" + filename;
	ifstream ifile(dataname);
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
				wavfiles.push_back(path);
			} else if (varname == "files") {
				string path;
				char lett;
				ifile >> lett >> path;
				while (path.back() != ']') {
					wavfiles.push_back(path);
					ifile >> path;
				}
				if (path != "]") {
					path.pop_back();
					wavfiles.push_back(path);
				}
			} else if (varname == "dir") {
				string dirpath;
				ifile >> dirpath;
				vector<string> newfiles;
				get_files_folder("resources/sounds/" + dirpath, &newfiles);
				for (string path : newfiles) {
					wavfiles.push_back(dirpath + '/' + path);
				}
			}
      getline(ifile, buff, ':');
      getline(ifile, varname, ':');
    }
		
		if (wavfiles.size() == 0) {
			wavfiles.push_back(name + ".wav");
		}
		
		duration = 0;
		num_buffers = wavfiles.size();
		
		buffers = new ALuint[num_buffers];
		alGenBuffers(num_buffers, buffers);
		
		for (int i = 0; i < num_buffers; i ++) {
			string fullname = "resources/sounds/" + wavfiles[i];
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
	start_time = glfwGetTime();
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








AudioContext::AudioContext() {
	init_context();
	load_sounds();
	cout << AL_REFERENCE_DISTANCE << ' ' << AL_MAX_DISTANCE << ' ' << AL_ROLLOFF_FACTOR << endl;
}

void AudioContext::init_context() {
	
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

void AudioContext::load_sounds() {
	vector<string> paths;
	get_files_folder("resources/sounds", &paths);
	for (string path : paths) {
		if (path.find(".txt") != string::npos) {
			Sound* sound = new Sound(this, path);
			library[sound->name] = sound;
		}
	}
	cout << "loaded in " << paths.size() << " sound files " << endl;
}

PlayingSound* AudioContext::play_sound(string name, vec3 pos, float gain, float pitch) {
	if (device != nullptr and name != "null" and listener != nullptr and glm::length(pos - listener->position) < library[name]->range) {
		PlayingSound* newsound = new PlayingSound(library[name], pos);
		newsound->gain(gain);
		newsound->pitch(pitch);
		play_onetime(newsound);
		return newsound;
	}
	return nullptr;
}

void AudioContext::play_onetime(PlayingSound* sound) {
	if (device != nullptr) {
		sound->play();
		onetime_sounds.push_back(sound);
	}
}

void AudioContext::play_repeat(PlayingSound* sound) {
	if (device != nullptr) {
		sound->play();
		repeat_sounds.push_back(sound);
	}
}

void AudioContext::tick() {
	if (device != nullptr) {
		check_sounds();
		if (listener != nullptr) {
			vec3 direction (
				cos(listener->angle.y) * sin(listener->angle.x),
				sin(listener->angle.y),
				cos(listener->angle.y) * cos(listener->angle.x)
			);
			vec3 right (
				sin(listener->angle.x - 3.14f/2.0f),
				0,
				cos(listener->angle.x - 3.14f/2.0f)
			);
			vec3 up = glm::cross(right, direction);
			ALfloat listenerOri[] = { direction.x, direction.y, direction.z, up.x, up.y, up.z };
			alListener3f(AL_POSITION, listener->position.x, listener->position.y, listener->position.z); geterr();
			alListener3f(AL_VELOCITY, listener->vel.x, listener->vel.y, listener->vel.z); geterr();
			alListenerfv(AL_ORIENTATION, listenerOri); geterr();
		}
	}
}

void AudioContext::check_sounds() {
	if (device != nullptr) {
		double time = glfwGetTime();
		for (int i = onetime_sounds.size() - 1; i >= 0; i --) {
			PlayingSound* sound = onetime_sounds[i];
			if (sound->start_time + sound->sound->duration < time) {
				delete sound;
				onetime_sounds.erase(onetime_sounds.begin() + i);
			}
		}
	}
}

void AudioContext::status(stringstream& debugstream) {
	debugstream << "----- audio status --------" << endl;
	for (PlayingSound* sound : onetime_sounds) {
		debugstream << sound->sound->name << ' ';
	}
	debugstream << endl;
	debugstream << onetime_sounds.size() << " sounds playing" << endl;
}

void AudioContext::geterr() {
	ALCenum error;
	error = alGetError();
	if (error != AL_NO_ERROR) {
		cout << "ERR: openal error " << error << endl;
	}
}


AudioContext::~AudioContext() {
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

#endif
