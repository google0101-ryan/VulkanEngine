#include "Application.h"

void HelloTriangleApplication::Init()
{
	frontend.InitFrontend(false);
}

void HelloTriangleApplication::Run()
{
	MainLoop();
}

void HelloTriangleApplication::MainLoop()
{
	while (!frontend.ShouldExit())
	{
		frontend.DrawView();
	}
}

void HelloTriangleApplication::Cleanup()
{

}
