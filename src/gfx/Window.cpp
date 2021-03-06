#include "Window.h"
#include "SDL.h"
#include "SDL_video.h"
using namespace gfx;

Window::Window() {

}

Window::~Window() {
	SDL_DestroyWindow(m_Window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	if (m_Initialized) {
		m_Initialized = false;
		SDL_GL_DeleteContext(m_GLContext);
	}
	m_Window = nullptr;
}

void Window::Initialize(const WindowSettings& windowSettings) {
	m_WindowSettings = windowSettings;
	if (m_Window) {
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}

	SDL_Init(SDL_INIT_EVERYTHING);

	int flags =
	    (m_WindowSettings.Fullscreen ? SDL_WINDOW_FULLSCREEN : 0) |
	    (m_WindowSettings.Resizeable ? SDL_WINDOW_RESIZABLE : 0) |
	    (m_WindowSettings.HighDPI ? SDL_WINDOW_ALLOW_HIGHDPI : 0) |
	    (m_WindowSettings.BorderLess ? SDL_WINDOW_BORDERLESS : 0) |
	    (m_WindowSettings.OpenGL ? SDL_WINDOW_OPENGL : 0);
	// Create an application window with the following settings:
	m_Window = SDL_CreateWindow(
	               m_WindowSettings.Title.c_str(),
	               m_WindowSettings.X == -1 ? SDL_WINDOWPOS_UNDEFINED : m_WindowSettings.X,
	               m_WindowSettings.Y == -1 ? SDL_WINDOWPOS_UNDEFINED : m_WindowSettings.Y,
	               m_WindowSettings.Width,
	               m_WindowSettings.Height,
	               flags
	           );

	if (m_Window == nullptr) {
		//Logger::Log("Could not create window: " + rString(SDL_GetError()), "Window", LogSeverity::ERROR_MSG);
		return;
	}
	//set up GL
	if (m_Initialized) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		m_Initialized = false;
		SDL_GL_DeleteContext(m_GLContext);
	}
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		//Logger::Log("Failed to initialize SDL video subsystem", "GraphicsEngine", LogSeverity::ERROR_MSG);
		return;
	}
	else {
		m_Initialized = true;
		m_GLContext = SDL_GL_CreateContext(m_Window);
		if (m_GLContext == nullptr) {
			//Logger::Log("Failed to create OpenGL context", "GraphicsEngine", LogSeverity::ERROR_MSG);
			return;
		}
	}
	//set up vsync
	if (m_WindowSettings.Vsync) {
		SDL_GL_SetSwapInterval(1);
	}
	else {
		SDL_GL_SetSwapInterval(0);
	}

}

SDL_Window* Window::GetWindow() const {
	return m_Window;
}

SDL_GLContext Window::GetContext() const {
	return m_GLContext;
}

const WindowSettings& Window::GetWindowSettings() const {
	return m_WindowSettings;
}

void Window::MakeCurrent() {
	if (SDL_GL_MakeCurrent(m_Window, m_GLContext) < 0)
		printf("Error makeing context current\n");
}
