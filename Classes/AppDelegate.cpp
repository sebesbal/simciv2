#include "stdafx.h"

#include "AppDelegate.h"
#include "world_ui.h"
#include "indgen_ui.h"

USING_NS_CC;

SimCivApp::SimCivApp(const std::string& config, const std::string& tmx_map): m_config_file(config), m_tmx_map_file(tmx_map) {

}

SimCivApp::~SimCivApp() 
{
}

//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void SimCivApp::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

bool SimCivApp::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
	auto size = Size(1600, 1000);
	//auto size = Size(1200, 1024);
	//auto size = director->getWinSizeInPixels();

    if(!glview) {
        //glview = GLViewImpl::createWithRect("simciv", Rect(0, 0, 960, 640));
		glview = GLViewImpl::createWithRect("simciv", Rect(0, 0, size.width, size.height));
		//glview = GLViewImpl::createWithFullScreen("simciv");
		//glview = GLViewImpl::create("simciv");
        director->setOpenGLView(glview);
    }

	director->getOpenGLView()->setDesignResolutionSize(size.width, size.height, ResolutionPolicy::SHOW_ALL);

    // turn on display FPS
    // director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60.0f);

    FileUtils::getInstance()->addSearchPath("res");

    // create a scene. it's an autorelease object
	auto scene = simciv::WorldUI::createScene(m_config_file, m_tmx_map_file);
	//auto scene = indgen::IndGenUI::createScene();

    // run
    director->runWithScene(scene);

    return true;
}
 
// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void SimCivApp::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void SimCivApp::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
