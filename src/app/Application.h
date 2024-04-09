#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <frontend/frontend.h>

class HelloTriangleApplication
{
public:
	virtual ~HelloTriangleApplication() {Cleanup();}

	void Init();
	void Run();
private:
	void MainLoop();
	void Cleanup();
private:
	RenderFrontend frontend;
};