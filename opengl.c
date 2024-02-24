#include "opengl.h"

GLuint VBO, VAO;
GLuint shaderProgram;

void opengl_setup() {

#ifdef __EMSCRIPTEN__
    char* vertexShaderSource = loadShaderSource("shaders/wasm/vertex_wasm.glsl");
    char* fragmentShaderSource = loadShaderSource("shaders/wasm/fragment_wasm.glsl");
#else
    char* vertexShaderSource = loadShaderSource("shaders/vertex.glsl");
    char* fragmentShaderSource = loadShaderSource("shaders/fragment.glsl");
#endif

    if(fragmentShaderSource == NULL || vertexShaderSource == NULL) {
        printf("Error loading shader source\n");
        return;
    }
    
    printf("OpenGL ES version: %s\n", glGetString(GL_VERSION));

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if(vertexShader == 0) {
        printf("Error creating vertex shader\n");
        return;
    }
    glShaderSource(vertexShader, 1, (const GLchar* const*)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for shader compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(fragmentShader == 0) {
        printf("Error creating fragment shader\n");
        return;
    }
    glShaderSource(fragmentShader, 1, (const GLchar* const*)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // free memory of shader sources
    free(vertexShaderSource);
    free(fragmentShaderSource);

    // Link shaders
    shaderProgram  = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    // Set up vertex data and buffers
    GLfloat vertices[] = {
         0.0f,  0.5f, 0.0f,  // top
        -0.5f, -0.5f, 0.0f,  // bottom left
         0.5f, -0.5f, 0.0f   // bottom right
    };
   
    // Generate VAO(vertex array object) and VBO(vertex buffer object)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    // Bind/Activate VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Copy vertices to buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // This line tells OpenGL how to interpret the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

    // Enable the vertex attribute
    glEnableVertexAttribArray(0);

    // Unbind VBO/buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind VAO/vertex array
    glBindVertexArray(0);
}

void opengl_render() {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
