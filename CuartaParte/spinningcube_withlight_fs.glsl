#version 450

struct Material {
  sampler2D diffuse;
  sampler2D specular;
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

uniform Light light2;

void main() {
  // Ambient
  //vec3 ambient = light.ambient * material.ambient;
  //vec3 ambient2 = light2.ambient * material.ambient;

  //Ambient Terceira Parte
  vec3 ambient = light.ambient * vec3(texture(material.diffuse, vs_tex_coord));
  vec3 ambient2 = light2.ambient * vec3(texture(material.diffuse, vs_tex_coord));


  // Diffuse
  vec3 light_dir = normalize(light.position - frag_3Dpos);
  float diff = max(dot(vs_normal, light_dir), 0.0);
  //vec3 diffuse = light.diffuse * diff * material.diffuse;

  //Terceira parte
  vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, vs_tex_coord));


  vec3 light_dir2 = normalize(light2.position - frag_3Dpos);
  float diff2 = max(dot(vs_normal, light_dir2), 0.0);
  vec3 diffuse2 = light2.diffuse * diff2 * vec3(texture(material.diffuse, vs_tex_coord));

  // Specular
  vec3 view_dir = normalize(view_pos - frag_3Dpos);
  vec3 reflect_dir = reflect(-light_dir, vs_normal);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  //vec3 specular = light.specular * spec * material.specular;

  vec3 view_dir2 = normalize(view_pos - frag_3Dpos);
  vec3 reflect_dir2 = reflect(-light_dir2, vs_normal);
  float spec2 = pow(max(dot(view_dir2, reflect_dir2), 0.0), material.shininess);
  //vec3 specular2 = light2.specular * spec2 * material.specular;


  //Specular Cuarta Parte
  vec3 specular = light.specular * spec * vec3(texture(material.specular, vs_tex_coord));
  vec3 specular2 = light2.specular * spec2 * vec3(texture(material.specular, vs_tex_coord));


  vec3 result = ambient + ambient2 + diffuse + diffuse2 + specular + specular2;
  frag_col = vec4(result, 1.0);
}