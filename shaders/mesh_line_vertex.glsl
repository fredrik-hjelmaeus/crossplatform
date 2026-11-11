#version 330 core

layout (location = 0) in vec2 rectangle;

uniform vec3 lineStart;
uniform vec3 lineEnd;
uniform float lineWidth;
uniform vec2 viewportSize;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{ 
  // Go to clip space
  mat4 modelToClip = projection * view * model;
  vec4 clipStart = modelToClip * vec4(lineStart, 1.0);
  vec4 clipEnd   = modelToClip * vec4(lineEnd,   1.0);

  // Near plane clip check
  float startNear = clipStart.z + clipStart.w;
  float endNear  = clipEnd.z + clipEnd.w;

  // If both endpoints are outside, reject the segment
  // We reject it by writing all verts outside of visible view
  if (startNear < 0.0 && endNear < 0.0)
  {
      gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
      return;
  }

  // If any point is outside, we calc intersection
  // with near
  if(startNear < 0.0 || endNear < 0.0)
  {
    float t = startNear / (startNear - endNear);

    vec4 intersection = clipStart + t * (clipEnd - clipStart);

    if(startNear < 0.0)
    {
     clipStart = intersection; 
    }
    else 
    {
      clipEnd = intersection;
    }
  }

  // Guard for division with zero
  if (clipStart.w <= 0.000001 || clipEnd.w <= 0.000001)
  {
      gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
      return;
  }

  // Guard against divide by zero
  if(viewportSize.x <= 0.000001 || viewportSize.y <= 0.000001)
  {
    gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
    return;
  }

  // Go from clip space to NDC
  vec3 ndcStart = clipStart.xyz / clipStart.w;
  vec3 ndcEnd   = clipEnd.xyz   / clipEnd.w;

  // Convert ndc to pixel values
  float dx = (ndcEnd.x - ndcStart.x) * viewportSize.x  * 0.5;
  float dy = (ndcEnd.y - ndcStart.y) * viewportSize.y * 0.5;

  // Measure the projected line length in pixels
  float pixelLength = sqrt(dx * dx + dy * dy);

  // Check so we dont divide by zero next
  if (pixelLength < 0.00001) {
    gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
    return;
  }

  vec2 sideways = vec2(
      -dy / pixelLength,
      dx / pixelLength
  );

  float halfWidth = lineWidth * 0.5;

  vec2 halfWidthPixels = sideways * halfWidth;

  // Convert back to ndc
  vec2 halfWidthNdc = vec2(
      halfWidthPixels.x * 2.0 / viewportSize.x,
      halfWidthPixels.y * 2.0 / viewportSize.y
  );

  // Use the width data and build the rectangle
  vec4 clipRect = (rectangle.x < 0.5) ? clipStart : clipEnd;

  clipRect.xy += halfWidthNdc * rectangle.y * clipRect.w;

  gl_Position = clipRect;

}