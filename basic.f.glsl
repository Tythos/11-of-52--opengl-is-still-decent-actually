/**
 * basic.f.glsl
 */
 
varying vec2 vUV;
out vec4 oRGBA;
uniform sampler2D uTexture;

void main() {
    oRGBA = texture(uTexture, vUV);
}
