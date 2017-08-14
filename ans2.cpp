#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
  {
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}







/**************************
 * Customizable functions *
 **************************/

glm::mat4 VP,MVP;
double last_update_time = glfwGetTime(), current_time,update_call = glfwGetTime(),change_time = glfwGetTime();
int flag_move=0,flag_complete=0,flag_fallcomp=0,flag_fall=0,flag_stand=1,fall_call=0,fall_lvl3=0,flag_attach=1,flag_shift=0;
int max_level=4,lvl3_x,lvl3_y;
int var=0;
int moves=0,timehr=0,timemin=0,timesec=0,flag_gameover=0,flag_gamestart=0,miss_limit=10,miss=0,zoom=26,v=0,flag_hover=0;
double xpos,ypos;

class Stage{
public:
  int stage[5][15][10],target[5][2],start[5][2];
  int level,start_stage,end_stage;
  int anim_i,anim_j,flag;
  float initx,inity,zs,zs2;
  VAO *rect1, *rect2, *circle, *rect3, *rect4, *rect5;
  
public:
  Stage()
  {
    level=1;
    start_stage=1;
    zs=-50;
    end_stage=0;
    flag=0;
    initx=5;inity=5;
    anim_i=0;
    anim_j=0;

    start[0][0]=1;
    start[0][1]=6;
    start[1][0]=1;
    start[1][1]=3;
    start[2][0]=1;
    start[2][1]=3;
    start[3][0]=1;
    start[3][1]=4;

    for(int i=0;i<15;i++)
    {
      for(int j=0;j<10;j++)
      {
        stage[0][i][j]=0;
        stage[1][i][j]=0;
        stage[2][i][j]=0;
        stage[3][i][j]=0;
      }
    }

    stage[0][1][7]=1;stage[0][1][6]=1;stage[0][1][5]=1;stage[0][1][4]=1;
    stage[0][2][7]=1;stage[0][2][6]=1;stage[0][2][5]=1;stage[0][2][4]=1;
    stage[0][3][6]=1;stage[0][3][5]=1;stage[0][3][4]=1;
    stage[0][4][4]=1;
    stage[0][5][4]=1;stage[0][5][3]=1;
    stage[0][6][5]=1;stage[0][6][4]=1;stage[0][6][3]=1;stage[0][6][2]=1;
    stage[0][7][4]=1;stage[0][7][2]=1;
    stage[0][8][5]=1;stage[0][8][4]=1;stage[0][8][3]=1;stage[0][8][2]=1;
    stage[0][9][4]=1;stage[0][9][2]=1;

    target[0][0]=7;target[0][1]=3;
    stage[0][7][3]=2;

    stage[1][0][2]=1;stage[1][0][3]=1;stage[1][0][4]=1;stage[1][0][5]=1;stage[1][0][6]=1;
    stage[1][1][2]=1;stage[1][1][3]=1;stage[1][1][4]=1;stage[1][1][5]=1;stage[1][1][6]=1;
    stage[1][2][2]=1;stage[1][2][3]=1;stage[1][2][4]=1;stage[1][2][5]=3;stage[1][2][6]=1;
    stage[1][3][2]=1;stage[1][3][3]=1;stage[1][3][4]=1;stage[1][3][5]=1;stage[1][3][6]=1;
    stage[1][6][2]=1;stage[1][6][3]=1;stage[1][6][4]=1;stage[1][6][5]=1;stage[1][6][6]=1;stage[1][6][7]=1;
    stage[1][7][2]=1;stage[1][7][3]=1;stage[1][7][4]=1;stage[1][7][5]=1;stage[1][7][6]=1;stage[1][7][7]=1;
    stage[1][8][2]=1;stage[1][8][3]=1;stage[1][8][4]=1;stage[1][8][5]=1;stage[1][8][6]=4;stage[1][8][7]=1;
    stage[1][9][2]=1;stage[1][9][3]=1;stage[1][9][4]=1;stage[1][9][5]=1;stage[1][9][6]=1;stage[1][9][7]=1;
    stage[1][12][3]=1;stage[1][12][4]=1;stage[1][12][5]=1;stage[1][12][6]=1;stage[1][12][7]=1;
    stage[1][13][3]=1;stage[1][13][4]=1;stage[1][13][5]=1;stage[1][13][6]=2;stage[1][13][7]=1;
    stage[1][14][3]=1;stage[1][14][4]=1;stage[1][14][5]=1;stage[1][14][6]=1;stage[1][14][7]=1;

    target[1][0]=13;target[1][1]=6;

    stage[2][0][2]=1;stage[2][0][3]=1;stage[2][0][4]=1;stage[2][0][5]=1;stage[2][0][6]=1;
    stage[2][1][2]=1;stage[2][1][3]=1;stage[2][1][4]=1;stage[2][1][5]=1;stage[2][1][6]=1;
    stage[2][2][2]=1;stage[2][2][3]=1;stage[2][2][4]=1;stage[2][2][5]=1;stage[2][2][6]=1;
    stage[2][3][6]=1;stage[2][3][7]=5;stage[2][3][8]=5;
    stage[2][4][7]=5;stage[2][4][8]=5;
    stage[2][5][0]=1;stage[2][5][1]=1;stage[2][5][2]=1;stage[2][5][3]=1;stage[2][5][7]=5;stage[2][5][8]=5;
    stage[2][6][0]=1;stage[2][6][1]=2;stage[2][6][2]=1;stage[2][6][3]=1;stage[2][6][7]=5;stage[2][6][8]=5;
    stage[2][7][0]=1;stage[2][7][1]=1;stage[2][7][2]=1;stage[2][7][3]=1;stage[2][7][7]=5;stage[2][7][8]=5;
    stage[2][8][2]=1;stage[2][8][3]=1;stage[2][8][7]=5;stage[2][8][8]=5;
    stage[2][9][2]=5;stage[2][9][3]=5;stage[2][9][6]=1;stage[2][9][7]=5;stage[2][9][8]=5;
    stage[2][10][0]=5;stage[2][10][1]=5;stage[2][10][2]=5;stage[2][10][3]=5;stage[2][10][4]=1;stage[2][10][5]=1;stage[2][10][6]=1;
    stage[2][11][0]=5;stage[2][11][1]=5;stage[2][11][2]=5;stage[2][11][3]=5;stage[2][11][4]=1;stage[2][11][5]=1;stage[2][11][6]=1;
    stage[2][12][0]=5;stage[2][12][1]=1;stage[2][12][2]=5;stage[2][12][3]=5;
    stage[2][13][0]=5;stage[2][13][1]=5;stage[2][13][2]=5;stage[2][13][3]=5;

    target[2][0]=6;target[2][1]=1;

    stage[3][0][3]=1;stage[3][0][4]=1;stage[3][0][5]=1;
    stage[3][1][3]=1;stage[3][1][4]=1;stage[3][1][5]=1;
    stage[3][2][3]=1;stage[3][2][4]=1;stage[3][2][5]=1;
    stage[3][3][3]=1;stage[3][3][4]=1;stage[3][3][5]=1;
    stage[3][4][3]=1;stage[3][4][4]=6;stage[3][4][5]=1;
    stage[3][5][3]=1;stage[3][5][4]=1;stage[3][5][5]=1;
    stage[3][9][0]=1;stage[3][9][1]=1;stage[3][9][2]=1;stage[3][9][3]=1;stage[3][9][4]=1;stage[3][9][5]=1;stage[3][9][6]=1;stage[3][9][7]=1;stage[3][9][8]=1;
    stage[3][10][0]=1;stage[3][10][1]=1;stage[3][10][2]=1;stage[3][10][3]=1;stage[3][10][4]=1;stage[3][10][5]=1;stage[3][10][6]=1;stage[3][10][7]=1;stage[3][10][8]=1;
    stage[3][11][0]=1;stage[3][11][1]=1;stage[3][11][2]=1;stage[3][11][3]=1;stage[3][11][4]=1;stage[3][11][5]=1;stage[3][11][6]=1;stage[3][11][7]=1;stage[3][11][8]=1;
    stage[3][12][3]=1;stage[3][12][4]=1;stage[3][12][5]=1;
    stage[3][13][3]=1;stage[3][13][4]=2;stage[3][13][5]=1;
    stage[3][14][3]=1;stage[3][14][4]=1;stage[3][14][5]=1;

    target[3][0]=13;target[3][1]=4;
  }

  ~Stage()
  {
    free(rect1);
    free(rect2);
    free(circle);
    free(rect3);
    free(rect4);
    free(rect5);
  }

  void checkTouch(int x1,int y1,int x2,int y2)
  {
    if(level==2)
    {
      if((x1==2 && y1==5) || (x2==2 && y2==5))
      {
        stage[1][4][3]=(stage[1][4][3]+1)%2;
        stage[1][5][3]=(stage[1][5][3]+1)%2;
      }
      if((x1==8 && y1==6) || (x2==8 && y2==6))
      {
        if(flag_stand==1)
        {
        stage[1][10][3]=(stage[1][10][3]+1)%2;
        stage[1][11][3]=(stage[1][11][3]+1)%2;
        }
      }
    }
    else if(level==3)
    {
      if(flag_stand==1)
      {
        if(stage[2][x1][y1]==5)
        {
          flag_fall=1;
          miss++;
          fall_lvl3=1;
          lvl3_x=x1;
          lvl3_y=y1;
          zs2=0;
        }
      }
      
    }
    else if(level==4)
    {
      if(flag_stand==1)
      {
        if(stage[3][x1][y1]==6)
        {
          flag_attach=0;
          flag_stand=0;
          flag_shift=1;
        }
      }
    }
  }

  void createStage1()
  {
    static const GLfloat vertex_buffer_data [] = {
      -5,-5,0, // vertex 1
      -5,5,0, // vertex 2
      0,0,0, // vertex 3

      -5,5,0,
      5,5,0,
      0,0,0,

      5,5,0,
      5,-5,0,
      0,0,0,

      5,-5,0, // vertex 3
      -5,-5,0, // vertex 4
      0,0,0,  // vertex 1
      };

      static const GLfloat color_buffer_data [] = {
      0.6,0.6,0.6, // color 1
      0.6,0.6,0.6, // color 2
      0.8,0.8,0.8, // color 3

      0.6,0.6,0.6, // color 1
      0.6,0.6,0.6, // color 2
      0.8,0.8,0.8, // color 3

      0.6,0.6,0.6, // color 1
      0.6,0.6,0.6, // color 2
      0.8,0.8,0.8, // color 3

      0.6,0.6,0.6, // color 1
      0.6,0.6,0.6, // color 2
      0.8,0.8,0.8, // color 3

      };

      static const GLfloat color_buffer_data1 [] = {
      0.8,0.30,0.11, // color 1
      0.8,0.30,0.11, // color 1
      1,0.50,0.31, // color 1

      0.8,0.30,0.11, // color 1
      0.8,0.30,0.11, // color 1
      1,0.50,0.31, // color 1

      0.8,0.30,0.11, // color 1
      0.8,0.30,0.11, // color 1
      1,0.50,0.31, // color 1

      0.8,0.30,0.11, // color 1
      0.8,0.30,0.11, // color 1
      1,0.50,0.31, // color 1

      };

    rect1 = create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
    rect4 = create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data1, GL_FILL);

  }
  void createStage2()
  {
    static const GLfloat vertex_buffer_data [] ={
      -5,0,-1,
      -5,0,1,
      0,0,0,

      -5,0,1,
      5,0,1,
      0,0,0,

      5,0,1,
      5,0,-1,
      0,0,0,

      5,0,-1,
      -5,0,-1,
      0,0,0,

    };

    static const GLfloat color_buffer_data [] = {
      0.6,0.6,0.6, // color 1
      0.6,0.6,0.6, // color 2
      0.8,0.8,0.8, // color 3

      0.6,0.6,0.6, // color 1
      0.6,0.6,0.6, // color 2
      0.8,0.8,0.8, // color 3

      0.6,0.6,0.6, // color 1
      0.6,0.6,0.6, // color 2
      0.8,0.8,0.8, // color 3

      0.6,0.6,0.6, // color 1
      0.6,0.6,0.6, // color 2
      0.8,0.8,0.8, // color 3
    };

      static const GLfloat color_buffer_data1 [] = {
      0.8,0.30,0.11, // color 1
      0.8,0.30,0.11, // color 1
      1,0.50,0.31, // color 1

      0.8,0.30,0.11, // color 1
      0.8,0.30,0.11, // color 1
      1,0.50,0.31, // color 1

      0.8,0.30,0.11, // color 1
      0.8,0.30,0.11, // color 1
      1,0.50,0.31, // color 1

      0.8,0.30,0.11, // color 1
      0.8,0.30,0.11, // color 1
      1,0.50,0.31, // color 1
      };

    rect2 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);
    rect5 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data1,GL_FILL);
  }
  void createRectangle()
  {
    static const GLfloat vertex_buffer_data [] = {
      -4.5+1,-4.5-1,0,
      -4.5-1,-4.5+1,0,
      4.5-1,4.5+1,0,

      4.5-1,4.5+1,0,
      4.5+1,4.5-1,0,
      -4.5+1,-4.5-1,0,
    };

    static const GLfloat color_buffer_data [] = {
      0,0,0,
      0,0,0,
      0,0,0,

      0,0,0,
      0,0,0,
      0,0,0,
    };
    rect3 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);
  }
  void createCircle()
  {
    static GLfloat vertex_buffer_data[180*9];
    static GLfloat color_buffer_data[180*9];

    for(int i=0;i<180;i++)
    {
      vertex_buffer_data[9*i+0]=0;
      vertex_buffer_data[9*i+1]=0;
      vertex_buffer_data[9*i+2]=0;

      vertex_buffer_data[9*i+3]=4.5*cos(i*M_PI/180);
      vertex_buffer_data[9*i+4]=4.5*sin(i*M_PI/180);
      vertex_buffer_data[9*i+5]=0;

      vertex_buffer_data[9*i+6]=4.5*cos((i+1)*M_PI/180);
      vertex_buffer_data[9*i+7]=4.5*sin((i+1)*M_PI/180);
      vertex_buffer_data[9*i+8]=0;
    }

    for(int i=0;i<180;i++)
    {
      color_buffer_data[9*i+0]=0;
      color_buffer_data[9*i+1]=0;
      color_buffer_data[9*i+2]=0;

      color_buffer_data[9*i+3]=0;
      color_buffer_data[9*i+4]=0;
      color_buffer_data[9*i+5]=0;

      color_buffer_data[9*i+6]=0;
      color_buffer_data[9*i+7]=0;
      color_buffer_data[9*i+8]=0;

    }

    circle = create3DObject(GL_TRIANGLES,180*3,vertex_buffer_data,color_buffer_data,GL_FILL);

  }

  void drawStage(float x1,float y1,float z1,int type)
  {

    glm::mat4 translateNet = glm::translate(glm::vec3(x1,y1,z1));

    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangle = glm::translate (glm::vec3(0, 0, 3));        // glTranslatef
    glm::mat4 rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    if(type==5)
      draw3DObject(rect4);
  else
      draw3DObject(rect1);

    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(0, 0, 1));        // glTranslatef
    rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    if(type==5)
      draw3DObject(rect4);
  else
      draw3DObject(rect1);

    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(0, -5, 2));        // glTranslatef
    rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    if(type==5)
      draw3DObject(rect5);
  else
      draw3DObject(rect2);

    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(0, 5, 2));        // glTranslatef
    rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateNet * translateRectangle * rotateRectangle);  
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    if(type==5)
      draw3DObject(rect5);
  else
      draw3DObject(rect2);

    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(-5, 0, 2));        // glTranslatef
    rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    if(type==5)
      draw3DObject(rect5);
  else
      draw3DObject(rect2);

    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(5, 0, 2));        // glTranslatef
    rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    if(type==5)
      draw3DObject(rect5);
  else
      draw3DObject(rect2);

    if(type==3)
    {
      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(0, 0, 3.2));        // glTranslatef
      rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(circle);
      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(0, 0, 3.2));        // glTranslatef
      rotateRectangle = glm::rotate((float)(180*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(circle);

    }

    if(type==4)
    {
      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(0, 0, 3.2));        // glTranslatef
      rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(rect3);

      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(0, 0, 3.2));        // glTranslatef
      rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(rect3);

    }
    if(type==6)
    {
      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(-1, 0, 3.2));        // glTranslatef
      rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(circle);
      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(1, 0, 3.2));        // glTranslatef
      rotateRectangle = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(circle);

    }
  }

  void animateStage()
  {
    for(int i=0;i<15;i++)
    {
      for(int j=0;j<10;j++)
      {
        if(stage[level-1][i][j]==1|| stage[level-1][i][j]==3|| stage[level-1][i][j]==4 || stage[level-1][i][j]==5 || stage[level-1][i][j]==6)
        {
        if ((lvl3_x==i && lvl3_y==j) && fall_lvl3==1)
        {
          if(zs2<-55)
          {
            fall_lvl3=0;
            zs2=0;
            continue;
          }
            drawStage(initx+(i-8)*10,inity+(j-5)*10,zs2,stage[level-1][i][j]);
          if(fall_call==1)
          {
          zs2-=5;
          } 
        }
        else
        {
          if(start_stage==1)
          {
        if(fall_call==1)
          flag++;
        if(flag<7)
          return;

            if(zs+2*i+3*j>0)
            {
              drawStage(initx+(i-8)*10,inity+(j-5)*10,0,stage[level-1][i][j]);
            }
            else
              drawStage(initx+(i-8)*10,inity+(j-5)*10,zs+2*i+3*j,stage[level-1][i][j]);

          }
          else if(end_stage==1 && flag_complete==0)
          {

            if(zs-2*(14-i)-3*(9-j)<-90)
            {
              drawStage(initx+(i-8)*10,inity+(j-5)*10,-100,stage[level-1][i][j]);
            }
            else
              drawStage(initx+(i-8)*10,inity+(j-5)*10,zs-2*(14-i)-3*(9-j),stage[level-1][i][j]);

          }
          else
          {
            drawStage(initx+(i-8)*10,inity+(j-5)*10,zs,stage[level-1][i][j]);
          }
        }
       }
      }
    }
    if(start_stage==1)
    {
      if(flag<5)
        return;
      stage[1][4][3]=0;
      stage[1][5][3]=0;
      stage[1][10][3]=0;
      stage[1][11][3]=0;


      flag_attach=1;
        if(zs<0)
          zs+=2;
      else if(zs>=0)
      {
       zs=0;
        start_stage=0;
      }
    }
    if(end_stage==1)
    {
      if(flag_complete==1)
      {
      system("mpg123 -vC level_up.mp3 &");

      if(level<max_level)
        level++;
      else
      {
        flag_gameover=1;
      }
        start_stage=1;
        zoom=26;
        v=0;
      Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
        zs=-50;
        flag=0;
        end_stage=0;
        flag_complete=0;
        fall_lvl3=0;
      }
      else
      {
        if(zs<-50)
        {

        start_stage=1;
        zoom=26;
        v=0;
      Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
        flag=0;
        zs=-50;         
        end_stage=0;
        }
        else
        {
          zs-=2;
        }

      }
    }

  }


}stage;


class Block{
public:
  int cube1i,cube1j,cube1k,cube2i,cube2j,cube2k,zs1,zs2,flag_check,flag_blockOpt,flag_animate,type;
  VAO *cube;
  glm::mat4 animate;
  int move_flag;

public:

  Block()
  {
    cube1i=1;
    cube1j=6;
    cube1k=0;
    zs1=0;
    zs2=0;
    cube2i=1;
    cube2j=6;
    cube2k=10;
    flag_attach=1;
    type=0;
    flag_animate=0;
    flag_check=0;
    move_flag=0;
    flag_blockOpt=0;
  }
  ~Block()
  {
    free(cube);
  }

  void createSquare ()
  {
  // GL3 accepts only Triangles. Quads are not supported
      static const GLfloat vertex_buffer_data [] = {
      -5,-5,0, // vertex 1
      -5,5,0, // vertex 2
      0,0,0, // vertex 3

      -5,5,0,
      5,5,0,
      0,0,0,

      5,5,0,
      5,-5,0,
      0,0,0,

      5,-5,0, // vertex 3
      -5,-5,0, // vertex 4
      0,0,0,  // vertex 1
      };

      static const GLfloat color_buffer_data [] = {
      102.0/255,0.0/255,0.0/255,
      102.0/255,0.0/255,0.0/255,
      178.0/255,34.0/255,34.0/255,

      102.0/255,0.0/255,0.0/255,
      102.0/255,0.0/255,0.0/255,
      178.0/255,34.0/255,34.0/255,

      102.0/255,0.0/255,0.0/255,
      102.0/255,0.0/255,0.0/255,
      178.0/255,34.0/255,34.0/255,

      102.0/255,0.0/255,0.0/255,
      102.0/255,0.0/255,0.0/255,
      178.0/255,34.0/255,34.0/255,

      };

  // create3DObject creates and returns a handle to a VAO that can be used later
      cube =  create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
  }

  void drawCube(float x1,float y1,float z1,int number)
  {

      glm::mat4 translateNet = glm::translate(glm::vec3(x1,y1,z1));

      Matrices.model = glm::mat4(1.0f);
      glm::mat4 translateRectangle = glm::translate (glm::vec3(0, 0, 13));        // glTranslatef
      glm::mat4 rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      if(flag_animate==1 && flag_attach==1)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
      else if(flag_animate==1 && flag_attach==0 && flag_blockOpt==number)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
    else        
        Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(cube);

      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(0, 0, 3));        // glTranslatef
      rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      if(flag_animate==1 && flag_attach==1)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
      else if(flag_animate==1 && flag_attach==0 && flag_blockOpt==number)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
    else        
        Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(cube);

      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(0, -5, 8));        // glTranslatef
      rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
      if(flag_animate==1 && flag_attach==1)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
      else if(flag_animate==1 && flag_attach==0 && flag_blockOpt==number)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
    else        
        Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(cube);

      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(0, 5, 8));        // glTranslatef
      rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
      if(flag_animate==1 && flag_attach==1)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
      else if(flag_animate==1 && flag_attach==0 && flag_blockOpt==number)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
    else        
        Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(cube);

      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(-5, 0, 8));        // glTranslatef
      rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
      if(flag_animate==1 && flag_attach==1)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
      else if(flag_animate==1 && flag_attach==0 && flag_blockOpt==number)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
    else        
        Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(cube);

      Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(5, 0, 8));        // glTranslatef
      rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
      if(flag_animate==1 && flag_attach==1)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
      else if(flag_animate==1 && flag_attach==0 && flag_blockOpt==number)
        Matrices.model *= (animate * translateNet * translateRectangle * rotateRectangle);
    else        
        Matrices.model *= (translateNet * translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(cube);

  }

  void initiateVariables(int level)
  {
          cube1i=stage.start[level-1][0];
          cube1j=stage.start[level-1][1];
          cube2i=stage.start[level-1][0];
          cube2j=stage.start[level-1][1];
          cube1k=0;
          cube2k=10;
          flag_attach=1;
          flag_blockOpt=0;
          flag_stand=1;
          fall_lvl3=0;
  }

  void animateCube()
  {
  glm::mat4 translate1,rotate,translate2;   
    if(flag_animate==1 && flag_attach==1)
    { 
      if(type==1)
      {
        if(cube1i>=cube2i)
        {
            translate1 = glm::translate (glm::vec3((cube2i-8)*10, -5,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(-((cube2i-8)*10), 5,-3));        // glTranslatef
            }
            else
            {
            translate1 = glm::translate (glm::vec3((cube1i-8)*10, -5,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(-((cube1i-8)*10), 5,-3));        // glTranslatef
            }
            rotate = glm::rotate((float)(zs2*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
            animate=translate1*rotate*translate2;

        if(zs2<=-90)
        {
        flag_animate=0;
        fall_call=1;
        type=0;
        zs2=0;          
        }
        else
        {
        zs2-=5;
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
        }

      }
      else if(type==2)
      {
        if(cube1i>=cube2i)
        {
            translate1 = glm::translate (glm::vec3(10+(cube1i-8)*10, -5,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(-(10+(cube1i-8)*10), 5,-3));        // glTranslatef
            }
            else
            {
            translate1 = glm::translate (glm::vec3(10+(cube2i-8)*10, -5,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(-(10+(cube2i-8)*10), 5,-3));        // glTranslatef
            }
            rotate = glm::rotate((float)(zs2*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
            animate=translate1*rotate*translate2;

        if(zs2>=90)
        {
        flag_animate=0;
        fall_call=1;
        type=0;
        zs2=0;          
        }
        else
        {
        zs2+=5;
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
        }
      }
      else if(type==3)
      {
        if(cube1j>=cube2j)
        {
            translate1 = glm::translate (glm::vec3(-5,10+(cube1j-5)*10,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(5,-(10+(cube1j-5)*10),-3));        // glTranslatef
            }
            else
            {
            translate1 = glm::translate (glm::vec3(-5,10+(cube2j-5)*10,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(5,-(10+(cube2j-5)*10),-3));        // glTranslatef
            }
            rotate = glm::rotate((float)(zs2*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
            animate=translate1*rotate*translate2;

        if(zs2<=-90)
        {
        flag_animate=0;
        fall_call=1;
        type=0;
        zs2=0;          
        }
        else
        {
        zs2-=5;
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
        }
      }
      else if(type==4)
      {
        if(cube1j>=cube2j)
        {
            translate1 = glm::translate (glm::vec3(-5,(cube2j-5)*10,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(5,-((cube2j-5)*10),-3));        // glTranslatef
            }
            else
            {
            translate1 = glm::translate (glm::vec3(-5,(cube1j-5)*10,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(5,-((cube1j-5)*10),-3));        // glTranslatef
            }
            rotate = glm::rotate((float)(zs2*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
            animate=translate1*rotate*translate2;

        if(zs2>=90)
        {
        flag_animate=0;
        fall_call=1;
        type=0;
        zs2=0;          
        }
        else
        {
        zs2+=5;
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
        }
      }
      if(flag_animate==1)
        return;
    }
    else if(flag_animate==1 && flag_attach==0)
    {
      if(type==1)
      {
        if(flag_blockOpt==1)
        {
            translate1 = glm::translate (glm::vec3((cube2i-8)*10, -5,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(-((cube2i-8)*10), 5,-3));        // glTranslatef
            }
            else
            {
            translate1 = glm::translate (glm::vec3((cube1i-8)*10, -5,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(-((cube1i-8)*10), 5,-3));        // glTranslatef
            }
            rotate = glm::rotate((float)(zs2*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
            animate=translate1*rotate*translate2;

        if(zs2<=-90)
        {
        flag_animate=0;
        fall_call=1;
        type=0;
        zs2=0;          
        }
        else
        {
        zs2-=5;
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,1);
        }


      }
      else if(type==2)
      {
        if(flag_blockOpt==0)
        {
            translate1 = glm::translate (glm::vec3(10+(cube1i-8)*10, -5,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(-(10+(cube1i-8)*10), 5,-3));        // glTranslatef
            }
            else
            {
            translate1 = glm::translate (glm::vec3(10+(cube2i-8)*10, -5,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(-(10+(cube2i-8)*10), 5,-3));        // glTranslatef
            }
            rotate = glm::rotate((float)(zs2*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
            animate=translate1*rotate*translate2;

        if(zs2>=90)
        {
        flag_animate=0;
        fall_call=1;
        type=0;
        zs2=0;          
        }
        else
        {
        zs2+=5;
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,1);
        }

      }
      else if(type==3)
      {
        if(flag_blockOpt==0)
        {
            translate1 = glm::translate (glm::vec3(-5,10+(cube1j-5)*10,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(5,-(10+(cube1j-5)*10),-3));        // glTranslatef
            }
            else
            {
            translate1 = glm::translate (glm::vec3(-5,10+(cube2j-5)*10,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(5,-(10+(cube2j-5)*10),-3));        // glTranslatef
            }
            rotate = glm::rotate((float)(zs2*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
            animate=translate1*rotate*translate2;

        if(zs2<=-90)
        {
        flag_animate=0;
        fall_call=1;
        type=0;
        zs2=0;          
        }
        else
        {
        zs2-=5;
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,1);
        }

      }
      else if(type==4)
      {
        if(flag_blockOpt==1)
        {
            translate1 = glm::translate (glm::vec3(-5,(cube2j-5)*10,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(5,-((cube2j-5)*10),-3));        // glTranslatef
            }
            else
            {
            translate1 = glm::translate (glm::vec3(-5,(cube1j-5)*10,3));        // glTranslatef
            translate2 = glm::translate (glm::vec3(5,-((cube1j-5)*10),-3));        // glTranslatef
            }
            rotate = glm::rotate((float)(zs2*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
            animate=translate1*rotate*translate2;

        if(zs2>=90)
        {
        flag_animate=0;
        fall_call=1;
        type=0;
        zs2=0;          
        }
        else
        {
        zs2+=5;
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,1);
        }

      }
      if(flag_animate==1)
        return;
    }

    if(fall_call==0 && stage.start_stage==0 && stage.end_stage==0)
    {
      if(flag_attach==1)
      {
      drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
      drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
      }
      else
      {
        if(flag_blockOpt==0)
        {
          drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
          drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k,0);
        }
        else
        {
            drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k,0);
          drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
        }
      }
      return;
    }
    else
    {
      fall_call=0;
    }
      if(flag_fallcomp==1)
      {
          if(zs1<-35)
          {
            flag_fallcomp=0;
            zs1=0;
              stage.end_stage=1;
              if(stage.level+1<=max_level)
                initiateVariables(stage.level+1);
            else
            {
              flag_gameover=1;
            }
            return;
          }
          else
          {
              drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
              drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
              zs1-=5;
              return;
          }

      }
      if(flag_fall==1)
      {
        if(flag_stand==1)
        {
          if(zs1<-55)
          {
            initiateVariables(stage.level);
            flag_fall=0;
            if(miss>=miss_limit)
            {
              flag_gameover=1;
            }
            zs1=0;
              stage.end_stage=1;
            return;
          }
          else
          {
              drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
              drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
              zs1-=5;
              return;
          }
        }
        else if(flag_attach==1)
        {
          if((stage.stage[stage.level-1][cube1i][cube1j]==0 && stage.stage[stage.level-1][cube2i][cube2j]==0) )
          {
            if(zs1<-55)
            {
              zs1=0;
              if(miss>=miss_limit)
              {
                flag_gameover=1;
              }
              initiateVariables(stage.level);
              flag_fall=0;
                stage.end_stage=1;
              return;
            }
            else
            {
                drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
                drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
                zs1-=5;
                return;
            }
          }
          else if(stage.stage[stage.level-1][cube1i][cube1j]==0 || cube1i<0 || cube1i>14 || cube1j<0 || cube1j>9)
          {
            cube2i=cube1i;cube2j=cube1j;
            cube2k=10;
            zs1=-10;
            drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
              drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
              flag_stand=1;
              return;
          }
          else
          {
            cube1i=cube2i;cube1j=cube2j;
            cube1k=10;
            zs1=-10;
            drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
              drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
              flag_stand=1;
              return;
          }
        }
        else{
          if(flag_blockOpt==0)
          {
            if(zs1<-55)
            {
              zs1=0;
              if(miss>=miss_limit)
              {
                flag_gameover=1;
              }
              initiateVariables(stage.level);
              flag_fall=0;
              flag_attach=1;
                stage.end_stage=1;
              return;
            }
            else
            {
                drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k+zs1,0);
                drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k,0);
                zs1-=5;
                return;
            }

          }
          else
          {
            if(zs1<-55)
            {
              zs1=0;
              if(miss>=miss_limit)
              {
                flag_gameover=1;
              }
              initiateVariables(stage.level);
              flag_fall=0;
              flag_attach=1;
                stage.end_stage=1;
              return;
            }
            else
            {
                drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k,0);
                drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k+zs1,0);
                zs1-=5;
                return;
            }

          }
        }
      }


    if(stage.start_stage==0 && stage.end_stage==0)
    {
      if(flag_attach==1)
      {
        if(move_flag==1)
        {
          if(flag_stand==1)
          {
            if(cube1k<cube2k)
            {
              cube1i-=1;
              cube2i-=2;
              cube2k=0;
              flag_stand=0;
            }
            else
            {
              cube1i-=2;
              cube2i-=1;
              cube1k=0;
              flag_stand=0;
            }
          }
          else
          {
            if(cube1i<cube2i)
            {
              cube1i-=1;
              cube2i-=2;
              flag_stand=1;
              cube2k=10;
            }
            else if(cube1i>cube2i)
            {
              cube1i-=2;
              cube2i-=1;
              flag_stand=1;
              cube1k=10;
            }
            else
            {
              cube1i-=1;
              cube2i-=1;
            }
          }
          move_flag=0;
        }
        else if(move_flag==2)
        {
          if(flag_stand==1)
          {
            if(cube1k<cube2k)
            {
              cube1i+=1;
              cube2i+=2;
              cube2k=0;
              flag_stand=0;
            }
            else
            {
              cube1i+=2;
              cube2i+=1;
              cube1k=0;
              flag_stand=0;
            }
          }
          else
          {
            if(cube1i<cube2i)
            {
              cube1i+=2;
              cube2i+=1;
              flag_stand=1;
              cube1k=10;
            }
            else if(cube1i>cube2i)
            {
              cube1i+=1;
              cube2i+=2;
              flag_stand=1;
              cube2k=10;
            }
            else
            {
              cube1i+=1;
              cube2i+=1;
            }
          }
          move_flag=0;
        }
        else if(move_flag==3)
        {
          if(flag_stand==1)
          {
            if(cube1k<cube2k)
            {
              cube1j+=1;
              cube2j+=2;
              cube2k=0;
              flag_stand=0;
            }
            else
            {
              cube1j+=2;
              cube2j+=1;
              cube1k=0;
              flag_stand=0;
            }
          }
          else
          {
            if(cube1j<cube2j)
            {
              cube1j+=2;
              cube2j+=1;
              flag_stand=1;
              cube1k=10;
            }
            else if(cube1j>cube2j)
            {
              cube1j+=1;
              cube2j+=2;
              flag_stand=1;
              cube2k=10;
            }
            else
            {
              cube1j+=1;
              cube2j+=1;
            }
          }
          move_flag=0;
        }
        else if(move_flag==4)
        {
          if(flag_stand==1)
          {
            if(cube1k<cube2k)
            {
              cube1j-=1;
              cube2j-=2;
              cube2k=0;
              flag_stand=0;
            }
            else
            {
              cube1j-=2;
              cube2j-=1;
              cube1k=0;
              flag_stand=0;
            }
          }
          else
          {
            if(cube1j<cube2j)
            {
              cube1j-=1;
              cube2j-=2;
              flag_stand=1;
              cube2k=10;
            }
            else if(cube1j>cube2j)
            {
              cube1j-=2;
              cube2j-=1;
              flag_stand=1;
              cube1k=10;
            }
            else
            {
              cube1j-=1;
              cube2j-=1;
            }
          }
          move_flag=0;
        }
      }
      else
      {
        if(flag_shift==1)
        {
        cube1i=10;
          cube2i=10;
          cube2j=1;
          cube1j=7;
          cube1k=0;
          cube2k=0;
          flag_shift=0;
        
        }
        if(flag_blockOpt==0)
        {
          if(move_flag==1)
          {
            cube1i--;
            move_flag=0;
          }
          else if(move_flag==2)
          {
            cube1i++;
            move_flag=0;
          }
          else if(move_flag==3)
          {
            cube1j++;
            move_flag=0;
          }
          else if(move_flag==4)
          {
            cube1j--;
            move_flag=0;
          }

          if(stage.stage[stage.level-1][cube1i][cube1j]==0)
          {
            flag_fall=1;
            miss++;
            zs1=0;
          }


        }
        else if(flag_blockOpt==1)
        {
          if(move_flag==1)
          {
            cube2i--;
            move_flag=0;
          }
          else if(move_flag==2)
          {
            cube2i++;
            move_flag=0;
          }
          else if(move_flag==3)
          {
            cube2j++;
            move_flag=0;
          }
          else if(move_flag==4)
          {
            cube2j--;
            move_flag=0;
          }

          if(stage.stage[stage.level-1][cube2i][cube2j]==0)
          {
            flag_fall=1;
            miss++;
            zs1=0;
          }
        }

        if(abs(cube1i-cube2i)==1 && abs(cube1j-cube2j)==0)
        {
          flag_attach=1;
        }
        else if(abs(cube1i-cube2i)==0 && abs(cube1j-cube2j)==1)
        {
          flag_attach=1;
        }

      }

    }

      if(flag_attach==1)
      {
        if(flag_stand==1)
        {
          if(stage.stage[stage.level-1][cube1i][cube1j]==2 || stage.stage[stage.level-1][cube1i][cube1j]==0)
          {
            if(stage.target[stage.level-1][0]==cube1i && stage.target[stage.level-1][1]==cube1j)
            {
              flag_complete=1;
              flag_fallcomp=1;
              zs1=0;
            }
            else
            {
              flag_fall=1;
              miss++;
              zs1=0;
            }
          }
        }
        else
        {
          if(stage.stage[stage.level-1][cube1i][cube1j]==0 || stage.stage[stage.level-1][cube2i][cube2j]== 0)
          {
            if(stage.stage[stage.level-1][cube1i][cube1j]==0 && stage.stage[stage.level-1][cube2i][cube2j]== 0)
            {
            flag_fall=1;
            miss++;
            zs1=0;
            }
            else
            {
            drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k,0);
            drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k,0);
                flag_fall=1;
                miss++;
                zs1=0;

            }
          }
        }
      } 

      if(cube1i<0 || cube2i<0 || cube1j<0 || cube2j<0 || cube1i>14 || cube2i>14 || cube1j>9 || cube2j>9)
      {
        if(flag_fall!=1)
        {
    drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k,0);
        flag_fall=1;
        miss++;
        zs1=0;
        }
      }
      if(stage.start_stage==0 && stage.end_stage==0)
      {
        drawCube(5+(cube1i-8)*10,5+(cube1j-5)*10,cube1k,0);
        drawCube(5+(cube2i-8)*10,5+(cube2j-5)*10,cube2k,0);
      }
      if(flag_check==1)
      {
    stage.checkTouch(cube1i,cube1j,cube2i,cube2j);
    flag_check=0;
    }
  }

}block;


VAO *rect1,*rect2;

void createRectangle()
{
  static const GLfloat vertex_buffer_data [] = {
    -2,-0.5,0,
    -2,0.5,0,
    2,0.5,0,

    2,0.5,0,
    2,-0.5,0,
    -2,-0.5,0,
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0,
    0,0,0,
    0,0,0,

    0,0,0,
    0,0,0,
    0,0,0,
  };
  rect1 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);

}

void createRectangle2()
{
  static const GLfloat vertex_buffer_data [] = {
    -7,-5,0,
    -7,5,0,
    7,5,0,

    7,5,0,
    7,-5,0,
    -7,-5,0,
  };

  static const GLfloat color_buffer_data [] = {
    0.5,0.5,1,
    0.5,0.5,1,
    0.5,0.5,1,

    0.5,0.5,1,
    0.5,0.5,1,
    0.5,0.5,1,
  };
  rect2 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);

}

double x_g,y_g;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                break;
            case GLFW_KEY_P:
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            case GLFW_KEY_LEFT:
              break;
            case GLFW_KEY_RIGHT:
              break;
            case GLFW_KEY_UP:
              break;
            case GLFW_KEY_DOWN:
              break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_LEFT:
              if(flag_move==1 && flag_fall==0 && flag_fallcomp==0 && block.flag_animate==0)
              {
                if(flag_gameover==0 && stage.start_stage==0 && stage.end_stage==0)
                {
                block.move_flag=1;
                  moves++;
                block.flag_check=1;
                block.flag_animate=1;
                block.type=1;
                flag_move=0;
                }
              }
              break;
            case GLFW_KEY_RIGHT:
              if(flag_move==1 && flag_fall==0 && flag_fallcomp==0 && block.flag_animate==0)
              {
                if(flag_gameover==0 && stage.start_stage==0 && stage.end_stage==0)
                {
                  moves++;
                block.move_flag=2;
                block.type=2;
                block.flag_check=1;
                block.flag_animate=1;
                flag_move=0;
                }
              }
              break;
            case GLFW_KEY_UP:
              if(flag_move==1 && flag_fall==0 && flag_fallcomp==0 && block.flag_animate==0)
              {
                if(flag_gameover==0 && stage.start_stage==0 && stage.end_stage==0)
                {
                  moves++;
                block.move_flag=3;
                block.type=3;
                block.flag_check=1;
                block.flag_animate=1;
                flag_move=0;

                }
              }
              break;
            case GLFW_KEY_DOWN:
              if(flag_move==1 && flag_fall==0 && flag_fallcomp==0 && block.flag_animate==0)
              {
                if(flag_gameover==0 && stage.start_stage==0 && stage.end_stage==0)
                {
                  moves++;
                block.move_flag=4;
                block.type=4;
                block.flag_check=1;
                block.flag_animate=1;
                flag_move=0;
                }
              }
              break;
            case GLFW_KEY_SPACE:
              if(flag_fall==0)
              {
                block.flag_blockOpt=(block.flag_blockOpt+1)%2;
              }
              break;

            case GLFW_KEY_V:
              v=(v+1)%6;
              if(v==0)
              {
          Matrices.view = glm::lookAt(glm::vec3(-22,-43,29), glm::vec3(-10,0,0), glm::vec3(0,0,1)); // preview view
              } 
              else if(v==1)
              {
          Matrices.view = glm::lookAt(glm::vec3(-10,0,29), glm::vec3(-10,0,0), glm::vec3(0,1,0)); // top view

              }
              else if(v==2)
              {
          Matrices.view = glm::lookAt(glm::vec3(-22,-43,19), glm::vec3(-10,0,0), glm::vec3(0,0,1)); // tower view               
              }
              else if(v==3)             //block view
              {
                if(block.cube1i<=block.cube2i)
              Matrices.view = glm::lookAt(glm::vec3((block.cube1i-8)*10,0,19), glm::vec3(-10,0,0), glm::vec3(0,0,1)); // top view               
                else
              Matrices.view = glm::lookAt(glm::vec3((block.cube2i-8)*10,0,19), glm::vec3(-10,0,0), glm::vec3(0,0,1)); // top view                                 
              }
              else if(v==5)
              {
                x_g=-22;
                y_g=-43;
              }
              break;
            default:
                break;
        }
    }
}


/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
    case 'Q':
    case 'q':
            quit(window);
            break;
    default:
      break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
            {

            }
            else if(action == GLFW_PRESS)
            {
                glfwGetCursorPos(window, &xpos, &ypos);
                x_g=(xpos-400)*1.0*3/10;
                y_g=(350-ypos)*1.0/3.5;
                if(flag_gameover==0)
                {
          if(x_g>=-118 && x_g<=-104 && y_g>=88 && y_g<=98)
          {
            flag_gameover=1;
            flag_gamestart=1;
          }
          else if(flag_move==1)
          {
            if(y_g>=10+(block.cube1j-5)*10 && y_g>=10+(block.cube2j-5)*10)
            {
              if(block.cube1i>=block.cube2i && x_g<=10+(block.cube1i-8)*10 && x_g>=(block.cube2i-8)*10)
              {
              block.move_flag=3;
              flag_move=0;
              block.flag_check=1;
//              block.flag_animate=1;

              }
              else if(block.cube2i>block.cube1i && x_g<=10+(block.cube2i-8)*10 && x_g>=(block.cube1i-8)*10)
              {
              block.move_flag=3;
              flag_move=0;
              block.flag_check=1;
//              block.flag_animate=1;

              }
            }
            else if(y_g<=(block.cube1j-5)*10 && y_g<=(block.cube2j-5)*10)
            {
              if(block.cube1i>=block.cube2i && x_g<=10+(block.cube1i-8)*10 && x_g>=(block.cube2i-8)*10)
              {
              block.move_flag=4;
              flag_move=0;
              block.flag_check=1;
//              block.flag_animate=1;

              }
              else if(block.cube2i>block.cube1i && x_g<=10+(block.cube2i-8)*10 && x_g>=(block.cube1i-8)*10)
              {
              block.move_flag=4;
              flag_move=0;
              block.flag_check=1;
//              block.flag_animate=1;
                
              }
            }
            else if(x_g<=(block.cube1i-8)*10 && x_g<=(block.cube2i-8)*10)
            {
              if(block.cube1j>=block.cube2j && y_g<=10+(block.cube1j-5)*10 && y_g>=(block.cube2j-5)*10)
              {
              block.move_flag=1;
              flag_move=0;
              block.flag_check=1;
//              block.flag_animate=1;
              }
              else if(block.cube1j<block.cube2j && y_g<=10+(block.cube2j-5)*10 && y_g>=(block.cube1j-5)*10)
              {
              block.move_flag=1;
              flag_move=0;
              block.flag_check=1;
//              block.flag_animate=1;

              }
            }
            else if(x_g>=10+(block.cube1i-8)*10 && x_g>=10+(block.cube2i-8)*10)
            {
              if(block.cube1j>=block.cube2j && y_g<=10+(block.cube1j-5)*10 && y_g>=(block.cube2j-5)*10)
              {
              block.move_flag=2;
              flag_move=0;
              block.flag_check=1;
//              block.flag_animate=1;
              }
              else if(block.cube1j<block.cube2j && y_g<=10+(block.cube2j-5)*10 && y_g>=(block.cube1j-5)*10)
              {
              block.move_flag=2;
              flag_move=0;
              block.flag_check=1;
//              block.flag_animate=1;

              }
            }
          }                 
                }
                else if(flag_gameover==1 && flag_gamestart==0)
                {
                  if(x_g>=-6 && x_g<=8 && y_g>=-18 && y_g<=-8)
                  {
                    flag_gamestart=1;
                  }
                }


            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
              flag_hover=0;
            }
            else if(action == GLFW_PRESS)
            {
                glfwGetCursorPos(window, &xpos, &ypos);
                x_g=(xpos-400)*1.0*3/10;
                y_g=(350-ypos)*1.0/3.5;
              flag_hover=1;
            }
            break;
        default:
            break;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if(yoffset>0 && zoom<40)
    zoom+=yoffset*2;
  if(yoffset<0 && zoom>=2)
    zoom+=yoffset*2;
    Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);

}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  // set the projection matrix as perspective
  /* glMatrixMode (GL_PROJECTION);
     glLoadIdentity ();
     gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
  // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
//     Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
}


void draw_rect(float x,float y,float rotation)
{
  glm::mat4 VP1,view1,p;
    p = glm::ortho(-120.0f, 120.0f, -100.0f, 100.0f, 0.1f, 120.0f);
    view1 = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    VP1 = p * view1;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 rotateRect = glm::rotate((float)(rotation*M_PI/180.0f), glm::vec3(0,0,1)); 
    glm::mat4 transRect = glm::translate (glm::vec3(x,y,0));
    Matrices.model *= transRect*rotateRect;
    MVP = VP1 * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(rect1);

}

void draw_boxes(int flag)
{
  glm::mat4 rotateRect,transRect;
  glm::mat4 VP1,view1,p;
    p = glm::ortho(-120.0f, 120.0f, -100.0f, 100.0f, 0.1f, 120.0f);
    view1 = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    VP1 = p * view1;

  float x,y;
  if(flag==1)
  {
    x=-111;
    y=93;

    Matrices.model = glm::mat4(1.0f);
    rotateRect = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); 
    transRect = glm::translate (glm::vec3(x,y,0));
    Matrices.model *= transRect*rotateRect;
    MVP = VP1 * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(rect2);
    draw_rect(x-2,y,90);
    draw_rect(x-2+2*cos(30.0*M_PI/180),y+1,-30);
    draw_rect(x-2+2*cos(30.0*M_PI/180),y-2*cos(60.0*M_PI/180),30);

  }
  else if(flag==2)
  {
    x=1;
    y=-13;

    Matrices.model = glm::mat4(1.0f);
    rotateRect = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); 
    transRect = glm::translate (glm::vec3(x,y,0));
    Matrices.model *= transRect*rotateRect;
    MVP = VP1 * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(rect2);
    draw_rect(x-2,y,90);
    draw_rect(x-2+2*cos(30.0*M_PI/180),y+1,-30);
    draw_rect(x-2+2*cos(30.0*M_PI/180),y-2*cos(60.0*M_PI/180),30);

  }
}

void draw_score(int flag)
{
  int value,value2,shift=6,flag2=0,i;
  i=0;
  float x=115,y;
  if(flag==0||flag==2)
  {
  value=moves;
  y=70;
  if(value<0)
  {
    flag2=1;
    value=-1*value;
  }
  }
  else if(flag==1)
  {
    value=miss_limit-miss;
    y=50;
  }
  else if(flag==3)
  {
    value=stage.level;
    y=30;
  }

  if(flag==2)
  {
    x=25;
    y=0;
  }
  do
  {
    value2=value%10;
    value=value/10;
    if(value2==2||value2==3||value2==5||value2==6||value2==8||value2==9||value2==0)
    {
      draw_rect(x-shift*i,y,0);
    }
    if(value2==2||value2==6||value2==8||value2==0)
    {
      draw_rect(x-2-shift*i,y+2,90);
    }
    if(value2==4||value2==5||value2==6||value2==8||value2==9||value2==0)
    {
      draw_rect(x-2-shift*i,y+6,90);
    }
    if(value2==2||value2==3||value2==5||value2==6||value2==8||value2==9||value2==7||value2==0)
    {
      draw_rect(x-shift*i,y+8,0);
    }
    if(value2==1||value2==2||value2==3||value2==4||value2==7||value2==8||value2==9||value2==0)
    {
      draw_rect(x+2-shift*i,y+6,90);
    }
    if(value2==1||value2==3||value2==4||value2==5||value2==6||value2==7||value2==8||value2==9||value2==0)
    {
      draw_rect(x+2-shift*i,y+2,90);
    }
    if(value2==2||value2==3||value2==4||value2==5||value2==6||value2==8||value2==9)
    {
      draw_rect(x-shift*i,y+4,0);
    }
  i++;

  }while(value>0);

  if(flag2==1)
  {
      value=-1*value;
      draw_rect(x-shift*i,y+4,0);

  }
}

void draw_gameover()
{
  float x,y;
  x=22.5;
  y=20;

  draw_rect(x-2,y+2,90);
  draw_rect(x-2,y+6,90);
  draw_rect(x,y+4,0);
  draw_rect(x,y+8,0);
  draw_rect(x+2,y+6,90);
  draw_rect(x-0,y+2,-45);

  draw_rect(x-5,y,0);
  draw_rect(x-5,y+4,0);
  draw_rect(x-5,y+8,0);
  draw_rect(x-5-2,y+2,90);
  draw_rect(x-5-2,y+6,90);

  draw_rect(x-5*2-1+2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1-2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*2-1-6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);

  draw_rect(x-5*3-1,y,0);
  draw_rect(x-5*3-2-1,y+2,90);
  draw_rect(x-5*3-2-1,y+6,90);
  draw_rect(x-5*3-1,y+8,0);
  draw_rect(x-5*3+2-1,y+2,90);
  draw_rect(x-5*3+2-1,y+6,90); 

  draw_rect(x-5*5,y,0);
  draw_rect(x-5*5,y+4,0);
  draw_rect(x-5*5,y+8,0);
  draw_rect(x-5*5-2,y+2,90);
  draw_rect(x-5*5-2,y+6,90);

  draw_rect(x-5*6-2+1,y+2,90);
  draw_rect(x-5*6-2+1,y+6,90);
  draw_rect(x-5*6+2,y+2,90);
  draw_rect(x-5*6+2,y+6,90);
  draw_rect(x-5*6-1-6*sin(20.0*M_PI/180)+3,y+6*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*6-1+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);

  draw_rect(x-5*7-2*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*7-6*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*7+2*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*7+6*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*7,y+2,0);

  draw_rect(x-5*8-1,y,0);
  draw_rect(x-5*8,y+3,0);
  draw_rect(x-5*8-1,y+8,0);
  draw_rect(x-5*8-2-1,y+2,90);
  draw_rect(x-5*8-2-1,y+6,90);
  draw_rect(x-5*8+2-1,y+1,90);

}

void draw_level()
{
  float x,y;
  x=115;
  y=40;

  draw_rect(x+1,y,0);
  draw_rect(x-2+1,y+2,90);
  draw_rect(x-2+1,y+6,90);

  draw_rect(x-5+1,y,0);
  draw_rect(x-5+1,y+4,0);
  draw_rect(x-5+1,y+8,0);
  draw_rect(x-5-2+1,y+2,90);
  draw_rect(x-5-2+1,y+6,90);

  draw_rect(x-5*2+2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*2-6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);

  draw_rect(x-5*3,y,0);
  draw_rect(x-5*3,y+4,0);
  draw_rect(x-5*3,y+8,0);
  draw_rect(x-5*3-2,y+2,90);
  draw_rect(x-5*3-2,y+6,90);

  draw_rect(x-5*4,y,0);
  draw_rect(x-5*4-2,y+2,90);
  draw_rect(x-5*4-2,y+6,90);

}
void draw_scoretext(int flag_option)
{
  float x,y;
  if(flag_option==0)
  {
    x=115;
    y=85;
  }
  else if(flag_option==1)
  {
    x=0;
    y=0;
    /*draw score */
  }
  draw_rect(x,y,0);
  draw_rect(x+2,y+2,90);
  draw_rect(x,y+4,0);
  draw_rect(x-2,y+6,90);
  draw_rect(x,y+8,0);

  draw_rect(x-5,y,0);
  draw_rect(x-5,y+4,0);
  draw_rect(x-5,y+8,0);
  draw_rect(x-5-2,y+2,90);
  draw_rect(x-5-2,y+6,90);

  draw_rect(x-5*2-1+2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1-2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*2-1-6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);

  draw_rect(x-5*3,y,0);
  draw_rect(x-5*3-2,y+2,90);
  draw_rect(x-5*3-2,y+6,90);
  draw_rect(x-5*3,y+8,0);
  draw_rect(x-5*3+2,y+2,90);
  draw_rect(x-5*3+2,y+6,90);

  draw_rect(x-5*4-2+1,y+2,90);
  draw_rect(x-5*4-2+1,y+6,90);
  draw_rect(x-5*4+2,y+2,90);
  draw_rect(x-5*4+2,y+6,90);
  draw_rect(x-5*4-1-6*sin(20.0*M_PI/180)+3,y+6*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*4-1+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);


  if(flag_option==0)
  {
  y=60;
  draw_rect(x,y,0);
  draw_rect(x+2,y+2,90);
  draw_rect(x,y+4,0);
  draw_rect(x-2,y+6,90);
  draw_rect(x,y+8,0);

  draw_rect(x-5,y,0);
  draw_rect(x-5,y+4,0);
  draw_rect(x-5,y+8,0);
  draw_rect(x-5-2,y+2,90);
  draw_rect(x-5-2,y+6,90);

  draw_rect(x-5*2-1+2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1-2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*2-1-6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);

  draw_rect(x-5*3-2,y+2,90);
  draw_rect(x-5*3-2,y+6,90);

  draw_rect(x-5*4,y,0);
  draw_rect(x-5*4-2,y+2,90);
  draw_rect(x-5*4-2,y+6,90);

  }


}


// Creates the rectangle object used in this sample code


float camera_rotation_angle = 90;



/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (double x,double y)
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
//  glm::vec3 eye (-22, -43, 29 );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
//  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
//  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
//  Matrices.view = glm::lookAt(glm::vec3(-30,-35,35), glm::vec3(-20,-20,10), glm::vec3(1,1,3)); // Fixed camera for 2D (ortho) in XY plane
//  Matrices.view = glm::lookAt(glm::vec3(-29,-43,25), glm::vec3(-23,-32,9), glm::vec3(1,1,3)); // Fixed camera for 2D (ortho) in XY plane
  Matrices.view = glm::lookAt(glm::vec3(-22,-43,29), glm::vec3(-10,0,0), glm::vec3(0,0,1)); // Fixed camera for 2D (ortho) in XY plane
//  Matrices.view = glm::lookAt(glm::vec3(-10,0,20), glm::vec3(-10,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane      
    if(v==0)  //preview view
    {
      Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
      Matrices.view = glm::lookAt(glm::vec3(-22,-43,29), glm::vec3(-10,0,0), glm::vec3(0,0,1));
    } 
    else if(v==1)      // top view
    {
      Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
      Matrices.view = glm::lookAt(glm::vec3(-10,0,29), glm::vec3(-10,0,0), glm::vec3(0,1,0)); 

    }
    else if(v==2)   // tower view
    {
      Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
      Matrices.view = glm::lookAt(glm::vec3(-22,-43,15), glm::vec3(-10,0,0), glm::vec3(0,0,1));       
    }
    else if(v==3)   //block view
    {
       Matrices.projection = glm::perspective ((GLfloat)90.0f, (GLfloat) 800 / (GLfloat) 700, 0.1f, 500.0f);
        if(flag_stand==1)
        {
      Matrices.view = glm::lookAt(glm::vec3(15+(block.cube1i-8)*10,5+(block.cube1j-5)*10,11), glm::vec3(60,0,0), glm::vec3(0,0,1)); // top view               
        }
      else if(flag_attach==1)
      {
        if(block.cube1i<block.cube2i)
          Matrices.view = glm::lookAt(glm::vec3(15+(block.cube2i-8)*10,5+(block.cube2j-5)*10,7), glm::vec3(60,0,0), glm::vec3(0,0,1)); // top view                                      
        else if(block.cube1i>block.cube2i)
          Matrices.view = glm::lookAt(glm::vec3(15+(block.cube1i-8)*10,5+(block.cube1j-5)*10,7), glm::vec3(60,0,0), glm::vec3(0,0,1)); // top view                                      
      else
          Matrices.view = glm::lookAt(glm::vec3(15+(block.cube1i-8)*10,(5+((block.cube1j+block.cube2j)/2.0-5)*10),7), glm::vec3(60,0,0), glm::vec3(0,0,1)); // top view                                     

      }
     }
    else if(v==4)   //follow-block view
    {
       Matrices.projection = glm::perspective ((GLfloat)90.0f, (GLfloat) 800 / (GLfloat) 700, 0.1f, 500.0f);
        if(flag_stand==1)
        {
      Matrices.view = glm::lookAt(glm::vec3((block.cube1i-8)*10-10,-5+(block.cube1j-5)*10,20), glm::vec3(60,0,8), glm::vec3(0,0,1)); // top view                
        }
      else if(flag_attach==1)
      {
        if(block.cube1i<block.cube2i)
          Matrices.view = glm::lookAt(glm::vec3((block.cube2i-8)*10-10,5+(block.cube2j-5)*10,20), glm::vec3(60,0,8), glm::vec3(0,0,1)); // top view                                     
        else if(block.cube1i>block.cube2i)
          Matrices.view = glm::lookAt(glm::vec3((block.cube1i-8)*10-10,5+(block.cube1j-5)*10,20), glm::vec3(60,0,8), glm::vec3(0,0,1)); // top view                                     
      else
          Matrices.view = glm::lookAt(glm::vec3((block.cube1i-8)*10-10,(5+((block.cube1j+block.cube2j)/2.0-5)*10),20), glm::vec3(60,0,8), glm::vec3(0,0,1)); // top view                                      

      }
  }
  else if(v==5)  //helicopter view
  {
    if(flag_hover==1)
    {
      x_g=x;
      y_g=y;
    glm::vec3 eye (x_g,y_g, 30 );
      Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
      Matrices.view = glm::lookAt(eye, glm::vec3(-10,0,0), glm::vec3(0,0,1)); // helicopter view    

    }
    else
    {
    glm::vec3 eye (x_g,y_g, 30 );
      Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
      Matrices.view = glm::lookAt(eye, glm::vec3(-10,0,0), glm::vec3(0,0,1)); // helicopter view    
    }
  }


  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */
   if(flag_gameover ==1)
  {
      if(flag_gamestart==1)
      {
        flag_gameover =0;
        moves=0;
        miss=0;
        stage.level=1;
        timehr=0;
        timesec=0;
        timemin=0;
        v=0;
        zoom=26;
        flag_complete=0;
        flag_fallcomp=0;
        flag_fall=0;
        stage.end_stage=0;
        stage.start_stage=1;
        stage.zs=-50;
      Matrices.projection = glm::ortho(-120.0f+zoom, 120.0f-zoom, -100.0f+zoom, 100.0f-zoom, 0.1f, 120.0f);
        block.initiateVariables(stage.level);
    last_update_time = glfwGetTime();
    update_call = glfwGetTime();
    change_time = glfwGetTime();
        
        flag_gamestart=0;
        return;
      }
      else
      {
      draw_boxes(2);
      draw_scoretext(1);
      draw_score(2);
      draw_gameover();
      }
      return;
  }

  draw_boxes(1);
  draw_scoretext(0);
  draw_level();
  draw_score(3);
  draw_score(0);
  draw_score(1);

  stage.animateStage();
  block.animateCube();
  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();

  // Increment angles

  //camera_rotation_angle++; // Simulating camera rotation
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Bloxorz", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
  // Create the models
  block.createSquare();
  stage.createStage2();
  stage.createStage1();
  stage.createCircle();
  stage.createRectangle();
  createRectangle();
  createRectangle2();
  // Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

  
  reshapeWindow (window, width, height);

    // Background color of the scene
  glClearColor (255.0/255, 250.0/255.0, 250.0/255.0, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
  int width = 800;
  int height = 700;

  double x1,y1;

    GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        glfwGetCursorPos(window, &xpos, &ypos);
        x1=(xpos-400)*1.0*3/10;
        y1=(350-ypos)*1.0/3.5;

        // OpenGL Draw commands
        draw(x1,y1);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.3) { // atleast 0.5s elapsed since last frame
            last_update_time = current_time;
            flag_move=1;
        }
        if ((current_time - update_call) >= 0.04) { // atleast 0.5s elapsed since last frame
            update_call = current_time;
            fall_call =1;
        }
        if ((current_time - change_time) >= 1) { // atleast 0.5s elapsed since last frame
            change_time = current_time;
            timesec++;
            if(timesec==60)
            {
              timemin++;
              timesec=0;
              if(timemin==60)
              {
                timehr++;
                timemin=0;
              }
            }
        }

    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
  