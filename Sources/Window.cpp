
#include "pch.h"

#include "mkr/Window.h"

void UpdateInputStateKeyboard(int, int, int, int);
void UpdateInputStateMouse(int, int, int);
void UpdateInputMousePosition(double, double);
void UpdateInputMouseWheel(double, double);

static void window_close_callback(GLFWwindow* window) {
    printf("\nBuh bye\n");
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
    printf("\rWidth %d - Height %d\n", width, height);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    printf("\rFramebuffer (%d, %d)\n", width, height);
    //glViewport(0, 0, width, height);
}

static void window_pos_callback(GLFWwindow* window, int xpos, int ypos) {
    printf("\rWindow at (%d, %d)\n", xpos, ypos);
}

static void window_focus_callback(GLFWwindow* window, int focused) {
    if (focused) {
        printf("\nHi\n");
    }
    else {
        printf("\nBye\n");
    }
}

static void window_refresh_callback(GLFWwindow* window) {
    printf("Something to do here\n\n");
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_REPEAT) return; 
    UpdateInputStateKeyboard(key, scancode, action, mods);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    UpdateInputStateMouse(button, action, mods);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    UpdateInputMousePosition(xpos, ypos);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    UpdateInputMouseWheel(xoffset, yoffset);
}

static void SetWindowHints() {
    // Context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    
    // Window
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE,                    GL_TRUE);
    glfwWindowHint(GLFW_DECORATED,                  GL_TRUE);
    glfwWindowHint(GLFW_FOCUSED,                    GL_TRUE);
    glfwWindowHint(GLFW_FLOATING,                   GL_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED,                  GL_FALSE);
    glfwWindowHint(GLFW_CENTER_CURSOR,              GL_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER,    GL_FALSE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW,              GL_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR,           GL_TRUE);

    // Framebuffer
    glfwWindowHint(GLFW_RED_BITS,       8);
    glfwWindowHint(GLFW_GREEN_BITS,     8);
    glfwWindowHint(GLFW_BLUE_BITS,      8);
    glfwWindowHint(GLFW_ALPHA_BITS,     8);
    glfwWindowHint(GLFW_DEPTH_BITS,     24);
    glfwWindowHint(GLFW_STENCIL_BITS,   8);
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
}

static void SetWindowCallbacks(GLFWwindow* const window) {
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowPosCallback(window, window_pos_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetWindowRefreshCallback(window, window_refresh_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

static void SetWindowCursor(GLFWwindow* const window) {
    const int W = 9; // image width
    const int H = W; // image height
    unsigned char pixels[W * H * 4];
    memset(pixels, 0xff, sizeof(pixels));

    int middle_pixel = (H >> 1) * (W + 1) * 4;
    
    int left_pixel = middle_pixel - 4;
    int right_pixel = middle_pixel + 4;
    int up_pixel = middle_pixel - (W + 1) * 4 + 4;
    int down_pixel = middle_pixel + (W + 1) * 4 - 4;
    
    int left_left_pixel = left_pixel - 4;
    int right_right_pixel = right_pixel + 4;
    int up_up_pixel = up_pixel - (W + 1) * 4 + 4;
    int down_down_pixel = down_pixel + (W + 1) * 4 - 4;
    
    int up_right_pixel = up_pixel + 4;
    int up_left_pixel = up_pixel - 4;
    int down_right_pixel = down_pixel + 4;
    int down_left_pixel = down_pixel - 4;
    
    pixels[middle_pixel + 3]      = 0;
    pixels[left_pixel + 3]        = 0;
    pixels[right_pixel + 3]       = 0;
    pixels[up_pixel + 3]          = 0;
    pixels[down_pixel + 3]        = 0;
    pixels[left_left_pixel + 3]   = 0;
    pixels[right_right_pixel + 3] = 0;
    pixels[up_up_pixel + 3]       = 0;
    pixels[down_down_pixel + 3]   = 0;
    pixels[up_right_pixel + 3]    = 0;
    pixels[up_left_pixel + 3]     = 0;
    pixels[down_right_pixel + 3]  = 0;
    pixels[down_left_pixel + 3]   = 0;

    GLFWimage image;
    image.width = W;
    image.height = H;
    image.pixels = pixels;

    GLFWcursor* cursor = glfwCreateCursor(&image, W>>1, H>>1);

    glfwSetCursor(window, cursor);
}

GLFWwindow* WindowInit() {
    GLFWwindow* window;
    
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return nullptr;
    }
    
    SetWindowHints();
    
    glm::vec2 window_size = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    GLFWmonitor* PrimaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* video_mode = glfwGetVideoMode(PrimaryMonitor);
    printf("Video mode:\n");
    printf("  Width: %d\n", video_mode->width);
    printf("  Height: %d\n", video_mode->height);
    printf("  RGB depth: [%d, %d, %d]\n", video_mode->redBits, video_mode->greenBits, video_mode->blueBits);
    printf("  Refresh rate: %d\n", video_mode->refreshRate); 
    window = glfwCreateWindow(window_size.x, window_size.y, "Window", nullptr/*PrimaryMonitor*/, nullptr);
    
    if (window) {
        glfwSetWindowPos(window,
                         video_mode->width*0.5f - window_size.x*0.5f,
                         video_mode->height*0.5f - window_size.y*0.5f);
        
        glfwMakeContextCurrent(window);
        
        glewExperimental = true;
        glewInit();
        
        int glfwMajor, glfwMinor, glfwRevision;
        glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
        printf("GLFW(%d.%d.%d):\n", glfwMajor, glfwMinor, glfwRevision);
        
        const unsigned char* glVersion = glGetString(GL_VERSION);
        printf("  OpenGL Version: %s\n", glVersion);
        
        const unsigned char* renderer = glGetString(GL_RENDERER);
        printf("  Renderer: %s\n", renderer);
        
        int vertex_units, fragment_units;
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &vertex_units);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &fragment_units);
        printf("  Vertex Texture Units: %d\n", vertex_units);
        printf("  Fragment Texture Units: %d\n", fragment_units);

        GLint maxTextureUnits;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
        printf("  Max Texture Units: %d\n", maxTextureUnits);
        
        GLint maxUniforms;
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxUniforms);
        printf("  Max Uniforms: %d\n", maxUniforms);
        
        GLint maxUBOSize = 0;
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUBOSize);
        printf("  Max Uniform Buffer Size: %d (%d KB)\n", maxUBOSize, maxUBOSize / 1024);
        
        GLint mmaxUniformBindings;
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &mmaxUniformBindings);
        printf("  Max Uniform Binding Points: %d\n", mmaxUniformBindings);
        
        GLint maxShaderStorageBufferBindings;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxShaderStorageBufferBindings);
        printf("  Max SSBO Binding Points: %d\n", maxShaderStorageBufferBindings);
        
        GLint maxTextureSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
        printf("  Max Texture Size: %d\n", maxTextureSize);
        
        GLint maxVertexTextureImageUnits;
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxVertexTextureImageUnits);
        printf("  Max Vertex Texture Image Units: %d\n", maxVertexTextureImageUnits);

        GLint maxFragmentTextureImageUnits;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxFragmentTextureImageUnits);
        printf("  Max Fragment Texture Image Units: %d\n", maxFragmentTextureImageUnits);

        GLint maxCombinedTextureImageUnits;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureImageUnits);
        printf("  Max Combined Texture Image Units: %d\n", maxCombinedTextureImageUnits);
        
        GLint maxRenderbufferSize;
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
        printf("  Max Renderbuffer Size: %d\n", maxRenderbufferSize);
        
        GLint maxVertexAttribs;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
        printf("  Max Vertex Attributes: %d\n", maxVertexAttribs);
        
        GLint maxFragmentOutputs;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxFragmentOutputs);
        printf("  Max Draw Buffers: %d\n", maxFragmentOutputs);
        
        GLint maxPatch;
        glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatch);
        printf("  Max Patch Vertices: %d\n", maxPatch);
        
        GLint tessSupport = 0;
        glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &tessSupport);
        printf("  Max Tessellation Level: %d\n", tessSupport);
        
        GLint maxTextureBufferSize;
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTextureBufferSize);
        printf("  Max Texture Buffer Size: %d (%d MB)\n",
                maxTextureBufferSize, maxTextureBufferSize / 1048576);
       
        GLint maxCombinedBlocks = 0;
        glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &maxCombinedBlocks);
        printf("  Max Combined Shader Storage Blocks: %d\n", maxCombinedBlocks);
        
        GLint maxBlockSize;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxBlockSize);
        printf("  Max Shader Storage Block Size: %d (%d MB)\n",
                maxBlockSize, maxBlockSize / 1048576);
        
        GLint maxWorkGroupCount[3];
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupCount[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWorkGroupCount[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWorkGroupCount[2]);
        printf("  Max compute work group count: [%d, %d, %d]\n",
                maxWorkGroupCount[0], maxWorkGroupCount[1], maxWorkGroupCount[2]);
        
        GLint maxWorkGroupSize[3];
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxWorkGroupSize[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxWorkGroupSize[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxWorkGroupSize[2]);
        printf("  Max compute work group size: [%d, %d, %d]\n",
                maxWorkGroupSize[0], maxWorkGroupSize[1], maxWorkGroupSize[2]);
        
        GLint maxWorkGroupInvocations;
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxWorkGroupInvocations);
        printf("  Max compute work group invocations: %d\n", maxWorkGroupInvocations);
        
        GLint maxSharedMemorySize;
        glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSharedMemorySize);
        printf("  Max shared memory size: %d (%d KB)\n", maxSharedMemorySize, maxSharedMemorySize / 1024);
        
        //GLint nExt;
        //glGetIntegerv(GL_NUM_EXTENSIONS, &nExt);
        //for (int i = 0; i < nExt; i++) {
        //    const char* name = (const char*)glGetStringi(GL_EXTENSIONS, i);
        //    if (strcmp(name, "GL_KHR_shader_subgroup") == 0) {
        //        printf("  Extension GL_KHR_shader_subgroup is supported\n");
        //    }
        //}
        printf("---------------------------------------------------------------\n");
    }
    
    SetWindowCallbacks(window);
    SetWindowCursor(window);
    
    HWND hwnd = glfwGetWin32Window(window);
    HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1)); // See file 'Siac.rc'
    if (icon) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    }
    
    return window;
}
