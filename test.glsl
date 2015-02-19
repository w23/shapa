uniform float uf_time;
uniform vec2 uv2_resolution;
float t = uf_time;

void main() {
  vec2 uv = gl_FragCoord.xy / uv2_resolution * 2. - 1.;
  uv.x *= uv2_resolution.x / uv2_resolution.y;
  gl_FragColor = vec4(uv, sin(t), 0.);
}
