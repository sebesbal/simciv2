#include "stdafx.h"

#include "main.h"
#include "AppDelegate.h"

USING_NS_CC;

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// create the application instance
	//SimCivApp app("res/mod4.xml", "res/simciv2.tmx");
	SimCivApp app("res/mod5.xml", "res/simciv2.tmx");
    return Application::getInstance()->run();
}
