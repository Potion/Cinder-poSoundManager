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
		ci::DataSourceRef source = ci::app::loadAsset("kaching.wav");
		ci::audio::SourceFileRef sourceFile = ci::audio::load(source);
		ci::audio::BufferRef music = sourceFile->loadBuffer();
		mSoundManager->play(music);
	}
	
	else if (event.getChar() == '2') {
		ci::DataSourceRef source = ci::app::loadAsset("blip.wav");
		ci::audio::SourceFileRef sourceFile = ci::audio::load(source);
		ci::audio::BufferRef music = sourceFile->loadBuffer();
		mSoundManager->play(music);
	}
}

CINDER_APP_NATIVE( MultipleSoundsApp, RendererGl )
