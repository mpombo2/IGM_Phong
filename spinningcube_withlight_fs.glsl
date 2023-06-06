#version 450

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

out vec4 frag_col;
in vec4 vs_color;

in vec3 frag_3Dpos;
in vec3 vs_normal;
in vec2 vs_tex_coord;

uniform Material material;
uniform Light light;
uniform vec3 view_pos;

void main() {
  // Ambient

  // Diffuse

  // Specular

  //vec3 result = ambient + diffuse + specular;
  frag_col = vec4(0.2, 0.5, 0.2, 0.2);
}
