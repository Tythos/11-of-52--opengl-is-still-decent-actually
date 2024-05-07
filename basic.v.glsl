/**
 * basic.v.glsl
 */
 
in vec2 aXY;
in vec2 aUV;
varying vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aXY, 0.0, 1.0);
}
