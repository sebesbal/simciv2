#include "HelloWorldScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

using namespace cocostudio::timeline;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto rootNode = CSLoader::createNode("MainScene.csb"); 

    addChild(rootNode);

	


	//TMXTiledMap* map = (TMXTiledMap*)rootNode->getChildByName("Map_1");
	//TMXLayer* terrain = map->getLayer("terrain");
	//uint32_t t = terrain->tileGIDAt(Vec2(0, 0));
	//t = terrain->tileGIDAt(Vec2(1, 0));
	//t = terrain->tileGIDAt(Vec2(2, 0));
	//t = terrain->tileGIDAt(Vec2(0, 1));
	//t = terrain->tileGIDAt(Vec2(1, 1));
	//t = terrain->tileGIDAt(Vec2(2, 1));
	//auto animals = map->getObjectGroup("animals");
	//auto kutya = animals->getObject("kutya");
	//animals->
	//auto k = kutya.at("miez");

	
    return true;
}
