// Copyright (C) 2020 Emilio J. Padrón
// Released as Free Software under the X11 License
// https://spdx.org/licenses/X11.html

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

// GLM library to deal with matrix operations
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include "textfile_ALT.h"

int gl_width = 640;
int gl_height = 480;

void glfw_window_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void render(double);

GLuint shader_program = 0; // shader program to set render pipeline
GLuint vao = 0; // Vertext Array Object to set input data
GLint model_location, view_location, proj_location; // Uniforms for transformation matrices
GLint normal_location, camera_location;
GLint light_pos_location, light_amb_location, light_diff_location, light_spec_location;
GLint light_pos_location2, light_amb_location2, light_diff_location2, light_spec_location2;
GLint material_amb_location, material_diff_location, material_spec_location, material_shin_location;


// Shader names
const char *vertexFileName = "spinningcube_withlight_vs.glsl";
const char *fragmentFileName = "spinningcube_withlight_fs.glsl";

// Camera
glm::vec3 camera_pos(0.0f, 0.0f, 3.0f);

// Lighting
glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
glm::vec3 light_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 light_diffuse(0.5f, 0.5f, 0.5f);
glm::vec3 light_specular(1.0f, 1.0f, 1.0f);

glm::vec3 light_pos2(0.3f, 1.0f, 1.0f);
glm::vec3 light_ambient2(0.2f, 0.2f, 0.2f);
glm::vec3 light_diffuse2(0.5f, 0.5f, 0.5f);
glm::vec3 light_specular2(0.5f, 0.5f, 0.5f);

// Material
glm::vec3 material_ambient(0.5f, 0.5f, 0.31f);
glm::vec3 material_diffuse(0.5f, 0.5f, 0.31f);
glm::vec3 material_specular(0.5f, 0.5f, 0.5f);
const GLfloat material_shininess = 32.0f;

int main() {
  /*
   * Inicialización e configuración de GLFW
   */
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  /*
   * Para cambiar a versión
   */
  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  /*
   * Creación da xanela e o contexto OpenGL
   */
  GLFWwindow* window = glfwCreateWindow(gl_width, gl_height, "My spinning cube", NULL, NULL);
  if (!window) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }
  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwMakeContextCurrent(window);

  /*
   * Chamadas a API
   */
  // start GLEW extension handler
  // glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte* vendor = glGetString(GL_VENDOR); // get vendor string
  const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte* glversion = glGetString(GL_VERSION); // version as a string
  const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION); // version as a string
  printf("Vendor: %s\n", vendor);
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", glversion);
  printf("GLSL version supported %s\n", glslversion);
  printf("Starting viewport: (width: %d, height: %d)\n", gl_width, gl_height);

  // Enable Depth test: only draw onto a pixel if fragment closer to viewer
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); // set a smaller value as "closer"

  // Vertex Shader
  const char* vertex_shader = textFileRead(vertexFileName);

  // Fragment Shader
  const char* fragment_shader = textFileRead(fragmentFileName);

  // Shaders compilation
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  free((void*) vertex_shader);
  glCompileShader(vs);

  int  success;
  char infoLog[512];
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vs, 512, NULL, infoLog);
    printf("ERROR: Vertex Shader compilation failed!\n%s\n", infoLog);

    return(1);
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  free((void*) fragment_shader);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fs, 512, NULL, infoLog);
    printf("ERROR: Fragment Shader compilation failed!\n%s\n", infoLog);

    return(1);
  }

  // Create program, attach shaders to it and link it
  shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  glValidateProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
    printf("ERROR: Shader Program linking failed!\n%s\n", infoLog);

    return(1);
  }

  // Release shader objects
  glDeleteShader(vs);
  glDeleteShader(fs);

  // Vertex Array Object
  /*
   * Crear e activar antes dos VBOs e Vertex attributes a agrupar neste VAO
   */
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Cube to be rendered
  //
  //          0        3
  //       7        4 <-- top-right-near
  // bottom
  // left
  // far ---> 1        2
  //       6        5
  //
  const GLfloat vertex_positions[] = {
    -0.25f, -0.25f, -0.25f, // 1
    -0.25f,  0.25f, -0.25f, // 0
     0.25f, -0.25f, -0.25f, // 2

     0.25f,  0.25f, -0.25f, // 3
     0.25f, -0.25f, -0.25f, // 2
    -0.25f,  0.25f, -0.25f, // 0

    0.25f, -0.25f, -0.25f, // 2
    0.25f,  0.25f, -0.25f, // 3
    0.25f, -0.25f,  0.25f, // 5

    0.25f,  0.25f,  0.25f, // 4
    0.25f, -0.25f,  0.25f, // 5
    0.25f,  0.25f, -0.25f, // 3

     0.25f, -0.25f,  0.25f, // 5
     0.25f,  0.25f,  0.25f, // 4
    -0.25f, -0.25f,  0.25f, // 6

    -0.25f,  0.25f,  0.25f, // 7
    -0.25f, -0.25f,  0.25f, // 6
     0.25f,  0.25f,  0.25f, // 4

    -0.25f, -0.25f,  0.25f, // 6
    -0.25f,  0.25f,  0.25f, // 7
    -0.25f, -0.25f, -0.25f, // 1

    -0.25f,  0.25f, -0.25f, // 0
    -0.25f, -0.25f, -0.25f, // 1
    -0.25f,  0.25f,  0.25f, // 7

     0.25f, -0.25f, -0.25f, // 2
     0.25f, -0.25f,  0.25f, // 5
    -0.25f, -0.25f, -0.25f, // 1

    -0.25f, -0.25f,  0.25f, // 6
    -0.25f, -0.25f, -0.25f, // 1
     0.25f, -0.25f,  0.25f, // 5

     0.25f,  0.25f,  0.25f, // 4
     0.25f,  0.25f, -0.25f, // 3
    -0.25f,  0.25f,  0.25f, // 7

    -0.25f,  0.25f, -0.25f, // 0
    -0.25f,  0.25f,  0.25f, // 7
     0.25f,  0.25f, -0.25f,  // 3

     
    //              A  <---- top
    //
    //
    // far --->     1
    //          2       3
    //
    // // //Base
    0.75f, -0.25f, -0.25f,  // 1
    0.5f,  -0.25f,  0.25f,  // 2
    1.0f,  -0.25f,  0.25f,   // 3

    // //Cara 1
    0.75f, -0.25f, -0.25f,  // 1
    0.5f,  -0.25f,  0.25f,  // 2
    0.75f,  0.25f,  0.0f,  // A

    // //Cara 2
    0.75f, -0.25f, -0.25f,  // 1
    1.0f,  -0.25f,  0.25f,  // 3
    0.75f,  0.25f,  0.0f,  // A

    // //Cara 3
    0.5f,  -0.25f,  0.25f,  // 2
    1.0f,  -0.25f,  0.25f,  // 3
    0.75f,  0.25f,  0.0f  // A

  };


// Vertex Buffer Object (for vertex coordinates)
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);

  /*
   * Conectar VBO e entradas do vertex shader
   */
  // Vertex attributes
  // 0: vertex position (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0); // Activar atributo, mapeando con IN var en VS 

  // 1: vertex normals (x, y, z)
  GLfloat vertex_normals[sizeof(float)* (36+12)*3];     //36 vertices + 12 vertices * 3 vertices/triangulo

  for(int i = 0; i < (36+12)*3; i+= 9) {

    //Tres vértices dun triángulo (A, B, C)
    GLfloat A[] = {vertex_positions[i], vertex_positions[i+1], vertex_positions[i+2]};
    GLfloat B[] = {vertex_positions[i+3], vertex_positions[i+4], vertex_positions[i+5]};
    GLfloat C[] = {vertex_positions[i+6], vertex_positions[i+7], vertex_positions[i+8]};

    //Vector do vértice A ao B e do A ao C
    GLfloat AB[] = {B[0] - A[0], B[1] - A[1], B[2] - A[2]};     // B.x - A.x, B.y - A.y, B.z - A.z
    GLfloat AC[] = {C[0] - A[0], C[1] - A[1], C[2] - A[2]};     // C.x - A.x, C.y - A.y, C.z - A.z

    //Producto cruz de AB e AC
    GLfloat x = AB[1] * AC[2] - AB[2] * AC[1];     // AB.y * AC.z - AB.z * AC.y
    GLfloat y = AB[2] * AC[0] - AB[0] * AC[2];     // AB.z * AC.x - AB.x * AC.z
    GLfloat z = AB[0] * AC[1] - AB[1] * AC[0];     // AB.x * AC.y - AB.y * AC.x


    vertex_normals[i] = x;
    vertex_normals[i+1] = y;
    vertex_normals[i+2] = z;

    vertex_normals[i+3] = x;
    vertex_normals[i+4] = y;
    vertex_normals[i+5] = z;

    vertex_normals[i+6] = x;
    vertex_normals[i+7] = y;
    vertex_normals[i+8] = z;

  }

  GLuint normals_buffer = 0;
  glGenBuffers(1, &normals_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normals_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_normals), vertex_normals, GL_STATIC_DRAW);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1); 

  // Unbind vbo (it was conveniently registered by VertexAttribPointer)
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 1);

  // Unbind vao
  glBindVertexArray(0);
  glBindVertexArray(1);

  /* Localizadores das variables do shader (model, projection, etc) que se usan despois
    para darlle un valor a ditas variables */
  // Uniforms
  // - Model matrix
  // - View matrix
  // - Projection matrix
  // - Normal matrix: normal vectors from local to world coordinates
  // - Camera position
  // - Light data
  // - Material data

  model_location = glGetUniformLocation(shader_program, "model");
  view_location = glGetUniformLocation(shader_program, "view");
  proj_location = glGetUniformLocation(shader_program, "projection");
  normal_location = glGetUniformLocation(shader_program, "normal_to_world");
  camera_location = glGetUniformLocation(shader_program, "view_pos");

  light_pos_location = glGetUniformLocation(shader_program, "light.position");
  light_amb_location = glGetUniformLocation(shader_program, "light.ambient");
  light_diff_location = glGetUniformLocation(shader_program, "light.diffuse");
  light_spec_location = glGetUniformLocation(shader_program, "light.specular");
  
  light_pos_location2 = glGetUniformLocation(shader_program, "light2.position");
  light_amb_location2 = glGetUniformLocation(shader_program, "light2.ambient");
  light_diff_location2 = glGetUniformLocation(shader_program, "light2.diffuse");
  light_spec_location2 = glGetUniformLocation(shader_program, "light2.specular");

  material_amb_location = glGetUniformLocation(shader_program, "material.ambient");
  material_diff_location = glGetUniformLocation(shader_program, "material.diffuse");
  material_spec_location = glGetUniformLocation(shader_program, "material.specular");
  material_shin_location = glGetUniformLocation(shader_program, "material.shininess");
  

  /*
   * Bucle de render para manter a ventana aberta
   */
// Render loop
  while(!glfwWindowShouldClose(window)) {

    processInput(window);       //Xestionamos entrada de usuario

    render(glfwGetTime());      //Comandos de render para debuxar nun buffer

    glfwSwapBuffers(window);    //Intercambia back e front buffer no framebuffer activo

    glfwPollEvents();           //Procesamos eventos varios
  }

  glfwTerminate();

  return 0;
}

void render(double currentTime) {
  float f = (float)currentTime * 0.3f;

  //Borrado buffers de cor e profundidade
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Tamaño do espacio de render
  glViewport(0, 0, gl_width, gl_height);

  glUseProgram(shader_program);
  glBindVertexArray(vao);

  glm::mat4 model_matrix, view_matrix, proj_matrix;
  glm::mat3 normal_matrix;

  model_matrix = glm::mat4(1.f);
  view_matrix = glm::lookAt(                 camera_pos,  // pos
                            glm::vec3(0.0f, 0.0f, 0.0f),  // target
                            glm::vec3(0.0f, 0.5f, 0.0f)); // up

  //Establecemos o valor para a uniform "view" no shader usando o seu localizador
  glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));

  // Moving cube
  // model_matrix = glm::rotate(model_matrix,

  // Matriz de rotación que xira en torno ao eixe "y" (0.0f, 0.5f, 0.0f) 
  // O ángulo de rotación calculase en función do tempo actual
  model_matrix = glm::rotate(model_matrix,
                          glm::radians((float)currentTime * 45.0f),
                          glm::vec3(0.0f, 0.5f, 0.0f));
                          
  // Matriz de rotación que xira en torno ao eixe "x" (0.5f, 0.0f, 0.0f)  
  // O ángulo de rotación calculase en función do tempo actual
  model_matrix = glm::rotate(model_matrix,
                         glm::radians((float)currentTime * 80.5f),
                         glm::vec3(0.5f, 0.0f, 0.0f));

  //Establecemos o valor para a uniform "model" no shader usando o seu localizador
  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));


  // Projection
  // proj_matrix = glm::perspective(glm::radians(50.0f),
  proj_matrix = glm::perspective(glm::radians(50.0f),
                                 (float) gl_width / (float) gl_height,
                                 0.1f, 1000.0f);

  //Establecemos o valor para a uniform "projection" no shader usando o seu localizador
  glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));



  // Normal matrix: normal vectors to world coordinates
  //normal_matrix = glm::inverseTranspose(glm::mat3(model_matrix));
  normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));

  //Establecemos o valor para a uniform "normal_to_world" no shader usando o seu localizador
  glUniformMatrix3fv(normal_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));



  //Establecemos o valor para a uniform "camera_pos" no shader usando o seu localizador
  glUniform3fv(camera_location, 1, glm::value_ptr(camera_pos));

  //Establecemos o valor para cada atributo da uniform "light" no shader usando o localizador correspondente
  glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
  glUniform3fv(light_amb_location, 1, glm::value_ptr(light_ambient));
  glUniform3fv(light_diff_location, 1, glm::value_ptr(light_diffuse));
  glUniform3fv(light_spec_location, 1, glm::value_ptr(light_specular));


  //Establecemos o valor para cada atributo da uniform "light2" no shader usando o localizador correspondente
  glUniform3fv(light_pos_location2, 1, glm::value_ptr(light_pos2));
  glUniform3fv(light_amb_location2, 1, glm::value_ptr(light_ambient2));
  glUniform3fv(light_diff_location2, 1, glm::value_ptr(light_diffuse2));
  glUniform3fv(light_spec_location2, 1, glm::value_ptr(light_specular2));


  //Establecemos o valor para cada atributo da uniform "material" no shader usando o localizador correspondente
  glUniform3fv(material_amb_location, 1, glm::value_ptr(material_ambient));
  glUniform3fv(material_diff_location, 1, glm::value_ptr(material_diffuse));
  glUniform3fv(material_spec_location, 1, glm::value_ptr(material_specular));
  glUniform1f(material_shin_location, material_shininess);  


  glDrawArrays(GL_TRIANGLES, 0, 36+12);  //36 vertices del cubo y 12 de la piramide

}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

// Callback function to track window size and update viewport
void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
  gl_width = width;
  gl_height = height;
  printf("New viewport: (width: %d, height: %d)\n", width, height);
}