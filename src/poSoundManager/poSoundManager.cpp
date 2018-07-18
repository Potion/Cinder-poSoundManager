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

#include "poSoundManager.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"

namespace po
{
	SoundManagerRef SoundManager::Instance = nullptr;

	SoundManagerRef SoundManager::get()
	{
		if( !Instance ) {
			Instance = std::shared_ptr<SoundManager>( new SoundManager() );
			Instance->setup();
		}

		return Instance;
	}

	SoundManager::SoundManager()
		: mTrackID( 0 )
		, mAverageVolume( 0.f )
		, mSilentMode( false )
		, mManualGainMode( false )
	{}

	void SoundManager::setup()
	{
		auto context = ci::audio::Context::master();
		mMasterGain = context->makeNode( new ci::audio::GainNode( 1.0 ) );
		context->enable();
	}

	void SoundManager::setSilentMode( bool silent )
	{
		mSilentMode = silent;

		float gain = mSilentMode ? 0.f : 1.f;
		mMasterGain->setValue( gain );
	}

	void SoundManager::update()
	{
		// clear out tracks that are done playing
		int numTracksPlaying = 0;

		for( auto thisTrack = mTracks.begin(); thisTrack != mTracks.end(); ) {
			TrackRef t = thisTrack->second;

			bool bRemove = true;

			if( t != nullptr && !t->isFinished() )  {
				numTracksPlaying++;
				++thisTrack;
				bRemove = false;
			}

			if( bRemove ) {
				// Remove reference to track
				onFinishedPlaying( thisTrack->first );
				t->disconnect();
				mGroup.erase( thisTrack->first );
				thisTrack = mTracks.erase( thisTrack );
			}
		}

		// Adjust gain if auto gain is on (on by default)
		if( !mManualGainMode ) {

			mAverageVolume = mSilentMode ? 0 : MAX_VOLUME / numTracksPlaying;

			for( auto thisTrack = mTracks.begin(); thisTrack != mTracks.end(); ++thisTrack ) {
				TrackRef t = thisTrack->second;

				if( t->gain->getParam()->getValue() < 0.f ) {
					t->gain->getParam()->setValue( 0.f ); // Making sure that minimum is 0
				}

				t->gain->getParam()->applyRamp( mAverageVolume, RAMP_TIME, ci::audio::Param::Options().rampFn( &ci::audio::rampInQuad ) );
			}
		}
	}

	unsigned int SoundManager::play( ci::DataSourceRef ref, unsigned int group, bool loop )
	{
		ci::audio::SourceFileRef sourceFile = ci::audio::SourceFile::create( ref );
		ci::audio::BufferRef buffer = sourceFile->loadBuffer();
		return play( buffer, group, loop );
	}

	unsigned int SoundManager::load( ci::DataSourceRef dataSource, unsigned int group, bool loop  )
	{
		ci::audio::SourceFileRef sourceFile = ci::audio::SourceFile::create( dataSource );
		ci::audio::BufferRef buffer = sourceFile->loadBuffer();

		auto trackId = load( buffer, group, loop );
		return trackId;
	}


	unsigned int SoundManager::load( ci::audio::BufferRef buffer, unsigned int group, bool loop  )
	{
		auto context = ci::audio::Context::master();

		//  Create Buffer Player
		ci::audio::BufferPlayerNodeRef bufferPlayer = context->makeNode( new ci::audio::BufferPlayerNode( buffer ) );
		bufferPlayer->setLoopEnabled( loop );

		//  Create Track
		TrackRef t( new Track( bufferPlayer ) );

		//  Connect track w/Master Gain Node
		t->connect( mMasterGain );

		//  Use the same gain as the group
		float volume = getGroupGain( group );

		//  Save track
		mGroup[mTrackID]    = group;
		mTracks[mTrackID]   = t;

		//  Increase track count and return prev id
		unsigned int currentTrackID = mTrackID;
		mTrackID++;
		return currentTrackID;
	}

	unsigned int SoundManager::play( ci::audio::BufferRef buffer, unsigned int group, bool loop )
	{
		//	Load track
		auto trackId = load( buffer, group, loop );

		//  Save track
		auto track = mTracks[trackId];

		//  Start track
		track->gain->getParam()->setValue( getGroupGain( group ) );
		track->bufferPlayer->start();

		return trackId;
	}

	void SoundManager::play( unsigned int trackID, bool bLoop )
	{
		if( trackExists( trackID ) ) {
			mTracks[trackID]->bufferPlayer->setLoopEnabled( bLoop );
			mTracks[trackID]->bufferPlayer->start();
		}
	}

	void SoundManager::stop( unsigned int trackID )
	{
		if( trackExists( trackID ) ) {
			mTracks[trackID]->bufferPlayer->stop();
			mTracks[trackID]->bufferPlayer->setLoopEnabled( false );
		}
	}

	void SoundManager::stopAllInGroup( unsigned int group )
	{
		for( auto thisGroup = mGroup.begin(); thisGroup != mGroup.end(); ++thisGroup ) {
			unsigned int g = thisGroup->second;

			if( g == group ) {
				unsigned int trackID = thisGroup->first;
				stop( trackID );
			}
		}
	}

	void SoundManager::stopAll()
	{
		for( auto thisTrack = mTracks.begin(); thisTrack != mTracks.end(); ++thisTrack ) {
			thisTrack->second->bufferPlayer->stop();
		}
	}

	void SoundManager::removeAllTracks()
	{
		stopAll();

		for( auto thisTrack = mTracks.begin(); thisTrack != mTracks.end(); ) {
			//  Remove reference to track
			TrackRef t = thisTrack->second;
			onFinishedPlaying( thisTrack->first );
			t->disconnect();
			mGroup.erase( thisTrack->first );
			thisTrack = mTracks.erase( thisTrack );
		}

		mGroup.empty();
		mTracks.empty();
	}

	//  Has the track completed?
	bool SoundManager::isSoundFinishedPlaying( unsigned int trackID )
	{
		return !trackExists( trackID );
	}

	//  Check whether or not we know about the track still
	bool SoundManager::trackExists( unsigned int trackID )
	{
		bool exists = ( mTracks.find( trackID ) != mTracks.end() );

		if( exists ) {
			return mTracks[trackID] != nullptr;
		}

		return false;
	}

	//	Send a message when a track has finished playing
	void SoundManager::onFinishedPlaying( unsigned int trackID )
	{
		mSoundFinishedPlaying.emit( trackID );
	}

	//	Set the track gain, ramps volume up and down
	void SoundManager::setGain( unsigned int trackID, float volume )
	{
		if( mTracks.find( trackID ) != mTracks.end() ) {
			mTracks[trackID]->gain->getParam()->applyRamp( volume, RAMP_TIME, ci::audio::Param::Options().rampFn( &ci::audio::rampInQuad ) );
		}
	}

	float SoundManager::getGain( unsigned int trackID )
	{
		if( mTracks.find( trackID ) != mTracks.end() ) {
			float gain = mTracks[trackID]->gain->getParam()->getValue();
			return gain;
		}
		else {
			return 0.f;
		}
	}

	void SoundManager::setGroupGain( unsigned int groupID, float volume )
	{
		for( auto thisGroup = mGroup.begin(); thisGroup != mGroup.end(); ++thisGroup ) {
			unsigned int g = thisGroup->second;

			if( g == groupID ) {
				unsigned int trackID = thisGroup->first;
				setGain( trackID, volume );
			}
		}
	}

	float SoundManager::getGroupGain( unsigned int groupID )
	{
		if( mGroup.size() == 0 ) { return 1.f; }

		float volumeSum = 0.f;

		for( auto thisGroup = mGroup.begin(); thisGroup != mGroup.end(); ++thisGroup ) {
			unsigned int g = thisGroup->second;

			if( g == groupID ) {
				unsigned int trackID = thisGroup->first;
				volumeSum += getGain( trackID );
			}
		}

		return volumeSum / ( float )mGroup.size();
	}

	void SoundManager::setPan( unsigned int trackID, float pan )
	{
		if( mTracks.find( trackID ) != mTracks.end() ) {
			mTracks[trackID]->pan->setPos( pan );
		}
	}

}
