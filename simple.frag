#version 130
in vec4 vColor;
out vec4 fColor;

void main(void)
{
  /*
  if (gl_FragCoord.x > 500) {
    fColor = vec4(1.0, 0.5, 0.5, 1.0);
  } else {
    fColor = vColor;
  }*/
  fColor = vColor;
}
