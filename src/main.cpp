
#include <SDL.h>

#define GLES_SILENCE_DEPRECATION
#include <OpenGLES/ES3/gl.h>

namespace {
void printProgramLog(GLuint f_programId) {
  if (glIsProgram(f_programId)) {
    int logLen = 0;
    glGetProgramiv(f_programId, GL_INFO_LOG_LENGTH, &logLen);

    char* infoLog_a = new char[logLen];
    int infoLogLen = 0;
    glGetProgramInfoLog(f_programId, logLen, &infoLogLen, infoLog_a);

    SDL_Log("%s\n", infoLog_a);
    delete[] infoLog_a;
  }
}

void printShaderLog(GLuint f_shaderId) {
  if (glIsShader(f_shaderId)) {
    int logLen = 0;
    glGetShaderiv(f_shaderId, GL_INFO_LOG_LENGTH, &logLen);

    char* infoLog_a = new char[logLen];
    int infoLogLen = 0;
    glGetShaderInfoLog(f_shaderId, logLen, &infoLogLen, infoLog_a);

    SDL_Log("%s\n", infoLog_a);
    delete[] infoLog_a;
  }
}

GLuint loadShader(const GLchar* f_source_p, GLenum f_type) {
  GLuint shaderId = glCreateShader(f_type);
  glShaderSource(shaderId, 1, &f_source_p, nullptr);
  glCompileShader(shaderId);

  GLint compileStatus = GL_FALSE;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

  if (!compileStatus) {
    printShaderLog(shaderId);
    glDeleteShader(shaderId);
    shaderId = 0;
  }

  return shaderId;
}

GLuint loadProgram(const GLchar* f_vertSource_p, const GLchar* f_fragSource_p) {
  GLuint vertShader = loadShader(f_vertSource_p, GL_VERTEX_SHADER);
  GLuint fragShader = loadShader(f_fragSource_p, GL_FRAGMENT_SHADER);

  if (!glIsShader(vertShader) || !glIsShader(fragShader)) {
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return 0;
  }

  GLuint programId = glCreateProgram();
  glAttachShader(programId, vertShader);
  glAttachShader(programId, fragShader);

  glLinkProgram(programId);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);

  if (!linkStatus) {
    printProgramLog(programId);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    glDeleteProgram(programId);
    return 0;
  }

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
  return programId;
}
}  // namespace

int main(int, char**) {
  // Init SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    SDL_Log("%s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_version version;
  SDL_GetVersion(&version);
  SDL_Log("SDL version: %d.%d.%d\n", static_cast<int>(version.major), static_cast<int>(version.minor), static_cast<int>(version.patch));
  
  // Create window
  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  // Explicitly set channel depths, otherwise we might get some < 8
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  const auto windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS;
  auto window = SDL_CreateWindow(nullptr, 0, 0, 0, 0, windowFlags);
  if (!window) {
    SDL_Log("%s\n", SDL_GetError());
      return EXIT_FAILURE;
  }

  // Init GL
  auto glContext = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, glContext);
  SDL_Log("GL version: %s\n", glGetString(GL_VERSION));
    
  // Set viewport
  int w = 0;
  int h = 0;
  SDL_GL_GetDrawableSize(SDL_GL_GetCurrentWindow(), &w, &h);
  glViewport(0, 0, w, h);

  // Load shader program
  constexpr char kVS[] = R"(
    #version 300 es
    layout(location = 0) in vec2 a_position;
    layout(location = 1) in vec2 a_texCoords;
    out vec2 v_texCoords;
    void main() {
      v_texCoords = a_texCoords;
      gl_Position = vec4(a_position, 0, 1);
    })";

  constexpr char kFS[] = R"(
    #version 300 es
    precision mediump float;
    in vec2 v_texCoords;
    out vec4 out_color;
    void main() {
      out_color = vec4(v_texCoords.x, v_texCoords.y, 0.0, 1.0);
    })";

  auto program = loadProgram(kVS, kFS);

  // Main loop
  bool isRunning = true;
  while (isRunning) {
    SDL_Event event;
    while (0 != SDL_PollEvent(&event)) {
      if (SDL_QUIT == event.type) {
        isRunning = false;
      }
    }
    
    // Clear
    glClearColor(0.2F, 0.2F, 0.2F, 1.F);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render scene
    const GLfloat coords[] = { 0.f, 0.5f, -1.f, -0.5f, 1.f, -0.5f };
    glUseProgram(program);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, coords);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, coords);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Update window
    SDL_GL_SwapWindow(window);
  }

  // Clean up
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
