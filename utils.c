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
    int vn[30000] = {0};
    float vArr[MAX];
    float tArr[MAX];
    float nArr[MAX];

    int vIndex = 0;
    int uvCount = 0;
    int normalCount = 0;
    int vfCount = 0;
    int tfCount = 0;
    int vnCount = 0;
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
            // TODO: grab normals in an array
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            nArr[normalCount] = strtof(token, NULL);
            token = strtok(NULL, " ");
            nArr[normalCount+1] = strtof(token, NULL);
            token = strtok(NULL, " ");
            nArr[normalCount+2] = strtof(token, NULL);
            //vnCount+=3;
            normalCount+=3;
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
           int facetokenvInt = atoi(facetoken);
           vn[vnCount] = facetokenvInt;
           vnCount++;
           facetoken = strtok(NULL, " /"); // v2
           int facetokenInt2 = atoi(facetoken);
           vf[vfCount] = facetokenInt2;
           vfCount++;
           facetoken = strtok(NULL, " /");
           int facetokentInt2 = atoi(facetoken);
           tf[tfCount] = facetokentInt2;
           tfCount++;
           facetoken = strtok(NULL, " /");
           int facetokenv2Int = atoi(facetoken);
           vn[vnCount] = facetokenv2Int;
           vnCount++;
           facetoken = strtok(NULL, " /");
           int facetokenInt3 = atoi(facetoken);
           vf[vfCount] = facetokenInt3;
           vfCount++;
           facetoken = strtok(NULL, " /");
           int facetokentInt3 = atoi(facetoken);
           tf[tfCount] = facetokentInt3;
           tfCount++;
           facetoken = strtok(NULL, " /");
           int facetokenv3Int = atoi(facetoken);
           vn[vnCount] = facetokenv3Int;
           vnCount++;
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
   // printf("normalCount %d \n", normalCount); 
   // printf("vnCount %d \n", vnCount);
  
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
        objData.vertexData[i].color[0] = 1.0; 
        objData.vertexData[i].color[1] = 1.0;
        objData.vertexData[i].color[2] = 1.0;

        // uv
       // printf("t %f %f \n",tArr[(tf[i]-1)*2],tArr[(tf[i]-1)*2+1]);
        // printf("\n");

        objData.vertexData[i].texcoord[0] = tArr[(tf[i]-1)*2];
        objData.vertexData[i].texcoord[1] = tArr[(tf[i]-1)*2+1];

        // normals
        objData.vertexData[i].normal[0] = nArr[(vn[i]-1)*3];
        objData.vertexData[i].normal[1] = nArr[(vn[i]-1)*3+1];
        objData.vertexData[i].normal[2] = nArr[(vn[i]-1)*3+2];
        // print normals
     //   printf("n %f %f %f \n",nArr[(vn[i]-1)*3],nArr[(vn[i]-1)*3+1],nArr[(vn[i]-1)*3+2]);
    }

    objData.num_of_vertices = vfCount;

    return objData;
  
   
}

// Function to convert an integer to a string and append ".png"
void intToPngFilename(int number, char* buffer, size_t bufferSize) {
    if (bufferSize < 10) {
        fprintf(stderr, "Buffer size is too small\n");
        return;
    }
    snprintf(buffer, bufferSize, "%d.png", number);
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
    view.rect.y = (float)windowHeight - (float)view.rect.y - (float)view.rect.height;
    return view.rect;
}

/**
 * Converts ui coordinates(x,y) to SDL coordinates for the provided view.
 * So if view has x,y = 0,200 and width,height = 800,200 , 
 * zero will be upper left corner: 0,400 and bottom right corner: 800,600
 */
void convertUIcoordinateToWindowcoordinates(View view, TransformComponent* transformComponent, int windowHeight,int windowWidth,vec2 convertedPoint) {
    float scaleFactorX = transformComponent->scale[0] / 100.0;  
    float scaleFactorY = transformComponent->scale[1] / 100.0; 
    transformComponent->scale[0] / 2.0 + transformComponent->position[0];
    convertedPoint[0] = (float)windowWidth / 2.0 + (transformComponent->position[0] - scaleFactorX * 100 / 2.0); 
    convertedPoint[1] = ((float)view.rect.height / 2.0) + transformComponent->position[1];
}
float absValue(float value) {
    return value < 0 ? -value : value;
}



// DEBUG

/**
 * Function to capture the framebuffer and save it to a png file.
 * Used to write every drawcall to a png file, for debugging purposes.
 */
void captureFramebuffer(int width, int height, int drawCallsCounter) {
    char filename[20];
    intToPngFilename(drawCallsCounter, filename, sizeof(filename));

    GLubyte* pixels = (GLubyte*)malloc(3 * width * height);
    if (!pixels) {
        fprintf(stderr, "Failed to allocate memory for pixels\n");
        return;
    }
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Flip y
    stbi_flip_vertically_on_write(1);

    // Save the pixels to an image file (e.g., using stb_image_write)
    stbi_write_png(filename, width, height, 3, pixels, width * 3);

    free(pixels);
}
