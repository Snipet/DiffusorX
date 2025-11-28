$input v_coordinates, v_dimensions, v_shader_values, v_position, v_gradient_pos, v_gradient_pos2, v_gradient_texture_pos

#include <shader_include.sh>

uniform vec4 u_time;
//uniform vec4 u_color_mult;

SAMPLER2D(s_gradient, 0);

float bevelGrad(float x) {
  const float g1 = 0.95;
  const float g2 = 0.2;
  float outv = 0.0;
  if(x > g1 - g2){
    float s = (1.0 / (1.0 - g1)) * (1.0 / (1.0 - g1)) * (1.0 / (1.0 - g1));
    float v = (x - g1 + g2) * (x - g1 + g2) * (x - g1 + g2);
    outv = s * v;
  }
  return -outv + 1.0;
}

void main() {
  const float body_radius = 0.8;

  // Circle
  vec2 pos = v_coordinates - vec2(0.0, 0.25);
  float dist = length(pos * vec2(v_dimensions.x / v_dimensions.y, 1.0));
  float body_mask = bevelGrad(dist);
  vec4 outColor = vec4(0.95, 0.95, 0.95, 1.0);
  outColor.a *= body_mask;

  gl_FragColor = outColor;
}

