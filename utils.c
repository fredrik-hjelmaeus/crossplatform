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

/* void addToArray(char* token, int* arr, int arrCount) {
    if(arrCount >= 300000){
        printf("Error: Too many vertices in obj file. Max is 300000. Exiting..");
        exit(1);
    }
    token = strtok(NULL, " /");
    int tokenInt = atoi(token);
    arr[arrCount] = tokenInt;
    printf("facetoken %s \n",token);
} */

int handleFaceLine(char* line, int* vf, int* tf, int* vn, int* vfCount, int* tfCount, int* vnCount, int* faceLineCount) {
 //  printf("line: %s\n", line);
   int vertexCount = 3;
   int num_slashes = 0;
   int spaceCount = 0;
  
   bool isSlash = false;
   bool hasTextureData = true;
   char tempNumber[10];
   int tempNumberIndex = 0;
   int tokenIndex = 0;
   int tokensArr[12] = {0};
   char* linePtr = (line+2); // skip f and space
   while(*linePtr != '\0'){

       //printf("linePtr--------: %c\n", *linePtr);
       //printf("tknIndex-------: %d\n", tokenIndex);
    
       if(*linePtr == ' '){
           spaceCount++;
           tempNumber[tempNumberIndex] = '\0'; // Null-terminate the string
           int num = atoi(tempNumber);
           tokensArr[tokenIndex] = num;
           tokenIndex++;
           //printf("space num: %d\n", num);
           tempNumberIndex = 0;
           isSlash = false;
       }
       else if(*linePtr == '/'){
         num_slashes++;
         if(isSlash){
            //printf("this is a double slash\n");
            hasTextureData = false;
         }
         if(tempNumberIndex > 0){
            tempNumber[tempNumberIndex] = '\0'; // Null-terminate the string
            int num = atoi(tempNumber);
            tokensArr[tokenIndex] = num;
            tokenIndex++;
            tempNumberIndex = 0;
           // printf("slash num: %d\n", num);
         }
         isSlash = true;
       }
       else{
        tempNumber[tempNumberIndex] = *linePtr;
        tempNumberIndex++;
        isSlash = false;
       }
       linePtr++;
   }
   // Pick up the last number
   if(tempNumberIndex > 0){
         tempNumber[tempNumberIndex] = '\0'; // Null-terminate the string
         int num = atoi(tempNumber);
         tokensArr[tokenIndex] = num;
        // printf("last num: %d\n", num);
   }
    //printf("spaceCount: %d\n", spaceCount);
   // printf("num_slashes: %d\n", num_slashes);

     // if has texture and is triangle,the token count should be 9
    // tokenIndex 0 , 3 , 6 is v
    // tokenIndex 1 , 4 , 7 is vt
    // tokenIndex 2 , 5 , 8 is vn
   // if has texture and is quad, the token count should be 12
    // tokenIndex 0 , 3 , 6 , 9 is v
    // tokenIndex 1 , 4 , 7 , 10 is vt
    // tokenIndex 2 , 5 , 8 , 11 is vn
   // if has no texture and is triangle, the token count should be 6
    // tokenIndex 0 , 2 , 4 is v
    // tokenIndex 1 , 3 , 5 is vn
   // if has no texture and is quad, the token count should be 8
    // tokenIndex 0 , 2 , 4 , 6 is v
    // tokenIndex 1 , 3 , 5 , 7 is vn
    if(hasTextureData){
            vf[*vfCount] = tokensArr[0];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[1];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[2];
            (*vnCount)++;
            vf[*vfCount] = tokensArr[3];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[4];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[5];
            (*vnCount)++;
            vf[*vfCount] = tokensArr[6];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[7];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[8];
            (*vnCount)++;
        if(spaceCount > 2){
           // printf("Quad with texture(uv) data\n");
            vertexCount = 9;
            
            //First triangle: v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            //Second triangle: v1/vt1/vn1 v3/vt3/vn3 v4/vt4/vn4

            // First vertex
            vf[*vfCount] = tokensArr[0];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[1];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[2];
            (*vnCount)++;

            // Second vertex
            vf[*vfCount] = tokensArr[6];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[7];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[8];
            (*vnCount)++;

            // Third vertex
            vf[*vfCount] = tokensArr[9];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[10];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[11];
            (*vnCount)++;
        }else {
          //  printf("Triangle with texture(uv) data\n");
        }
    }else {
        vf[*vfCount] = tokensArr[0];
        (*vfCount)++;
        vn[*vnCount] = tokensArr[1];
        (*vnCount)++;
        vf[*vfCount] = tokensArr[2];
        (*vfCount)++;
        vn[*vnCount] = tokensArr[3];
        (*vnCount)++;
        vf[*vfCount] = tokensArr[4];
        (*vfCount)++;
        vn[*vnCount] = tokensArr[5];
        (*vnCount)++;
        if(spaceCount > 2){
            
            vertexCount = 6;
            
            //First triangle: v1//vn1 v2//vn2 v3//vn3
            //Second triangle: v1//vn1 v3//vn3 v4//vn4

            // First vertex
            vf[*vfCount] = tokensArr[0];
            (*vfCount)++;
            vn[*vnCount] = tokensArr[1];
            (*vnCount)++;
            // Second vertex
            vf[*vfCount] = tokensArr[4];
            (*vfCount)++;
            vn[*vnCount] = tokensArr[5];
            (*vnCount)++;
            // Third vertex
            vf[*vfCount] = tokensArr[6];
            (*vfCount)++;
            vn[*vnCount] = tokensArr[7];
            (*vnCount)++;

            //printf("Quad without texture(uv) data\n");

        }else {
           // printf("Triangle without texture(uv) data\n");
        }
    }

    // Print out the vf,tf,vn data
    /* for(int i = 0; i < *vfCount; i++){
        printf("vf: %d\n", vf[i]);
    }
    for(int i = 0; i < *tfCount; i++){
        printf("tf: %d\n", tf[i]);
    }
    for(int i = 0; i < *vnCount; i++){
        printf("vn: %d\n", vn[i]);
    } 
    printf("vfCount: %d\n", *vfCount);
    printf("tfCount: %d\n", *tfCount);
    printf("vnCount: %d\n", *vnCount); */


    (*faceLineCount)++;
    return vertexCount;
}

/**
 * Run tests.
 * Atm no expect/asserts, just printouts you manually have to check.
 */
void runTests()
{
   char line[] =  "f 1/2/3 4/5/6 7/8/9"; // 9
   char line2[] = "f 7//7 8//8 9//9"; // 6
   char line3[] = "f 7//7 8//8 9//9 10//10"; // 8
   char line4[] = "f 7555555//55555557 83333333//833333333 222222229//222222229"; // 6
   char line5[] = "f 1/2/3 4/5/6 7/8/9 3/3/3"; // 12

   int vf[300000] = {0}; 
   int tf[300000] = {0}; 
   int vn[300000] = {0};
  
   FaceLine faceLineList[300000] = {};
   int faceLineCount = 0;
   
   int vfCount = 0;
   int tfCount = 0;
   int vnCount = 0; 

   faceLineList[0].vertexCount = handleFaceLine(line,  vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   faceLineList[1].vertexCount = handleFaceLine(line2, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   faceLineList[2].vertexCount = handleFaceLine(line3, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   faceLineList[3].vertexCount = handleFaceLine(line4, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   faceLineList[4].vertexCount = handleFaceLine(line5, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   
   // Print the faceLineList data
    for(int i = 0; i < 5; i++){
         printf("faceLineList[%d].vertexCount: %d\n", i, faceLineList[i].vertexCount);
    }
   // Exit program after tests
   exit(0);
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
    const int MAX = 3000000;
   // int vf[300000] = {0}; 
    int tf[300000] = {0}; 
    int vn[300000] = {0};
    float vArr[300000] = {0.0f};
    float tArr[300000]=  {0.0f};
    float nArr[300000]=  {0.0f}; 

     int* vf = (int*)malloc(MAX * sizeof(int));
    if (vf == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
   /* int* tf = (int*)malloc(MAX * sizeof(int));
    if (tf == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    int* vn = (int*)malloc(MAX * sizeof(int));
    if (vn == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    float* vArr = (float*)malloc(MAX * sizeof(float));
    if (vArr == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    float* tArr = (float*)malloc(MAX * sizeof(float));
    if (tArr == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    float* nArr = (float*)malloc(MAX * sizeof(float));
    if (nArr == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    } */

/*     objData.vertexData = malloc(MAX * sizeof(Vertex));
    if(objData.vertexData == NULL){
    printf("Failed to allocate memory for objData.vertexData\n");
    exit(1);
    } */

    int vIndex = 0;
    int uvCount = 0;
    int normalCount = 0;
    int vfCount = 0;
    int tfCount = 0;
    int vnCount = 0;
    FaceLine faceLineList[5000] = {};
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
     //printf(TEXT_COLOR_ERROR "line %s" TEXT_COLOR_RESET "\n", line);
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
            //printf(TEXT_COLOR_ERROR "v %s" TEXT_COLOR_RESET "\n", line);
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
        // f: face indicies, 
        // Can be : f 1/1/1 2/2/2 3/3/3
        // or       f 1//1 2//2 3//3
        // or       f 1//1 2//2 3//3 4//4  <- quad
        if((int)line[0] == 102){
           faceLineList[faceLineCount].vertexCount = handleFaceLine(line, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
           if(vfCount >= MAX || tfCount >= MAX){
                printf("Error: Too many vertices in obj file. Max is 300000. Exiting..");
                printf("vfCount: %d\n", vfCount);
                printf("tfCount: %d\n", tfCount);
                exit(1);
           }
           if(faceLineCount >= 500000){
                printf("Error: Too many face lines in obj file. Max is 500000. Exiting..");
                exit(1);
           }
           //char *facetoken = strtok(line, " /"); // f
           //addToArray(facetoken, vf, vfCount);
           //vfCount++;
           // Check if // or / structure here:
           // f 1//1 2//2 3//3
           // f 1/1/1 2/2/2 3/3/3
          /*  addToArray(facetoken, tf, tfCount);
           tfCount++;
           facetoken = strtok(NULL, " /");  // vn1
           if(facetoken == NULL) continue;
           int facetokenvInt = atoi(facetoken);
           vn[vnCount] = facetokenvInt;
           vnCount++;
           facetoken = strtok(NULL, " /");  // v2
           if(facetoken == NULL) continue;
           int facetokenInt2 = atoi(facetoken);
           vf[vfCount] = facetokenInt2;
           vfCount++;
           facetoken = strtok(NULL, " /"); 
           if(facetoken == NULL) continue;
           int facetokentInt2 = atoi(facetoken);
           tf[tfCount] = facetokentInt2;
           tfCount++;
           facetoken = strtok(NULL, " /");
           if(facetoken == NULL) continue;
           int facetokenv2Int = atoi(facetoken);
           vn[vnCount] = facetokenv2Int;
           vnCount++;
           facetoken = strtok(NULL, " /");
           if(facetoken == NULL) continue;
           int facetokenInt3 = atoi(facetoken);
           vf[vfCount] = facetokenInt3;
           vfCount++;
           facetoken = strtok(NULL, " /");
           if(facetoken == NULL) continue;
           int facetokentInt3 = atoi(facetoken);
           tf[tfCount] = facetokentInt3;
           tfCount++;
           facetoken = strtok(NULL, " /");
           if(facetoken == NULL) continue;
           int facetokenv3Int = atoi(facetoken);
           vn[vnCount] = facetokenv3Int;
           vnCount++;
           continue; */
        }
  
    }

    fclose(fp);
        
    //printf("material: %s \n",material);
    printf("faceline count: %d \n",faceLineCount);
    //printf("group: %s \n",group);
    //printf("usemtl: %s \n",usemtl);
  //  printf("vIndex %d \n", vIndex);
    printf("num_of_vertex %d \n", vfCount);
   // printf("vfCount %d \n", vfCount); 
   // printf("uvCount %d \n", uvCount);
   // printf("normalCount %d \n", normalCount); 
   // printf("vnCount %d \n", vnCount);
    int num_of_vertex = vfCount;
  
   objData.vertexData = malloc(num_of_vertex * 2 * sizeof(Vertex));
   if(objData.vertexData == NULL){
    printf("Failed to allocate memory for objData.vertexData\n");
    exit(1);
   }

    int vertexIndex = 0;
    for(int j = 0; j < faceLineCount; j+=1){

      //  printf("faceline: "  "%d"  " of %d \n", j,faceLineCount);
   //   printf("faceline vertex count %d\n", faceLineList[j].vertexCount);
        for(int i = 0; i < faceLineList[j].vertexCount; i++){
    /*        if(vArr[(vf[vertexIndex]-1)*3] > 1000.0){
                printf("vArr out of bounds %f\n", vArr[(vf[vertexIndex]-1)*3]);
                exit(1);
           } */
           //printf("(vf[vertexIndex]-1)*3 %d\n", (vf[vertexIndex]-1)*3);
            // x y z
            //printf("f " TEXT_COLOR_YELLOW "%f %f %f " ,vArr[(vf[i]-1)*3],vArr[(vf[i]-1)*3+1],vArr[(vf[i]-1)*3+2]);
            objData.vertexData[vertexIndex].position[0] = vArr[(vf[vertexIndex]-1)*3];
            objData.vertexData[vertexIndex].position[1] = vArr[(vf[vertexIndex]-1)*3+1];
            objData.vertexData[vertexIndex].position[2] = vArr[(vf[vertexIndex]-1)*3+2];
           // printf("f " TEXT_COLOR_YELLOW "%f %f %f " ,objData.vertexData[vertexIndex].position[0],objData.vertexData[vertexIndex].position[1],objData.vertexData[vertexIndex].position[2]);

            // color, default to black atm
            objData.vertexData[vertexIndex].color[0] = 1.0; 
            objData.vertexData[vertexIndex].color[1] = 1.0;
            objData.vertexData[vertexIndex].color[2] = 1.0;
            //printf(TEXT_COLOR_BLUE " %f %f %f ",objData.vertexData[i].color[0],objData.vertexData[i].color[1],objData.vertexData[i].color[2]);

            // uv
            
            // printf("\n");
            if(uvCount == 0){
                objData.vertexData[vertexIndex].texcoord[0] = 0.0;
                objData.vertexData[vertexIndex].texcoord[1] = 0.0;
            }else {
                objData.vertexData[vertexIndex].texcoord[0] = tArr[(tf[vertexIndex]-1)*2];
                objData.vertexData[vertexIndex].texcoord[1] = tArr[(tf[vertexIndex]-1)*2+1];
               // printf(TEXT_COLOR_MAGENTA" %f %f ",objData.vertexData[vertexIndex].texcoord[0],objData.vertexData[vertexIndex].texcoord[1]);
            }


            // normals
            if(vnCount == 0){
                objData.vertexData[vertexIndex].normal[0] = 0.0;
                objData.vertexData[vertexIndex].normal[1] = 0.0;
                objData.vertexData[vertexIndex].normal[2] = 0.0;
            }else {
                objData.vertexData[vertexIndex].normal[0] = nArr[(vn[vertexIndex]-1)*3];
                objData.vertexData[vertexIndex].normal[1] = nArr[(vn[vertexIndex]-1)*3+1];
                objData.vertexData[vertexIndex].normal[2] = nArr[(vn[vertexIndex]-1)*3+2];
                // print normals
                //printf(TEXT_COLOR_CYAN" %f %f %f \n" TEXT_COLOR_RESET,objData.vertexData[vertexIndex].normal[0],objData.vertexData[vertexIndex].normal[1],objData.vertexData[vertexIndex].normal[2]);
            }
            vertexIndex++;
         //   printf("vertexIndex %d\n", vertexIndex);
        }
    }

    objData.num_of_vertices = num_of_vertex;
     free(vf);
 /*  free(tf);
    free(vn);
    free(vArr);
    free(tArr);
    free(nArr);  */
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
