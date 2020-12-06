#version 330 core

// fragment shader

uniform vec4 fixedColor;         // parameter: grid color as rgba-value

out vec4 finalColor;             // output:    final color value as rgba-value

void main() {
  finalColor = fixedColor;
}
