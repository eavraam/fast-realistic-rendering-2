#include <GL/glew.h>
#include <GL/glut.h>
#include "Application.h"

#include "imgui.h"
#include "backends/imgui_impl_glut.h"
#include "backends/imgui_impl_opengl3.h"

//Remove console (only works in Visual Studio)
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")


// #define TIME_PER_FRAME 1000.f / 60.f // Approx. 60 fps

#define GLUT_SCROLL_UP      0x0003
#define GLUT_SCROLL_DOWN    0x0004

static int prevTime;
static Application app; // This object represents our whole app


// If a key is pressed this callback is called

static void keyboardDownCallback(unsigned char key, int x, int y)
{
	Application::instance().keyPressed(key);

	ImGui_ImplGLUT_KeyboardFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;
}

// If a key is released this callback is called

static void keyboardUpCallback(unsigned char key, int x, int y)
{
	Application::instance().keyReleased(key);
	
	ImGui_ImplGLUT_KeyboardUpFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;
}

// If a special key is pressed this callback is called

static void specialDownCallback(int key, int x, int y)
{
	Application::instance().specialKeyPressed(key);

	ImGui_ImplGLUT_SpecialFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;
}

// If a special key is released this callback is called

static void specialUpCallback(int key, int x, int y)
{
	Application::instance().specialKeyReleased(key);

	ImGui_ImplGLUT_SpecialFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;
}

// Same for changes in mouse cursor position

static void motionCallback(int x, int y)
{
	Application::instance().mouseMove(x, y);

	if (app.bNavigation) return;
    ImGui_ImplGLUT_MotionFunc(x, y);
}

// Same for mouse button presses or releases

static void mouseCallback(int button, int state, int x, int y)
{

	if (button == GLUT_SCROLL_UP) ImGui_ImplGLUT_MouseWheelFunc(button, 1, x, y);
    else if (button == GLUT_SCROLL_DOWN) ImGui_ImplGLUT_MouseWheelFunc(button, -1, x, y);
    else ImGui_ImplGLUT_MouseFunc(button, state, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

	int buttonId;
	
	switch(button)
	{
	case GLUT_LEFT_BUTTON:
		buttonId = 0;
		break;
	case GLUT_RIGHT_BUTTON:
		buttonId = 1;
		break;
	case GLUT_MIDDLE_BUTTON:
		buttonId = 2;
		break;
	}

		if(state == GLUT_DOWN)
			Application::instance().mousePress(buttonId);
		else if(state == GLUT_UP)
			Application::instance().mouseRelease(buttonId);
}

// Resizing the window calls this function

static void resizeCallback(int width, int height)
{
	ImGui_ImplGLUT_ReshapeFunc(width, height);
  	Application::instance().resize(width, height);
}

// Called whenever the windows needs to be redrawn
// This includes trying to render the scene at 60 FPS

static void drawCallback()
{
	// Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
	ImGui::NewFrame();

	// Render the application frame (including calls to Dear ImGui)
	Application::instance().render();
	
	// Render the Dear ImGui frame (with the calls to Dear ImGui that the applcation has made)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
	glutSwapBuffers();
}

// When there is no other event to be processed GLUT calls this function
// Here we update the scene and order GLUT to redraw the window every 
// 60th of a second, so that we get 60 FPS

static void idleCallback()
{
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = currentTime - prevTime;
	
	// if(deltaTime > TIME_PER_FRAME)
	// {
		// Every time we enter here is equivalent to a game loop execution
		if(!Application::instance().update(deltaTime))
			exit(0);
		prevTime = currentTime;
		glutPostRedisplay();
	// }
}


int main(int argc, char **argv)
{
	// GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(950, 750);
	glutCreateWindow(argv[0]);

	// Register all callbacks. GLUT will call the functions whenever 
	// the corresponding event is triggered
	glutReshapeFunc(resizeCallback);
	glutDisplayFunc(drawCallback);
	glutIdleFunc(idleCallback);
	glutKeyboardFunc(keyboardDownCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutSpecialFunc(specialDownCallback);
	glutSpecialUpFunc(specialUpCallback);
	glutMouseFunc(mouseCallback);
	glutPassiveMotionFunc(motionCallback);

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer backends
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init();

	// GLEW will take care of OpenGL extension functions
	glewExperimental = GL_TRUE;
	glewInit();
	
	// Application instance initialization
	Application::instance().init();
	if(argc > 1)
	  Application::instance().loadMesh(argv[1]);
	prevTime = glutGet(GLUT_ELAPSED_TIME);
	// GLUT gains control of the application
	glutMainLoop();

	// Dear ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

	return 0;
}



