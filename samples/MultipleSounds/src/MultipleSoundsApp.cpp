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
	void keyDown(KeyEvent event);
	
	unsigned int mMusicID;
	bool mPlay;
};

void MultipleSoundsApp::setup()
{
	mPlay = false;
}

void MultipleSoundsApp::mouseDown( MouseEvent event )
{
	mPlay = !mPlay;
	
	if (mPlay) {
		DataSourceRef source = loadAsset("tune.wav");
		mMusicID = po::SoundManager::get()->play(source, 0, true);
	} else {
		po::SoundManager::get()->stop(mMusicID);
	}
}

void MultipleSoundsApp::update()
{
	po::SoundManager::get()->update();
}

void MultipleSoundsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

void MultipleSoundsApp::keyDown(cinder::app::KeyEvent event)
{
	if (event.getChar() == '1') {
		DataSourceRef source = app::loadAsset("kaching.wav");
		unsigned int trackID = po::SoundManager::get()->play(source);
		po::SoundManager::get()->setPan(trackID, 0);
	}
	
	else if (event.getChar() == '2') {
		DataSourceRef source = app::loadAsset("blip.wav");
		unsigned int trackID = po::SoundManager::get()->play(source);
		po::SoundManager::get()->setPan(trackID, 0.25);
	}
	
	else if (event.getChar() == '3') {
		DataSourceRef source = app::loadAsset("buzz.wav");
		unsigned int trackID = po::SoundManager::get()->play(source);
		po::SoundManager::get()->setPan(trackID, 0.75);
	}
	
	else if (event.getChar() == '4') {
		DataSourceRef source = app::loadAsset("bloop.wav");
		unsigned int trackID = po::SoundManager::get()->play(source);
		po::SoundManager::get()->setPan(trackID, 1);
	}
}

CINDER_APP_NATIVE( MultipleSoundsApp, RendererGl )
