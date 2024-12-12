#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

out vec3 fColor;

uniform int switcher;

void main()
{
    fColor = vec3(0, 0, 0);
    if(switcher == 0)
    {
        if(gl_in[0].gl_Position.x > -0.05 && gl_in[0].gl_Position.x < 0.05)
        {
            if(gl_in[0].gl_Position.y > 0.9 && gl_in[0].gl_Position.y < 1.1)
            {
                fColor = vec3(1, 0, 0);
            }
        }

        gl_Position = gl_in[0].gl_Position;
        EmitVertex();

        gl_Position = vec4(gl_Position.x, gl_Position.y * -1, 0.0, 1.0);
        EmitVertex();


    }else
    {
        if(gl_in[0].gl_Position.x > -1.1 && gl_in[0].gl_Position.x < -0.9)
        {
            if(gl_in[0].gl_Position.y > -0.05 && gl_in[0].gl_Position.y < 0.05)
            {
                fColor = vec3(0, 1, 0);
            }
        }

        gl_Position = gl_in[0].gl_Position;
        EmitVertex();

        gl_Position = vec4(gl_Position.x * -1, gl_Position.y, 0.0, 1.0);
        EmitVertex();

    }

    EndPrimitive();

}