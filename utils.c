#include "utils.h"

char* readFile(const char *filename) {

    FILE *fp;

    // open file
    fp = fopen(filename, "r");

    // Check if this file is null
    if (fp == NULL) {
        printf("Could not open file %s", filename);
        return NULL;
    }

    // Find out the size of the file
    fseek(fp, 0, SEEK_END);

    // Store the size of the file
    long filesize = ftell(fp);

    // seek back to the beginning of the file
    fseek(fp, 0, SEEK_SET);

    // allocate memory based on filesize and type
    char* buffer = malloc(sizeof(char) * filesize + 1);
    
    // read the file into the buffer
    size_t result = fread(buffer, sizeof(char), filesize, fp);
    if(result != filesize) {
        printf(TEXT_COLOR_ERROR "Error reading file %s" TEXT_COLOR_RESET, filename);
        return NULL;
    }

    // Null terminate the buffer
    buffer[filesize] = '\0';

    fclose(fp);
    return buffer;
}



// Get a random number from 0 to 255
int randInt(int rmin, int rmax) {
    return rand() % rmax + rmin;
}
float randFloat(float rmin, float rmax) {
    return (float)rand() / (float)RAND_MAX * (rmax - rmin) + rmin;
}

unsigned char* loadImage(const char* filename, int* width, int* height, int* nrChannels){
    stbi_set_flip_vertically_on_load(1); 
    unsigned char* result = stbi_load(filename, width, height, nrChannels, 0);
    if(result == NULL) {
        
        printf(TEXT_COLOR_ERROR "Error loading image %s" TEXT_COLOR_RESET "\n", filename);
        return NULL;
    }
    return result;
}

float deg2rad(float degrees) {
    return degrees * M_PI / 180.0;
}

// Support for up to 30000 vertices only!. No indicies created.
// Collects vertices position(vArr),uv/texcoords(tArr) , vertex indices(vf) ,texture indices(tf).
// Then uses these and creates a new vertex data 
// line looking like this: x, y ,z, u ,v
// Number of lines is based on the number f-lines in obj. And every f line has 3 vertex. So 3 * f-lines is the new vertex data.
// obj specification: https://paulbourke.net/dataformats/obj/
ObjData loadObjFile(const char *filepath)
{
    ObjData objData;
    int MAX = 30000;
    int vf[30000] = {0}; 
    int tf[30000] = {0}; 
    float vArr[MAX];
    float tArr[MAX];

    int vIndex = 0;
    int uvCount = 0;
    int normalCount = 0;
    int vfCount = 0;
    int tfCount = 0;
    int faceLineCount = 0;
    char material[100];
    char group[100];
    char usemtl[100];
  
    FILE* fp;
    char line[1024];

    fp = fopen(filepath, "r");
    if(fp == NULL) {
        printf(TEXT_COLOR_ERROR "Error opening file %s" TEXT_COLOR_RESET "\n", filepath);
        exit(1);
    }
    
    while (fgets(line, sizeof line, fp) != NULL){
   
        // comment
        if((int)line[0] == 35){
            continue;
        }
        // newline
        if((int)line[0] == 10){
            continue;
        }
        // m, check for mtllib
        if((int)line[0] == 109){
            if(strncmp(line, "mtllib", 6) == 0){
                for(int i = 7; i < strlen(line); i++){
                    material[i-7] = line[i];
                }
            }
            continue;
        }
        // g, grouping, read in the groups
        if((int)line[0] == 103){
            for(int i = 2; i < strlen(line); i++){
                group[i-2] = line[i];
            }
            continue;
        }
        // u, check for usemtl
        if((int)line[0] == 117){
            if(strncmp(line, "usemtl", 6) == 0){
                for(int j = 7; j < strlen(line); j++){
                    usemtl[j-7] = line[j];
                }
                usemtl[strlen(line)-7] = '\0'; // Null terminate usemtl
            }
            continue;
        }
        // v: v & space(32), read vertices
        if((int)line[0] == 118 && (int)line[1] == 32){
          sscanf(line, "v %f %f %f", &vArr[vIndex], &vArr[vIndex+1], &vArr[vIndex+2]);
          vIndex+=3;
          continue;
        }
        // vt: v & t, read texcoords(uv)
        if((int)line[0] == 118 && (int)line[1] == 116){
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            tArr[uvCount] = strtof(token, NULL);
            token = strtok(NULL, " ");
            tArr[uvCount+1] = strtof(token, NULL);
            uvCount+=2;
            continue;
        }
        // vn: v & n, read normals
        if((int)line[0] == 118 && (int)line[1] == 110){
            normalCount++;
            continue;
        }
        // f: face indicies, ex: f 1/1/1 2/2/2 3/3/3 
        if((int)line[0] == 102){
           faceLineCount++;
           if(faceLineCount >= MAX || vfCount >= MAX || tfCount >= MAX){
                printf("Error: Too many vertices in obj file. Max is 30000. Exiting..");
                exit(1);
           }
           char *facetoken = strtok(line, " /"); // f
           facetoken = strtok(NULL, " /"); // v1
           int facetokenInt = atoi(facetoken);
           vf[vfCount] = facetokenInt;
           vfCount++;
           facetoken = strtok(NULL, " /");
           int facetokentInt = atoi(facetoken);
           tf[tfCount] = facetokentInt;
           tfCount++;
           facetoken = strtok(NULL, " /");
           facetoken = strtok(NULL, " /"); // v2
           int facetokenInt2 = atoi(facetoken);
           vf[vfCount] = facetokenInt2;
           vfCount++;
           facetoken = strtok(NULL, " /");
           int facetokentInt2 = atoi(facetoken);
           tf[tfCount] = facetokentInt2;
           tfCount++;
           facetoken = strtok(NULL, " /");
           facetoken = strtok(NULL, " /");
           int facetokenInt3 = atoi(facetoken);
           vf[vfCount] = facetokenInt3;
           vfCount++;
           facetoken = strtok(NULL, " /");
           int facetokentInt3 = atoi(facetoken);
           tf[tfCount] = facetokentInt3;
           tfCount++;
           continue;
        }
  
    }

   
       fclose(fp);
        

       

     
  //  printf("material: %s \n",material);
  //  printf("faceline count: %d \n",faceLineCount);
  //  printf("group: %s",group);
  //  printf("usemtl: %s",usemtl);
  //  printf("vIndex %d \n", vIndex);
    //printf("vfCount %d \n", vfCount); 
  //  printf("uvCount %d \n", uvCount);
  //  printf("normalCount %d \n", normalCount); 
  
 objData.vertexData = malloc(vfCount * sizeof(Vertex));
   if(objData.vertexData == NULL){
    exit(1);
   }
 

    for(int i = 0; i < vfCount; i+=1){
    //  printf("vertex %i \n",vf[i]);
     //printf("vertex %i \n",i);

      // x y z
    //printf("v %f %f %f ",vArr[(vf[i]-1)*3],vArr[(vf[i]-1)*3+1],vArr[(vf[i]-1)*3+2]);
      objData.vertexData[i].position[0] = vArr[(vf[i]-1)*3];
      objData.vertexData[i].position[1] = vArr[(vf[i]-1)*3+1];
      objData.vertexData[i].position[2] = vArr[(vf[i]-1)*3+2];

      // color, default to black atm
     objData.vertexData[i].color[0] = 0.0; 
     objData.vertexData[i].color[1] = 0.0;
     objData.vertexData[i].color[2] = 0.0;

      // uv
      //printf("t %f %f \n",tArr[(tf[i]-1)*2],tArr[(tf[i]-1)*2+1]);
   // printf("\n");

      objData.vertexData[i].texcoord[0] = tArr[(tf[i]-1)*2];
      objData.vertexData[i].texcoord[1] = tArr[(tf[i]-1)*2+1];
    }

    objData.num_of_vertices = vfCount;

    return objData;
  
   
}




//----------------------------------------
//---COLLISION DETECTION------------------
//----------------------------------------

/**
 *  Function to check if a point is inside the rectangle.
 * NOTE: x,y,width,height is expected to be in SDL coordinates, Where x,y is the bottom left corner of the rectangle.
 * use convertViewRectangleToSDLCoordinates to convert view coordinates to SDL coordinates if needed.
 * @param rect Rectangle to check against, x,y,width,height is expected to be in SDL coordinates
 */ 
int isPointInsideRect(Rectangle rect, vec2 point) {
    return (point[0] >= rect.x && point[0] <= rect.x + rect.width &&
            point[1] >= rect.y && point[1] <= rect.y + rect.height);
}

/**
 * Function to convert a view rectangle to SDL coordinates from view coordinates(opengl viewport coords).
 */
Rectangle convertViewRectangleToSDLCoordinates(View view,int windowHeight) {
    view.rect.y = windowHeight - view.rect.y - view.rect.height;
    return view.rect;
}

/**
 * Converts ui coordinates(x,y) to SDL coordinates for the provided view.
 * So if view has x,y = 0,200 and width,height = 800,200 , 
 * zero will be upper left corner: 0,400 and bottom right corner: 800,600
 */
Vector2 convertUIcoordinateToSDLcoordinates(View view, float p1,float p2, int windowHeight,int windowWidth)
{
  
    float newX = (float)windowWidth - (float)view.rect.width + (float)view.rect.width / 2.0 + p1;
    float newY = (float)windowHeight - (float)view.rect.height + p2;
    Vector2 result = {.x=newX,.y=newY};
    printf("------------------\n");
    printf("view.rect.x %f \n", (float)view.rect.x);
    printf("view.rect.y %f \n", (float)view.rect.y);
    printf("view.rect.width %f \n", (float)view.rect.width);
    printf("view.rect.height %f \n", (float)view.rect.height);
    printf("windowHeight %d \n", windowHeight);
    printf("point[0] %f \n", p1);
    printf("point[1] %f \n", p2); 
    printf("newX %f \n",newX);
    printf("newY %f \n",newY); 
    return result;
    
    /* point[1] = windowHeight - point[1];
    point[0] = point[0] + view.rect.x;
    point[1] = point[1] + view.rect.y; */
}
