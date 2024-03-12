#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include "opengl.h"

typedef struct View {
    int x;
    int y;
    int width;
    int height;
} View;

enum Viewport {
    VIEWPORT_MAIN = 0,
    VIEWPORT_UI = 1
};

typedef struct Color {
    GLfloat  r;
    GLfloat  g;
    GLfloat  b;
    GLfloat  a;
} Color;

typedef struct Vertex {
    vec3 position;
    vec3 color;
    vec2 texcoord;
   // Vector3 normal;
} Vertex;


typedef struct Texture {
    GLuint id;
    char* type;
} Texture;

typedef struct GpuData {
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;
    GLuint shaderProgram;
    GLuint numIndicies;
    GLenum drawMode;
} GpuData;

typedef struct Material {
    Color ambient;
    Color diffuse;
    Color specular;
    GLfloat shininess;
   /*  Texture* diffuseMap;
    Texture* specularMap;
    Texture* normalMap; */
} Material;




// ECS

enum Tag {
    UNINITIALIZED = 0,
    MODEL = 1
};

typedef struct TransformComponent {
    int active;
    int isDirty;
    vec3 position;
    vec3 rotation;
    vec3 scale;
    mat4x4 transform;
} TransformComponent;

typedef struct GroupComponent {
    int active;
    mat4x4 transform;
} GroupComponent;

typedef struct MeshComponent {
    int active;
    int drawIndexed;
    Vertex* vertices;
    size_t vertexCount;
    unsigned int* indices;
    size_t indexCount;
    GpuData* gpuData;
} MeshComponent;

typedef struct UIComponent {
    int active;
} UIComponent;

typedef struct MaterialComponent {
    int active;
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;
   // Texture* diffuseMap;
  //  Texture* specularMap;
  //  Texture* normalMap;
} MaterialComponent;

typedef struct Entity {
    int id;
    int alive;
    int tag;
    TransformComponent* transformComponent;
    GroupComponent* groupComponent;
    MeshComponent* meshComponent;
    MaterialComponent* materialComponent;
    UIComponent* uiComponent;
} Entity;






#endif // TYPES_H
