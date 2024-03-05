#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include "opengl.h"

typedef struct Scene {
    GLuint VBO;
    GLuint VAO;
} Scene;

typedef struct View {
    int x;
    int y;
    int width;
    int height;
} View;

typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Color {
    GLfloat  r;
    GLfloat  g;
    GLfloat  b;
    GLfloat  a;
} Color;

typedef struct Matrix4 {
    float m[4][4];
} Matrix4;

typedef struct Vertex {
    Vector3 position;
   // Vector3 normal;
   // Vector2 texcoord;
} Vertex;


typedef struct Texture {
    unsigned int id;
    char* type;
} Texture;

typedef struct GpuData {
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;
    GLuint shaderProgram;
    unsigned int numIndicies;
} GpuData;

typedef struct Material {
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;
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
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Matrix4 transform;
} TransformComponent;

typedef struct GroupComponent {
    int active;
    Matrix4 transform;
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
