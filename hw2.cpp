#include <stdlib.h>			
#include <stdio.h>			
#include <cmath>
#include <cstring>  
#include <math.h>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>

#include <sys/resource.h>

#include "Angel.h"

#define PI 3.14159265

using namespace std;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

struct face {
  int index1;
  int index2;
  int index3;
  int index4;
} ;

point4* vertices; //// vertices
point4* verticesArr; //// vertices of faces by order
point4* colors;  /// colors
face* faces;  /// faces of model

GLfloat  Theta=0.0; ///// buyuk T ile baslayan y ekseninde dondurmek icin

int numVertices=0; //// for pulling dynnamicly vertices number
int numFaces=0; //// for pulling dynnamicly face number data
int numPoints=0; //// for pulling dynnamicly points of face sdata
int currentDrawMode=0; /// 0-triangles 1-lines 2-points
// Viewing transformation parameters
char* shapes[10]; //// modellerin dosya isimleri burda tutulacak
int shapeIndex=0; /// hangi modeli gosterdigi
int shapenumber=0;  /// kac tane model oldugu
int modelchanged=0; /// n ve p ye basinca 1 olur modele gore sistem kendini update edip 0 yapar
GLfloat scalevalue=1.0; /// scaling
GLfloat radius = 2.0; /// default value will be alter after reading data for given shape
GLfloat theta = 0.0; /// kucuk t ile olan  viewerin konumu belirlemek icin
GLfloat phi = 0.0;  
GLfloat centerX = 0.0; GLfloat centerY = 0.0; GLfloat centerZ = 0.0;
GLfloat translateX = 0.0; GLfloat translateY = 0.0; GLfloat translateZ = 0.0;

const GLfloat  dr = 5.0 * DegreesToRadians;

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  rotation;   //// rotation matrixi
GLuint  scaling;   //// scaling matrixi
GLuint  translation;  //// translation matrixi
GLuint  toorigin;   //// rotate ve scale yapmadan once modelin merkezini origine getir
GLuint  fromorigin;  ///// toorigini reverse eder
GLuint  projection; // projection matrix uniform shader variable location
// Projection transformation parameters


GLfloat  fovy = 135.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 10.0;

void increaseStackSize(){ /// armadillo ve brain sayfa boyutu 3.6M gectiginden sayfayi genisletiyor
    const rlim_t kStackSize = 128 * 1024 * 1024;   // min stack size = 16 MB
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0)
    {
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
        }
    }
}

int getRandomInt(int limit){
	return rand()%limit;
}
float getRandomFloat(int max){ /// random generates for max 100 return 0-1.00
	return ( (float)( getRandomInt(max)))/100;
}
int fetchShapeData(){ /// ilgili modellerdeki verileri ceker
    string line;
    string filename =shapes[shapeIndex];
    ifstream myfile ("Models/"+filename);
    int index =0;
    if (myfile.is_open()){
    getline (myfile,line);
    getline (myfile,line);
    char* c = const_cast<char*>(line.c_str());
    char *ptr; // declare a ptr pointer  
    ptr = strtok(c, " "); 
    // cout << ptr  << endl;  
    numVertices=atoi(ptr);
    ptr = strtok (NULL, " ");
    numFaces=atoi(ptr);
    numPoints=numFaces*3;
    // cout << numVertices  << endl;  
    // cout << numFaces <<" assasa" << endl;  
    vertices= (point4*) malloc(sizeof(point4)*numVertices);
    verticesArr= (point4*) malloc(sizeof(point4)*numFaces*3);
    colors= (point4*) malloc(sizeof(point4)*numFaces*3);
    faces= (face*) malloc(sizeof(face)*numFaces);
    int indexhatavermeyen=0;
    float f1=0.0; float f2=0.0; float f3=0.0;
    while ( getline (myfile,line) &&  indexhatavermeyen<numVertices) /// pulling vertices
        {
            c = const_cast<char*>(line.c_str());
            sscanf(c,"%f %f %f",&f1,&f2,&f3);
            vertices[indexhatavermeyen].x=f1;  
            vertices[indexhatavermeyen].y=f2;
            vertices[indexhatavermeyen].z=f3;
            vertices[indexhatavermeyen].w=1.0; 
            indexhatavermeyen++;
            // cout << vertices[indexhatavermeyen-1] << "----" << '\n';
        }
    int indexhatavermeyen2=0;
    int i1=0; int i2=0; int i3=0; int i4=0;
    do //// pull faces
    {
        c = const_cast<char*>(line.c_str());
        sscanf(c,"%d %d %d %d",&i1,&i2,&i3,&i4);
        faces[indexhatavermeyen2].index1=i1; faces[indexhatavermeyen2].index2=i2; 
        faces[indexhatavermeyen2].index3=i3; faces[indexhatavermeyen2].index4=i4;
        indexhatavermeyen2++;
    }while ( getline (myfile,line) &&  indexhatavermeyen2<numFaces);

    for(int index1=0,index2=0;index1<numFaces;index1++){ 
        verticesArr[index2]=vertices[faces[index1].index2]; 
        colors[index2]=point4(getRandomFloat(100),getRandomFloat(100),getRandomFloat(100),1);     index2++;
        verticesArr[index2]=vertices[faces[index1].index3]; 
        colors[index2]=point4(getRandomFloat(100),getRandomFloat(100),getRandomFloat(100),1);     index2++;
        verticesArr[index2]=vertices[faces[index1].index4];
        colors[index2]=point4(getRandomFloat(100),getRandomFloat(100),getRandomFloat(100),1);     index2++;
    }
    myfile.close();
  }
  else cout << "Unable to open file"; 
  return 0;
}
void adjustView(){ //// bu fonksiyon modelin degerlerini inceler uygun radius ve zFar degerleri verir modeli gorunur yapar
    float maxx=0; float minx=0; float maxy=0; float miny=0; float maxz=0; float minz=0; float sumx=0; float sumy=0; float sumz=0;
    for(int i=0;i<numVertices;i++){
        if(maxx<vertices[i].x) maxx=vertices[i].x;
        if(minx>vertices[i].x) minx=vertices[i].x;
        if(maxy<vertices[i].y) maxy=vertices[i].y;
        if(miny>vertices[i].y) miny=vertices[i].y;
        if(maxx<vertices[i].z) maxz=vertices[i].z;
        if(minx>vertices[i].z) minz=vertices[i].z;
        sumx+=vertices[i].x; sumy+=vertices[i].y; sumz+=vertices[i].z;
    } 
    centerX=sumx/numVertices; centerY=sumy/numVertices; centerZ=sumz/numVertices; /// modelin merkezini hesaplama
    float maxmax=max(maxx,max(maxy,maxz));
    float minmin=min(minx,min(miny,minz));
    // cout << centerX << "----  "<< centerY << "  ----  " <<  centerZ << "  ----  " <<'\n';
    radius=(maxmax-minmin);
    zFar=1.0+(maxmax-minmin)*4;
}

void init()
{
    srand(time(NULL));
    fetchShapeData();
    
    adjustView();
    
    point4 verticesArr2[numPoints]; //// pointer verince isler karisti array vermek icin tanimladim
    point4 colors2[numPoints];
    for(int index=0;index<numPoints;index++){
        verticesArr2[index]=*(verticesArr+index);
        colors2[index]=*(colors+index);
    }
     
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(verticesArr2) + sizeof(colors2),
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(verticesArr2), verticesArr2 );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(verticesArr2), sizeof(colors2), colors2 );
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "hw2sh.glsl", "fshader42.glsl" );
    glUseProgram( program );
            
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(verticesArr2)) );

    model_view = glGetUniformLocation( program, "model_view" );
    projection = glGetUniformLocation( program, "projection" );
    rotation = glGetUniformLocation( program, "rotation" );
    scaling = glGetUniformLocation( program, "scaling" );
    translation = glGetUniformLocation( program, "translation" );
    toorigin = glGetUniformLocation( program, "toorigin" );
    fromorigin = glGetUniformLocation( program, "fromorigin" );

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  
    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
}
//----------------------------------------------------------------------------
void reset(){
    translateZ=0; translateY=0; translateX=0;
    scalevalue=1.0; phi=0.0; theta=0;
}

void display( void )
{
    if(modelchanged==1){
        reset();
        init();
        modelchanged=0;
    }
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    point4  eye( radius*sin(theta)*cos(phi),
		 radius*sin(theta)*sin(phi),
		 radius*cos(theta),
		 1.0 );
    point4  at( 0.0, 0.0, 0.0, 1.0 );
    vec4    up( 0.0, 1.0, 0.0, 0.0 );

    vec3 c = cos( (Theta*PI)/180);
    vec3 s = sin( (Theta*PI)/180);
    mat4 rotationmat = mat4( c.y, 0.0, -s.y, 0.0, /// rotation matrixii
                    0.0, 1.0,  0.0, 0.0,
                    s.y, 0.0,  c.y, 0.0,
                    0.0, 0.0,  0.0, 1.0 );

    glUniformMatrix4fv( rotation, 1, GL_TRUE, rotationmat );

    mat4 tooriginmat = mat4( 1.0, 0.0, 0.0, 0.0,
                        0.0, 1.0,  0.0, 0.0,
                        0.0, 0.0,  1.0, 0.0,
                        -centerX,-centerY, -centerZ, 1.0 );

    glUniformMatrix4fv( toorigin, 1, GL_TRUE, tooriginmat );
         
    mat4 fromoriginmat = mat4( 1.0, 0.0, 0.0, 0.0,
                        0.0, 1.0,  0.0, 0.0,
                        0.0, 0.0,  1.0, 0.0,
                        centerX,centerY, centerZ, 1.0 );

    glUniformMatrix4fv( fromorigin, 1, GL_TRUE, fromoriginmat );

    mat4 scalemat= mat4( scalevalue, 0.0,  0.0, 0.0, /// scaling
                0.0, scalevalue,  0.0, 0.0,
                0.0, 0.0,  scalevalue, 0.0,
                0.0, 0.0,  0.0, 1.0 );
    glUniformMatrix4fv( scaling, 1, GL_TRUE, scalemat ); /// transition

    mat4 translatemat= mat4( 1.0, 0.0,  0.0, 0.0,
                        0.0, 1.0,  0.0, 0.0,
                        0.0, 0.0,  1.0, 0.0,
                        translateX, translateY,translateZ, 1.0 );
    
    glUniformMatrix4fv( translation, 1, GL_TRUE,  translatemat );

    mat4  mv = LookAt( eye, at, up );
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );

    mat4  p = Perspective( fovy, aspect, zNear, zFar );
    glUniformMatrix4fv( projection, 1, GL_TRUE, p );

    if(currentDrawMode==0)
        glDrawArrays( GL_TRIANGLES, 0, numPoints ); 
    else if(currentDrawMode==1)
        glDrawArrays( GL_LINE_STRIP, 0, numPoints ); 
    else
        glDrawArrays( GL_POINTS, 0, numPoints ); 
    
    glutSwapBuffers();
}
//----------------------------------------------------------------------------
void keyboard( unsigned char key, int x, int y )
{
    
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;

	case 'x': translateX =translateX+ 0.2; break;
	case 'X': translateX =translateX- 0.2; break;
	case 'y': translateY =translateY+ 0.2; break;
	case 'Y': translateY =translateY- 0.2; break;
	case 'z': translateZ =translateZ+ 0.2; break;
	case 'Z': translateZ =translateZ- 0.2; break;
	case 'r': radius *= 0.9; break;
	case 'R': radius *= 1.1; break;
	case 'o': theta += dr; break;
	case 'O': theta -= dr; break;
	case 'u': phi += dr; break;
	case 'U': phi -= dr; break;
    case 'v': currentDrawMode = 2; break;
	case 'e': currentDrawMode = 1; break;
	case 'f': currentDrawMode = 0; break;
    case 'n': shapeIndex = (shapeIndex+1)%shapenumber; modelchanged=1; break;
    case 'p': shapeIndex = (shapeIndex+shapenumber-1)%shapenumber; modelchanged=1; break; //// previous niye boyle cunku -1% yapinca garip hata aliyorum
    // case  GLUT_KEY_LEFT: Theta+=2.0;break; //// arrowkeyler algilamiyor.
    // case  GLUT_KEY_RIGHT: Theta-=2.0;break;
    case 'd': Theta+=2.0;break; break;
	case 'D': Theta-=2.0;break; break;
    case 's': scalevalue *= 2; break;
	case 'S': scalevalue /= 2; break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
    aspect = GLfloat(width)/height;
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    increaseStackSize();
    glutInit( &argc, argv );
    for (int i = 1; i < argc; ++i) {
        shapes[i-1]=argv[i];
    } 
    shapenumber=argc-1;
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 662, 662 );
//    glutInitContextVersion( 3, 2 );
//    glutInitContextProfile( GLUT_CORE_PROFILE );
    glutCreateWindow( "Color Cube" );
	glewExperimental = GL_TRUE;
    glewInit();

    init();

    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );

    glutMainLoop();
    return 0;
}
