#include <iostream>
#include <thread>
#include "Music.hpp"
#include "AudioError.hpp"

Music::Music(Musics m, float loop)
  : source(AL_NONE), loopTime(loop), fade(false)
{
  alGenBuffers(2, buffers.data());
  setMusic(m, loop);
  alGenSources(1, &source);
  if (!Audio::checkError(false))
    {
      alDeleteBuffers(2, &source);
      ov_clear(&oggStream);
      throw AudioError("Couldn't generate a source in Music::Music");
    }
  alSource3f(source, AL_POSITION, 0., 0., 0.);
  alSource3f(source, AL_VELOCITY, 0., 0., 0.);
  alSource3f(source, AL_DIRECTION, 0., 0., 0.);
  alSourcef(source, AL_ROLLOFF_FACTOR, 0.);
  alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
  if (!Audio::checkError(false))
    {
      alDeleteSources(1, &source);
      ov_clear(&oggStream);
      alDeleteBuffers(1, &buffers[0]);
      throw AudioError("Couldn't delete a buffer in Music::Music");
    }
  std::cout << "ENDCTOR" << std::endl;
}

Music::~Music()
{
  releaseMusic();
  alDeleteBuffers(1, &buffers[0]);
  alDeleteSources(1, &source);
}

void Music::initMusic(Musics music, float loopTime)
{
  if (ov_fopen(Audio::getMusicFileName(music), &oggStream) < 0)
    throw AudioError("Could not open .ogg file");

  this->music = music;
  vorbisInfo = ov_info(&oggStream, -1);
  vorbisComment = ov_comment(&oggStream, -1);
  format = vorbisInfo->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  this->loopTime = loopTime;
}

void Music::releaseMusic(void)
{
  alSourceStop(source);
  unqueuePending();
  ov_clear(&oggStream);
}

void Music::setMusic(Musics music, float loopTime)
{
  std::lock_guard<std::mutex> guard(mutex);
  if (source != AL_NONE)
    {
      releaseMusic();
    }
  initMusic(music, loopTime);
}

void Music::play(void)
{
  if (!streamFile(buffers[0]) || !streamFile(buffers[1]))
    throw AudioError("Couldn't play music");
  std::cout << "PLAYINIT" << std::endl;
  alSourceQueueBuffers(source, 2, buffers.data());
  alSourcePlay(source);
  std::cout << "PLAYINITEND" << std::endl;
}

void Music::setLoopTime(float f)
{
  loopTime = f;
}

void Music::setVolume(float f) const
{
  alSourcef(source, AL_GAIN, f);
  Audio::checkError();
}

float Music::getVolume(void) const
{
  ALfloat gain;

  alGetSourcef(source, AL_GAIN, &gain);
  return gain;
}

void Music::setFade(bool b)
{
  fade = b;
}

bool Music::isPlaying(void) const
{
  ALenum state;

  alGetSourcei(source, AL_SOURCE_STATE, &state);
  return state == AL_PLAYING;
}

void Music::update(void)
{
  ALuint buffer[1];
  int processed(AL_NONE);
  bool active(true);
  mutex.lock();
  Musics first(this->music);
  mutex.unlock();

  if (fade)
    setVolume(getVolume() / 1.1f);
  alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
  if (processed)
    {
      alSourceUnqueueBuffers(source, 1, buffer);
      Audio::checkError();
      active = streamFile(buffer[0]);
	{
	  std::lock_guard<std::mutex> guard(mutex);
	  if (first != this->music)
	    return;
	}
      alSourceQueueBuffers(source, 1, buffer);
      Audio::checkError();
      if (!active)
	ov_time_seek(&oggStream, loopTime);
    }
}

bool Music::streamFile(ALuint buffer)
{
  char data[BUFFER_SIZE];
  unsigned int size(0);
  Musics first(this->music);

  while (size < BUFFER_SIZE)
    {
	{
	  std::lock_guard<std::mutex> guard(mutex);

	  if (first != this->music)
	    return true;
	}
      int size_read(ov_read(&oggStream, data + size, BUFFER_SIZE - size, 0, 2, 1, 0));
      size += size_read;
      if (size_read <= 0)
	return false;
    }
  alBufferData(buffer, format, data, size, vorbisInfo->rate);
  Audio::checkError();
  return true;
}

void Music::unqueuePending(void)
{
  ALuint buffer(AL_NONE);
  int queued(AL_NONE);

  alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
  while (queued--)
    {
      alSourceUnqueueBuffers(source, 1, &buffer);
      Audio::checkError();
    }
}
