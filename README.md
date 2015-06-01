# Cinder-poSoundManager

[Potion's](http://potiondesign.com) simple sound manager block for [Cinder](http://libcinder.org).

The block consists of a single class, **SoundManager**, that provides basic sound playback control.

## Features

* Play, loop a track created with a datasource or audio buffer
* Stop a track, all sounds, or tracks in a group
* Track pan and gain
* Signal when a sound has finished playing
* Averaged volume when playing multiple tracks

## Samples

* MultipleSounds: plays multiple sounds on mouse down and key presses

## Getting Started

Playing a sound:

```C++
DataSourceRef source = loadAsset("tune.wav");
po::SoundManager::get()->play(source);
```

Stopping a looped sound:

```C++
DataSourceRef source = loadAsset("tune.wav");
mTrackID = po::SoundManager::get()->play(source, 0, true); // looped

po::SoundManager::get()->stop(mTrackID);
```
