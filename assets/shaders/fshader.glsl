#version 130

uniform sampler2D frame;
uniform float s;
uniform int level;

struct Pair{
    vec3 color1, color2;
};

Pair pairs[10] = Pair[](Pair(vec3(060.0/255.0,188.0/255.0,252.0/255.0), vec3(000.0/255.0,088.0/255.0,248.0/255.0)),
                            Pair(vec3(184.0/255.0,248.0/255.0,024.0/255.0), vec3(000.0/255.0,168.0/255.0,252.0/000.0)),
                            Pair(vec3(248.0/255.0,120.0/255.0,248.0/255.0), vec3(216.0/255.0,000.0/255.0,204.0/255.0)),
                            Pair(vec3(088.0/255.0,216.0/255.0,084.0/255.0), vec3(000.0/255.0,088.0/255.0,248.0/255.0)),
                            Pair(vec3(088.0/255.0,248.0/255.0,152.0/255.0), vec3(228.0/255.0,000.0/255.0,088.0/255.0)),
                            Pair(vec3(104.0/255.0,136.0/255.0,252.0/255.0), vec3(088.0/255.0,248.0/255.0,152.0/255.0)),
                            Pair(vec3(124.0/255.0,124.0/255.0,124.0/255.0), vec3(248.0/255.0,056.0/255.0,000.0/255.0)),
                            Pair(vec3(168.0/255.0,000.0/255.0,032.0/255.0), vec3(104.0/255.0,068.0/255.0,252.0/255.0)),
                            Pair(vec3(248.0/255.0,056.0/255.0,000.0/255.0), vec3(000.0/255.0,088.0/255.0,248.0/255.0)),
                            Pair(vec3(252.0/255.0,160.0/255.0,068.0/255.0), vec3(248.0/255.0,056.0/255.0,000.0/255.0)));
out vec4 color;

void main() {
    Pair p = pairs[level%10];
    vec2 pos = gl_FragCoord.xy / vec2(s*255.0,s*222.0);
    color = texture(frame, pos);
    if(color == vec4(1,0,0,1))
        color = vec4(p.color1.rgb*0.75,1);
    else if(color == vec4(0,0,1,1))
        color = vec4(p.color2.rgb*0.75,1);
    return;
}