uniform float uf_time;
float t = uf_time;

void main() {
  gl_FragColor = vec4(sin(t));
}
