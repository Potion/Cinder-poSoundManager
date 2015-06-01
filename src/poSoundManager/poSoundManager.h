//
//  SoundManager.h
//  AudioTest
//
//  Created by bruce on 8/6/14.
//
//

#pragma once

#include <boost/signals2.hpp>
#include "cinder/audio/Context.h"
#include "cinder/audio/GainNode.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Param.h"
#include "cinder/audio/PanNode.h"
#include "cinder/audio/SamplePlayerNode.h"

namespace po {
	class SoundManager;
	typedef std::shared_ptr<SoundManager> SoundManagerRef;
	
	class SoundManager
	{
	public:
		static SoundManagerRef get();
		
		//	Sound finished playing signal
		typedef boost::signals2::signal<void(unsigned int)> SoundFinishedPlayingSignal;
		
		void update();
		
		//	Play data source
        unsigned int play(ci::DataSourceRef dataSource, unsigned int group=0, bool loop = false);
		
		//	Play audio buffer
        unsigned int play(ci::audio::BufferRef buffer, unsigned int group=0, bool loop = false);
		
		//	Stop
		void stop(unsigned int trackID);
		
		//	Stop all in specific group
        void stopAllInGroup(unsigned int group);
		
		//	Stop all sounds
		void stopAll();
		
		//	Silence the sounds
        void setSilentMode(bool silent);
		
		//	Track gain
		void setGain(unsigned int trackID, float volume);
		
		//	Track panning
		void setPan(unsigned int trackID, float pan);
		
		//	Check if a track finished playing
        bool isSoundFinishedPlaying(unsigned int trackID);
		SoundFinishedPlayingSignal &getSignalSoundFinishedPlaying() { return mSoundFinishedPlaying; }
		
	protected:
		SoundManager();
		
		void setup();
		
	private:
		//	Track data
        struct Track {
            ci::audio::BufferPlayerNodeRef bufferPlayer;
            ci::audio::GainNodeRef gain;
            ci::audio::MonitorNodeRef monitor;
            ci::audio::Pan2dNodeRef pan;
			
            Track(ci::audio::BufferPlayerNodeRef bufferPlayer)
            : bufferPlayer(bufferPlayer)
            {
                auto context = ci::audio::Context::master();
                
                gain    = context->makeNode(new ci::audio::GainNode(1.0));
                monitor = context->makeNode(new ci::audio::MonitorNode);
				pan     = context->makeNode(new ci::audio::Pan2dNode());
				pan->setStereoInputModeEnabled();
            }
            
            bool isFinished() {
                return (!bufferPlayer->isEnabled() && !bufferPlayer->isLoopEnabled());
            }
            
            void connect(ci::audio::GainNodeRef masterGain) {
                auto context = ci::audio::Context::master();
                bufferPlayer >> monitor >> gain >> pan >> masterGain >> context->getOutput();
            }
            
            void disconnect() {
                gain->disconnectAll();
                monitor->disconnectAll();
                pan->disconnectAll();
                bufferPlayer->disconnectAll();
            }
        };
        
        typedef std::shared_ptr<Track> TrackRef;
        
		static SoundManagerRef Instance;
		
		float MAX_VOLUME = 0.9f;
		const float RAMP_TIME = 0.1f;
		const float PAN_CENTER = 0.5f;
		
		unsigned int mTrackID;
        
        bool trackExists(unsigned int trackID);
        std::map<unsigned int, unsigned int> mGroup;
		std::map<unsigned int, TrackRef> mTracks;
        
		float mAverageVolume;
		ci::audio::GainNodeRef mMasterGain;
        
        bool mSilentMode;
		
		void onFinishedPlaying(unsigned int trackID);
	
		SoundFinishedPlayingSignal mSoundFinishedPlaying;
		
	};
	
}
