#version 330

// GLSL version 3.3
// vertex shader

layout(location = 0) in vec2 position; // input:  (x, y coords) per vertex

uniform mat4 worldToView;              // parameter: world to view transformation matrix

void main() {
  gl_Position = worldToView * vec4(position.x, position.y, -0.02, 1.0);
}

