#version 130
in vec4 vColor;
out vec4 fColor;

void main(void)
{
  fColor = vColor;
  fColor.a = 0.4;
}

