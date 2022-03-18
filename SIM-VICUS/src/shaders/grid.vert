#version 330

// GLSL version 3.3
// vertex shader

layout(location = 0) in vec2 position; // input:  (x, y) per vertex

uniform mat4 worldToView;              // parameter: world to view transformation matrix
uniform mat4 modelToWorld;             // parameter: model to world transformation matrix

void main() {
  /* Shift z-value a little down, so that grid is underneath geometry at same z-level. */
  gl_Position = worldToView * modelToWorld * vec4(position.x, position.y, - 0.02, 1.0);
}

