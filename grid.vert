#version 130
in highp vec2 position;
in highp vec3 color;
out vec4 vColor;

// We need width / height of the screen in
// their respective units.
uniform mat2 grid_to_heap_map;
uniform mat2 heap_to_screen_map;
uniform vec2 grid_to_heap_translation;
uniform vec2 heap_to_screen_translation;

void main(void)
{
    gl_Position =
            vec4(heap_to_screen_map *
            (grid_to_heap_map * position + grid_to_heap_translation)
            + heap_to_screen_translation, 0.0, 1.0);
    vColor = vec4(color, 1.0);
}
