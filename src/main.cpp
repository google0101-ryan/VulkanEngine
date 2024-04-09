#include <app/Application.h>

int main()
{
	HelloTriangleApplication app;

	try
	{
		app.Init();
	} catch(std::exception& e)
	{
		printf("ERROR: %s\n", e.what());
		return 1;
	}

	app.Run();

	return 0;
}