/*
 Copyright (c) 2015, Potion Design LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 * Neither the name of copyright holder nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
		typedef ci::signals::Signal<void(unsigned int)> SoundFinishedPlayingSignal;
		
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
		
		//	Remove all tracks
		void removeAllTracks();
		
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
