#version 400 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float Near;
in float Far;

uniform sampler2D tex;

void    main() {
    vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);

    vec4 color = texture(tex, uv);
    if (length(color.xyz) < 0.001) // that's damn ugly, sorry...
        discard;
    FragColor = vec4(color.xyz, 1.0);
}