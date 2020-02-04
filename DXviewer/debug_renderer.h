#pragma once

#include "math_types.h"

// Interface to the debug renderer
namespace end
{
	namespace debug_renderer
	{
		void add_line(XMFLOAT3 point_a, XMFLOAT3 point_b, XMFLOAT4 color_a, XMFLOAT4 color_b);

		inline void add_line(XMFLOAT3 p, XMFLOAT3 q, XMFLOAT4 color) { add_line(p, q, color, color); }

		void clear_lines();

		const colored_vertex* get_line_verts();

		size_t get_line_vert_count();

		size_t get_line_vert_capacity();
	}
}