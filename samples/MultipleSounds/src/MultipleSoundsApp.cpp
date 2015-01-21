#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "poSoundManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MultipleSoundsApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void keyDown(cinder::app::KeyEvent event);
	
	unsigned int mMusicID;
	unsigned int mPanTrackID;
	bool mPlay;
	
	po::SoundManagerRef mSoundManager;
};

void MultipleSoundsApp::setup()
{
	mPlay = false;
	mSoundManager = po::SoundManager::get();
}

void MultipleSoundsApp::mouseDown( MouseEvent event )
{
	mPlay = !mPlay;
	
	if (mPlay) {
		ci::DataSourceRef source = ci::app::loadAsset("musicbed.wav");
		ci::audio::SourceFileRef sourceFile = ci::audio::load(source);
		ci::audio::BufferRef music = sourceFile->loadBuffer();
		mMusicID = mSoundManager->play(music, 0, true);
	} else {
		mSoundManager->stop(mMusicID);
	}
	
}

void MultipleSoundsApp::update()
{
	mSoundManager->update();
}

void MultipleSoundsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

void MultipleSoundsApp::keyDown(cinder::app::KeyEvent event)
{
	if (event.getChar() == '1') {
		ci::DataSourceRef source1 = ci::app::loadAsset("kaching.wav");
		ci::audio::SourceFileRef sourceFile1 = ci::audio::load(source1);
		ci::audio::BufferRef music1 = sourceFile1->loadBuffer();
		mSoundManager->play(music1);
	}
	
	else if (event.getChar() == '2') {
		ci::DataSourceRef source2 = ci::app::loadAsset("blip.wav");
		ci::audio::SourceFileRef sourceFile2 = ci::audio::load(source2);
		ci::audio::BufferRef music2 = sourceFile2->loadBuffer();
		mSoundManager->play(music2);
	}
	
	else if (event.getChar() == '3') {
		ci::DataSourceRef source3 = ci::app::loadAsset("kaching.wav");
		ci::audio::SourceFileRef sourceFile3 = ci::audio::load(source3);
		ci::audio::BufferRef music3 = sourceFile3->loadBuffer();
		mSoundManager->setPan(mSoundManager->play(music3), 0);
	}
	
	else if (event.getChar() == '4') {
		ci::DataSourceRef source4 = ci::app::loadAsset("blip.wav");
		ci::audio::SourceFileRef sourceFile4 = ci::audio::load(source4);
		ci::audio::BufferRef music4 = sourceFile4->loadBuffer();
		mSoundManager->setPan(mSoundManager->play(music4), 1);
	}
}

CINDER_APP_NATIVE( MultipleSoundsApp, RendererGl )
