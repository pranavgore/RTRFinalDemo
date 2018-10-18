#version 450 core

in vec2 out_texcoord;
out vec4 FragColor;

uniform sampler2D Texture;
uniform int Width;
uniform float odw;

void main(void)
{
	vec3 Color = vec3(0.0);
	int wp1 = Width + 1;
	float Sum = 0.0;

	for (int x = -Width; x <= Width; x++)
	{
		float width = (wp1 - abs(float(x)));
		Color += texture2D(Texture, out_texcoord + vec2(0.0, odw * x)).rgb * width;
		Sum += width;
	}

	FragColor = vec4(Color / Sum, 1.0);
}
