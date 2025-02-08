#ifndef TYPES_H
#define TYPES_H


#include <stdbool.h>
#include "opengl_types.h"

// Freetype
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct Entity Entity;

typedef struct Color {
    GLfloat  r;
    GLfloat  g;
    GLfloat  b;
    GLfloat  a;
} Color;

typedef struct Arena {
    size_t size;  // Total size of the memory block
    size_t used;  // Amount of memory already used
    void* base;   // Pointer to the base of the memory block
} Arena;

typedef enum {
    SPLIT_DEFAULT = 0,
    SPLIT_HORIZONTAL = 1,
    SPLIT_VERTICAL = 2
} SplitDirection;

typedef enum {
    UITYPE_NONE = 0,
    UITYPE_INPUT = 1,
    TEXTAREA = 2,
    UITYPE_TEXT = 3,
    UITYPE_BUTTON = 4,
    UITYPE_PANEL = 5,
    UITYPE_SLIDER = 6,
} UiType;

 typedef struct {
    unsigned int TextureID;  // ID handle of the glyph texture
    int Size[2];             // Size of glyph (width, height)
    int Bearing[2];          // Offset from baseline to left/top of glyph (x, y)
    unsigned int Advance;    // Offset to advance to next glyph
} Character;

typedef enum {
    CAMERAMODE_FPS = 0,
    CAMERAMODE_ORBITAL = 1,
} CameraMode;

typedef struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 target;
    mat4x4 view;
    mat4x4 projection;
    float speed;
    bool viewMatrixNeedsUpdate;
    bool projectionMatrixNeedsUpdate;
    CameraMode mode;

    // Perspective
    float fov;
    float near;
    float far;
    float aspectRatio;
    // Orthographic
    float left;   // Left boundary for orthographic projection
    float right;  // Right boundary for orthographic projection
    float bottom; // Bottom boundary for orthographic projection
    float top;    // Top boundary for orthographic projection
    bool isOrthographic; // Flag to indicate if the camera is orthographic
} Camera;

typedef struct BoundingBox {
    vec3 min;
    vec3 max;
} BoundingBox;

typedef struct Rectangle{
    int x;      // Rectangle left position (top-left or bottom-left corner)
    int y;      // Rectangle top position (top-left or bottom-left corner)
    int width;  // Rectangle width
    int height; // Rectangle height
} Rectangle;

// TODO: introduce concept of Layouts?
typedef struct View {
    Rectangle rect; // x left start corner, y top start corner, x right corner = x + width, y bottom corner = y + height
    Color clearColor;
    Camera* camera; // Pointer to a camera, can be NULL if no camera
    bool isMousePointerWithin; // Flag to indicate if the mouse pointer is within the view
} View;

typedef struct Views {
    View main;
    View ui;
    View full;
} Views;

enum Viewport {
    VIEWPORT_MAIN = 0,
    VIEWPORT_UI = 1
};

typedef struct Vertex {
    vec3 position;
    vec3 color;
    vec2 texcoord;
    vec3 normal;
} Vertex;

typedef struct Line {
    vec3 position;
} Line;

typedef struct TextureData {
    unsigned char* data;
    int width;
    int height;
    int channels;
} TextureData;

typedef struct Texture {
    GLuint id;
    char* type;
} Texture;

#define MATERIAL_DIFFUSEMAP_ENABLED   (1 << 0) // 0b00000001
#define MATERIAL_SPECULARMAP_ENABLED  (1 << 1) // 0b00000010
#define MATERIAL_AMBIENTMAP_ENABLED   (1 << 2) // 0b00000100
#define MATERIAL_SHININESSMAP_ENABLED (1 << 3) // 0b00001000
#define MATERIAL_BLINN_ENABLED        (1 << 4) // 0b00010000

typedef enum {
    DIFFUSEMAP_ENABLED   = 1 << 0,
    SPECULARMAP_ENABLED  = 1 << 1,
    AMBIENTMAP_ENABLED   = 1 << 2,
    SHININESSMAP_ENABLED = 1 << 3,
    BLINN_ENABLED =        1 << 4,
} Material_flags;

typedef struct Material {
    bool active;
    char* name;
    Color ambient;
    Color diffuse;
    Color specular;
    Color emissive;
    float shininess;
    float transparency;
    GLuint diffuseMap;
    GLfloat diffuseMapOpacity;
    GLuint specularMap;
    GLuint shininessMap;
    GLuint ambientMap;
    unsigned int material_flags;
  //  Texture* normalMap;
    float ior;
    GLuint alpha;
    bool isPostProcessMaterial;
} Material;

typedef struct GpuData {
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;
    GLuint FBO;
    GLuint RBO;
    GLuint shaderProgram;
    GLuint numIndicies;
    GLuint vertexCount;
    GLenum drawMode;
} GpuData;

// Data we get from obj-loader/parser.
// Child struct in ObjGroup.
typedef struct ObjData {
    char* name;
    int materialIndex; // index in globals.materials
    Vertex* vertexData;
    int num_of_vertices;
} ObjData;

// Data we get from obj-loader/parser. 
// Parent struct for ObjData.
typedef struct ObjGroup {
    const char* name;
    int objectCount;
    ObjData* objData;
} ObjGroup;

// Not used atm, but will be when we implement .obj group(g) support. 
// This is needed for sketchup obj exported files.
typedef struct ObjNode {
    char* name;
    struct ObjNode* children;
    int childCount;
    ObjData* objData;
} ObjNode;

// Used to instruct createMesh what type of data struct to setup.
typedef enum {
    VERTS_ONEUV = 0,
    VERTS_COLOR_ONEUV = 1,
    VERTS_COLOR_ONEUV_INDICIES = 2
} VertexDataType;

typedef void (*ButtonCallback)();
typedef void (*OnChangeCallback)();

typedef enum {
    SPOT = 0,
    DIRECTIONAL = 1,
    POINT = 2
} LightType;

// ECS

enum Tag {
    UNINITIALIZED = 0,
    MODEL = 1,
    BOUNDING_BOX = 2,
    TEXT = 3,
    LIGHT = 4,
};
typedef struct Vector2{
    float x;
    float y;
} Vector2;

typedef struct TransformComponent {
    bool active;
    bool modelNeedsUpdate;
    vec3 position;
    vec3 rotation;
    vec3 scale;
    mat4x4 transform;
} TransformComponent;

typedef struct GroupComponent {
    bool active;
    mat4x4 transform;
    //Entity* children
} GroupComponent;

typedef struct MeshComponent {
    bool active;
    bool drawIndexed;
    Vertex* vertices;
    size_t vertexCount;
    unsigned int* indices;
    size_t indexCount;
    GpuData* gpuData;
} MeshComponent;

// This is a line segment component atm. 
// If we want to draw a line with more than 2 points,
// we adapt this component (by adding vertices?) or create a separate component for that. TBD.
typedef struct LineComponent {
    bool active;
    vec3 start;
    vec3 end;
    //Vertex* vertices; 
    GpuData* gpuData;
    Color color;
    //int lineCount; see createPolyline 
    //vec3* linePositions; see createPolyline 
    // line width (See OpenGL_ES_3.0_Programming_Guide page 205)
    // line type, dotted (GL_LINE_STIPPLE )
    // line dotted pattern ( glLineStipple)
    // line smooth? (GL_LINE_SMOOTH) seems unsupported by oges3.0
} LineComponent;

typedef struct PointComponent {
    bool active;
    GLfloat* points;
    float pointSize;
    GpuData* gpuData;
    Color color;
} PointComponent;

#define MAX_UI_CHILDREN 20
typedef struct UIComponent {
    bool active;
    bool hovered;
    bool clicked;
    //Rectangle boundingBox;
    int boundingBoxEntityId;
    char* text;
    bool uiNeedsUpdate;
    ButtonCallback onClick;
    OnChangeCallback onChange;
    UiType type;
    Entity* parent;
    int childCount;
    int children[MAX_UI_CHILDREN];
    float sliderRange;
    int sliderRangeEntityId;
    float sliderValue;
    // padding?
    // margin?
    // offset?
    // border?
    // font?
    // fontSize?
    // textAlign
    // textOverflow?
    // opacity?
    // visibility?
}  UIComponent;

typedef struct LightComponent {
    bool active;
    vec3 direction;
    Color ambient;
    Color diffuse;
    Color specular;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
    LightType type;
} LightComponent;

typedef struct MaterialComponent {
    bool active;
    int materialIndex; // index in globals.materials
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;
    GLuint diffuseMap;
    GLfloat diffuseMapOpacity;
    GLuint specularMap;
    unsigned int material_flags;
    bool isPostProcessMaterial;
  //  Texture* normalMap;
} MaterialComponent;

typedef struct BoundingBoxComponent {
    bool active;
    BoundingBox boundingBox;
} BoundingBoxComponent;

typedef struct Entity {
    int id;
    bool alive;
    bool visible;
    int tag;
    TransformComponent* transformComponent;
    GroupComponent* groupComponent;
    MeshComponent* meshComponent;
    MaterialComponent* materialComponent;
    UIComponent* uiComponent;
    LightComponent* lightComponent;
    LineComponent* lineComponent;
    PointComponent* pointComponent;
    BoundingBoxComponent* boundingBoxComponent;
} Entity;

typedef vec3 Point;
typedef Vector2 SDLVector2;
typedef Vector2 UIVector2;

typedef struct ClosestLetter {
    SDLVector2 position;
    int characterIndex;
    float charWidth;
} ClosestLetter;

#endif // TYPES_H
