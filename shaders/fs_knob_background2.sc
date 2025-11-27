$input v_coordinates, v_dimensions, v_shader_values, v_position, v_gradient_pos, v_gradient_pos2, v_gradient_texture_pos

#include <shader_include.sh>

uniform vec4 u_time;
//uniform vec4 u_color_mult;

SAMPLER2D(s_gradient, 0);

float fcos(float x) {
  return cos(x) * smoothstep(6.28, 0.0, fwidth(x));
}

float getColor(in float t) {
  float col = 0.3;
  col += 0.12 * fcos(6.28318 * t *   1.0);
  col += 0.11 * fcos(6.28318 * t *   3.1);
  col += 0.10 * fcos(6.28318 * t *   5.1);
  col += 0.09 * fcos(6.28318 * t *   9.1);
  col += 0.08 * fcos(6.28318 * t *  17.1);
  col += 0.07 * fcos(6.28318 * t *  31.1);
  col += 0.06 * fcos(6.28318 * t *  65.1);
  col += 0.06 * fcos(6.28318 * t * 115.1);
  col += 0.09 * fcos(6.28318 * t * 265.1);
  return col;
}

vec2 deform(float time, in vec2 p) {
  p *= 4.0;
  p = 0.5 * p / dot(p, p);
  p.x += time * 0.05;

  p += 0.2 * cos(1.5 * p.yx + 0.03 * 1.0 * time + vec2(0.1, 1.1));
  p += 0.2 * cos(2.4 * p.yx + 0.03 * 1.6 * time + vec2(4.5, 2.6));
  p += 0.2 * cos(3.3 * p.yx + 0.03 * 1.2 * time + vec2(3.2, 3.4));
  p += 0.2 * cos(4.2 * p.yx + 0.03 * 1.7 * time + vec2(1.8, 5.2));
  p += 0.2 * cos(9.1 * p.yx + 0.03 * 1.1 * time + vec2(6.3, 3.9));

  return p;
}

float smoothy(float d, float s, float pos){
  return (tanh(s * (d - pos)) + 1.0) * 0.5;
}

float smoothring(float d, float s, float pos, float width){
  float a = smoothy(d, s, pos + width);
  float b = smoothy(1.0 - d, s, 1.0 - pos + width);
  return clamp(1.0 - (a + b), 0.0, 1.0);
}

float smoothgrad(float d, float pos){
  float t = (1.0 / (pos * 0.5)) * (d - 0.5 * pos);
  if(d > pos / 2){
    return t * t * t * t * t * t * t;
  }else{
    return 0.0;
  }
  
}

void main() {
  const float body_radius = 0.8;
  const float ring_width = 0.025;
  const float ring_radius = 0.8 + ring_width;
  vec2 gradient_pos = gradient(s_gradient, v_gradient_texture_pos, v_gradient_pos, v_gradient_pos2, v_position);
  vec4 input_values = gradient(s_gradient, v_gradient_texture_pos, v_gradient_pos, v_gradient_pos2, v_position);
  // float knob_value = 0.5;
  float knob_value = input_values.z;

  float d = length(v_coordinates);
  float body_mask = 1.0 - smoothy(d, 50.0, body_radius);
  float grad = (1.0 - clamp(smoothgrad(d, body_radius), 0.0, 1.0)) * 0.2 + 0.07;
  float body_grad = grad * body_mask;
  
  vec4 body = vec4(body_grad, body_grad, body_grad, body_mask);

  float ring_mask = smoothring(d, 200.0, ring_radius, ring_width * (1.0 + input_values.g * 0.9));
  float ypos = (v_coordinates.y + ring_radius + ring_width * 3.0) / (2.0 * (ring_radius + ring_width * 3.0));
  ring_mask = ring_mask * smoothy(ypos, 50.0, 1.0 - knob_value);
  vec4 ring = vec4(0.13, 0.67, 1.0, ring_mask);
  vec4 outside_decay = vec4(0.0, 0.0, 0.0, 1.0);
  float outside_decay_mask = smoothring(d, 200.0, ring_radius + 0.08, 0.08);
  outside_decay_mask = clamp(outside_decay_mask * (1.0 - smoothy(d, 20.0, 0.85)), 0.0, 1.0);

  gl_FragColor = body * body_mask + ring * ring_mask + outside_decay * outside_decay_mask * 0.5;
}

