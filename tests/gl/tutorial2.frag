#version 150
// It was expressed that some drivers required this next line to function properly
precision highp float;

in  vec3 ex_Color;
out vec4 gl_FragColor;

void main(void) {
    // Pass through our original color with full opacity.
    gl_FragColor = vec4(ex_Color,1.0);
}
