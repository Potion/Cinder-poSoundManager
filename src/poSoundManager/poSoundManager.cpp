//
//  SoundManager.cpp
//  AudioTest
//
//  Created by bruce on 8/6/14.
//
//

#include "poSoundManager.h"

#include "cinder/Rand.h"
#include "cinder/app/App.h"

namespace po {
	
	SoundManagerRef SoundManager::Instance = nullptr;
	
	SoundManagerRef SoundManager::get()
	{
		if (!Instance) {
			Instance = std::shared_ptr<SoundManager>(new SoundManager());
			Instance->setup();
		}
		
		return Instance;
	}
	
	SoundManager::SoundManager()
	: mTrackID(0)
	, mAverageVolume(0.f)
	, mScatterMin(50.0f)
	, mScatterMax(150.0f)
    , mSilentMode(false)
	{}
	
	void SoundManager::setup()
	{
		auto context = ci::audio::Context::master();
        mMasterGain = context->makeNode(new ci::audio::GainNode(1.0));
        context->enable();
	}
    
    
    void SoundManager::setSilentMode(bool silent)
    {
        mSilentMode = silent;
        
        float gain = mSilentMode ? 0.f : 1.f;
        mMasterGain->setValue(gain);
    }
	
    
	void SoundManager::update()
	{
		// clear out tracks that are done playing
		int numTracksPlaying = 0;
        
		for (auto thisTrack = mTracks.begin(); thisTrack != mTracks.end();) {
            TrackRef t = thisTrack->second;
            
			if (t!= nullptr && t->bufferPlayer->isEnabled())  {
				numTracksPlaying++;
				++thisTrack;
			} else {
                //  Remove reference to track
				onFinishedPlaying(thisTrack->first);
                t->disconnect();

                mGroup.erase(thisTrack->first);
				thisTrack = mTracks.erase(thisTrack);
			}
		}
		
		// adjust gain
		mAverageVolume = mSilentMode ? 0 : MAX_VOLUME / numTracksPlaying;

		for (auto thisTrack = mTracks.begin(); thisTrack != mTracks.end(); ++thisTrack) {
			TrackRef t = thisTrack->second;
            t->gain->getParam()->applyRamp(mAverageVolume, RAMP_TIME, ci::audio::Param::Options().rampFn(&ci::audio::rampInQuad));
		}
		
	}
	
    
    unsigned int SoundManager::play(ci::DataSourceRef ref, unsigned int group, bool loop, bool scatter)
	{
        ci::audio::SourceFileRef sourceFile = ci::audio::SourceFile::create(ref);
        ci::audio::BufferRef buffer = sourceFile->loadBuffer();
        
		return play(buffer, group, loop, scatter);
	}
	
    
    unsigned int SoundManager::play(ci::audio::BufferRef buffer, unsigned int group, bool loop, bool scatter)
    {
        auto context = ci::audio::Context::master();
        
        //  Create Buffer Player
        ci::audio::BufferPlayerNodeRef bufferPlayer = context->makeNode(new ci::audio::BufferPlayerNode(buffer));
        
        if (loop) {
            bufferPlayer->setLoopEnabled();
        }
        
        //  Create Track
        TrackRef t(new Track(bufferPlayer));
        
        //  Set Pan and Scatter
        //  TODO: This needs to be readdressed
//        // Set random pan for scatter
//        float randomPan = ci::Rand::randFloat(1.0);
//        t->pan->setStereoInputModeEnabled();
//        
//        // Set random delay for scatter
//        float randomDelay = ci::Rand::randFloat(mScatterMin, mScatterMax);
//        t->delay->setDelaySeconds(randomDelay);
        
        //  Connect track w/Master Gain Node
        t->connect(mMasterGain);
        
        //Save track
        mGroup[mTrackID]    = group;
        mTracks[mTrackID]   = t;
        
        //  Start track
        t->bufferPlayer->start();
        
        //  Increase track count and return prev id
        unsigned int currentTrackID = mTrackID;
        mTrackID++;
        return currentTrackID;
    }
    
    
	void SoundManager::stop(unsigned int trackID)
	{
        if(trackExists(trackID)) {
            mTracks[trackID]->bufferPlayer->stop();
            mTracks[trackID]->bufferPlayer->setLoopEnabled(false);
        }
	}
    
    
    void SoundManager::stopAllInGroup(unsigned int group)
    {
        for (auto thisGroup = mGroup.begin(); thisGroup != mGroup.end(); ++thisGroup) {
            unsigned int g = thisGroup->second;
            if (g == group) {
                unsigned int trackID = thisGroup->first;
                stop(trackID);
            }
        }
    }
	
    
	void SoundManager::stopAll()
	{
		for (auto thisTrack = mTracks.begin(); thisTrack != mTracks.end(); ++thisTrack) {
            thisTrack->second->bufferPlayer->stop();
		}
	}
    
    
    //  Has the track completed?
    bool SoundManager::isSoundFinishedPlaying(unsigned int trackID)
    {
        return !trackExists(trackID);
    }
    
    
    //  Check whether or not we know about the track still
    bool SoundManager::trackExists(unsigned int trackID)
    {
        bool exists = (mTracks.find(trackID) != mTracks.end());
        
        if(exists) {
            return mTracks[trackID] != nullptr;
        }
        
        return false;
        
    }
	
    
	//
	//	Send a message when a track has finished playing
	//
	void SoundManager::onFinishedPlaying(unsigned int trackID)
	{
		//ci::app::console() << "SoundManager::onFinishedPlaying:: " << trackID << std::endl;
		mSoundFinishedPlaying(trackID);
	}
	
	//
	//	Set the track gain, ramps volume up and down
	//
	void SoundManager::setGain(unsigned int trackID, float volume)
	{
		if (mTracks.find(trackID) != mTracks.end()) {
			mTracks[trackID]->gain->getParam()->applyRamp(volume, RAMP_TIME, ci::audio::Param::Options().rampFn(&ci::audio::rampInQuad));
		}
	}
	
}