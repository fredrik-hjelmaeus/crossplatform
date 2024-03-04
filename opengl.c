#include "opengl.h"
#include "types.h"

Scene scene3D;
Scene sceneUI;
GpuBuffer myBuffer;

GLuint sceneShaderProgram, uiShaderProgram;

void setupMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, GpuBuffer* buffer) {
    printf("Setting up mesh\n");
    // Generate VAO(vertex array object) and VBO(vertex buffer object)
   /*  if(buffer == NULL) {
        printf("Buffer is null\n");
        return;
    } */
    glGenVertexArrays(1, &(buffer->VAO));
    glGenBuffers(1, &(buffer->VBO));
    glGenBuffers(1, &buffer->EBO);
    glBindVertexArray(buffer->VAO);

    // Bind/Activate VBO
    glBindBuffer(GL_ARRAY_BUFFER, buffer->VBO);

    // Copy vertices to buffer
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // Bind/Activate EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->EBO);

    // Copy indices to buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // This line tells OpenGL how to interpret the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

    // Enable the vertex attribute
    glEnableVertexAttribArray(0);

    // Unbind VBO/buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind VAO/vertex array
    glBindVertexArray(0);

}

void setupMaterial(GpuBuffer* buffer){
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
    buffer->shaderProgram = glCreateProgram();
    glAttachShader(buffer->shaderProgram, vertexShader);
    glAttachShader(buffer->shaderProgram, fragmentShader);
    glLinkProgram(buffer->shaderProgram);

    printf("Shader program: %d\n", buffer->shaderProgram);

    // Check for linking errors
    glGetProgramiv(buffer->shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(buffer->shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
}

void create_rectangle(Scene* scene) {
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
    uiShaderProgram  = glCreateProgram();
    glAttachShader(uiShaderProgram, vertexShader);
    glAttachShader(uiShaderProgram, fragmentShader);
    glLinkProgram(uiShaderProgram);

    // Check for linking errors
    glGetProgramiv(uiShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(uiShaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    // Set up vertex data and buffers
    GLfloat vertices[] = {
    // triangle one
    -1.0f, -1.0f, 0.0f,  // bottom left
     1.0f, -1.0f, 0.0f,  // bottom right
    -1.0f,  1.0f, 0.0f,  // top left

    // triangle two
    -1.0f,  1.0f, 0.0f,  // top left
     1.0f, -1.0f, 0.0f,  // bottom right
     1.0f,  1.0f, 0.0f   // top right
    };
   
    // Generate VAO(vertex array object) and VBO(vertex buffer object)
    glGenVertexArrays(1, &(scene->VAO));
    glGenBuffers(1, &(scene->VBO));
    glBindVertexArray(scene->VAO);

    // Bind/Activate VBO
    glBindBuffer(GL_ARRAY_BUFFER, scene->VBO);

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

void renderMesh(GpuBuffer* buffer) {
   
    glUseProgram(buffer->shaderProgram);

    // Set the color uniform
    GLint colorLocation = glGetUniformLocation(buffer->shaderProgram, "color");
    glUniform4f(colorLocation, 0.0f, 1.0f, 1.0f, 1.0f); 

    glBindVertexArray(buffer->VAO);
    glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}


// UI
void setup_ui() {
    create_rectangle(&sceneUI);
}


void render_ui() {
    glUseProgram(uiShaderProgram);

    // Set the color uniform
    GLint colorLocation = glGetUniformLocation(uiShaderProgram, "color");
    glUniform4f(colorLocation, 1.0f, 0.0f, 0.0f, 1.0f);  // Red color

    glBindVertexArray(sceneUI.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
