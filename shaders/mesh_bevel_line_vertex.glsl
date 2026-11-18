#version 330 core

uniform vec3 pointA;
uniform vec3 pointB;
uniform vec3 pointC;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 viewportSize;

uniform float lineWidth;

void main()
{
    mat4 modelToClip = projection * view * model;

    vec4 clipA = modelToClip * vec4(pointA, 1.0);
    vec4 clipB = modelToClip * vec4(pointB, 1.0);
    vec4 clipC = modelToClip * vec4(pointC, 1.0);

    // Handle near plane clipping for the bevels
    float nearA = clipA.z + clipA.w;
    float nearB = clipB.z + clipB.w;
    float nearC = clipC.z + clipC.w;

    // The join itself is centered at B.
    if (nearB < 0.0 || clipB.w <= 0.000001 ||
        viewportSize.x <= 0.0 || viewportSize.y <= 0.0)
    {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    // If A is outside, replace it with the A→B intersection.
    if (nearA < 0.0)
    {
        float t = nearA / (nearA - nearB);
        clipA = clipA + t * (clipB - clipA);
    }

    // If C is outside, replace it with the C→B intersection.
    if (nearC < 0.0)
    {
        float t = nearC / (nearC - nearB);
        clipC = clipC + t * (clipB - clipC);
    }

    if (clipA.w <= 0.000001 || clipC.w <= 0.000001)
    {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }


    vec2 ndcA = clipA.xy / clipA.w;
    vec2 ndcB = clipB.xy / clipB.w;
    vec2 ndcC = clipC.xy / clipC.w;

    // Difference between A and B ,measured in screen pixels.
    // Multiply by half viewPort size converts to pixels.
    vec2 incoming = (ndcB - ndcA) * viewportSize * 0.5;

    // Get direction
    float incomingLength = length(incoming);
    if(incomingLength < 0.00001)
    {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    } 
    vec2 incomingDir = incoming / incomingLength;

    // Create a side vector 90 deg. perpendicular to A-B.
    vec2 incomingSide = vec2(-incomingDir.y, incomingDir.x);

    // Do the same perpendicular calc. for C-B.
    vec2 outgoing = (ndcC - ndcB) * viewportSize * 0.5;

    float outgoingLength = length(outgoing);

    if (outgoingLength < 0.00001)
    {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    vec2 outgoingDir = outgoing / outgoingLength;
    vec2 outgoingSide = vec2(-outgoingDir.y, outgoingDir.x);

    float turn = incomingDir.x * outgoingDir.y - incomingDir.y * outgoingDir.x;

    if (abs(turn) < 0.00001)
    {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    float outsideSign = -sign(turn);
    float halfWidth = lineWidth * 0.5;

    vec2 offsets[3] = vec2[3](
        vec2(0.0,  0.0),
        incomingSide * outsideSign * halfWidth,
        outgoingSide * outsideSign * halfWidth
    );

    vec2 offsetPixels = offsets[gl_VertexID];
    vec2 offsetNdc = offsetPixels * 2.0 / viewportSize;

    clipB.xy += offsetNdc * clipB.w;
    gl_Position = clipB;
}
