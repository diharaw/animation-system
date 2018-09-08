
out vec3 PS_OUT_Color;

in vec3 PS_IN_FragPos;
in vec3 PS_IN_Normal;

void main()
{
	vec3 light_pos = vec3(-200.0, 200.0, 0.0);
	vec3 n = normalize(PS_IN_Normal);
	vec3 l = normalize(light_pos - PS_IN_FragPos);
	float lambert = max(0.0f, dot(n, l));
    vec3 diffuse = vec3(1.0f);
	vec3 ambient = diffuse * 0.03;
	vec3 color = diffuse * lambert + ambient;
    PS_OUT_Color = color;
}
