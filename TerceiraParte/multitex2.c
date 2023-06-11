// Copyright (C) 2020 Emilio J. Padrón
// Released as Free Software under the X11 License
// https://spdx.org/licenses/X11.html
//
// Strongly inspired by textures.cpp in learnopengl.com
// https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.1.textures/textures.cpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int gl_width = 640;
int gl_height = 480;

void glfw_window_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void render(void);

GLuint shader_program = 0; // shader program to set render pipeline
GLuint vao = 0; // Vertext Array Object to set input data
GLuint texture[2]; // Our two textures

int main() {
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(gl_width, gl_height, "Hello Texture on Quad", NULL, NULL);
  if (!window) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }
  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwMakeContextCurrent(window);

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

  // Vertex Shader
  const char* vertex_shader =
    "#version 130\n"
    "in vec3 v_pos;"
    "in vec2 tex_coord;"
    "out vec2 vs_tex_coord;"
    "void main() {"
    "  gl_Position = vec4(v_pos, 1.0);"
    "  vs_tex_coord = tex_coord;"
    "}";

  // Fragment Shader
  const char* fragment_shader =
    "#version 130\n"
    "out vec4 frag_col;"
    "in vec2 vs_tex_coord;"
    "uniform sampler2D texture1;"
    "uniform sampler2D texture2;"
    "void main() {"
    "  vec4 tex1 = texture(texture1, vs_tex_coord);"
    "  vec4 tex2 = texture(texture2, vs_tex_coord);"
    "  gl_FragColor = tex2 + (1 - tex2.a) * tex1;"
    "}";

  // Shaders compilation
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);

  // Create program, attach shaders to it and link it
  shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  // Release shader objects
  glDeleteShader(vs);
  glDeleteShader(fs);

  // Quad to be rendered (NDC): (x, y, z) (s, t)
  // => Two triangles, sharing 2 vertices
  float points[] = {
   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  // lower-left corner
    0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // lower-right corner
    0.5f,  0.5f, 0.0f, 1.0f, 1.0f,  // top-right corner
   -0.5f,  0.5f, 0.0f, 0.0f, 1.0f   // top-left corner
  };

  unsigned int indices[] = {
    0, 1, 2, // right triangle
    0, 2, 3  // left triangle
  };

  // VAO, VBO, VBE
  GLuint vbo, ebo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  // VBO: 3D vertices with s,t texcoord
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
  // EBO (triangle indices)
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  // 0: vertex position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);
  // 1: vertex texCoord attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Unbind vbo (it was conveniently registered by VertexAttribPointer)
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Unbind vao
  glBindVertexArray(0);

  // Create texture objects
  glGenTextures(2, texture);

  // First texture in Texture Unit #0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);

  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(1);
  unsigned char *data = stbi_load("texture.jpg", &width, &height, &nrChannels, 0);
  // Image from http://www.flickr.com/photos/seier/4364156221
  // CC-BY-SA 2.0

  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("Failed to load first texture\n");
  }
  stbi_image_free(data);

  // Second texture in Texture Unit #1
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture[1]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glUniform1i(glGetUniformLocation(shader_program, "texture2"), 1);

  data = stbi_load("watchmen_smiley_trans.png", &width, &height, &nrChannels, 0);
  // Image from https://es.wikipedia.org/wiki/Archivo:Watchmen_Smiley.svg
  // CC-BY-SA 3.0
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("Failed to load second texture\n");
  }
  stbi_image_free(data);

  // Render loop
  while(!glfwWindowShouldClose(window)) {

    processInput(window);

    render();

    // put the stuff we've been drawing onto the display
    glfwSwapBuffers(window);

    // update other events like input handling
    glfwPollEvents();
  }

  // close GL context and any other GLFW resources
  glfwTerminate();

  return 0;
}

void render(void) {
  // wipe the drawing surface clear
  glClear(GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, gl_width, gl_height);

  glUseProgram(shader_program);
  glBindVertexArray(vao);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture[1]);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);

  // Switch fill/wireframe view mode with key v
  static GLenum view_mode = GL_FILL;
  static int allowchange = 5;

  if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {

    if (allowchange > 0) {
      allowchange--;
    } else {
      allowchange = 5;

      if (view_mode == GL_FILL)
        view_mode = GL_LINE;
      else if (view_mode == GL_LINE)
        view_mode = GL_POINT;
      else
        view_mode = GL_FILL;

      glPolygonMode(GL_FRONT_AND_BACK, view_mode);
    }
  }
}

// Callback function to track window size and update viewport
void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
  gl_width = width;
  gl_height = height;
  printf("New viewport: (width: %d, height: %d)\n", width, height);
}
