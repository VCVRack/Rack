#include "widgets.hpp"


// #define DEBUG_ONLY(x) x
#define DEBUG_ONLY(x)

namespace rack {


static NVGcolor getNVGColor(uint32_t color) {
	return nvgRGBA(
		(color >> 0) & 0xff,
		(color >> 8) & 0xff,
		(color >> 16) & 0xff,
		(color >> 24) & 0xff);
}

static NVGpaint getPaint(NVGcontext *vg, NSVGpaint *p) {
	assert(p->type == NSVG_PAINT_LINEAR_GRADIENT || p->type == NSVG_PAINT_RADIAL_GRADIENT);
	NSVGgradient *g = p->gradient;
	assert(g->nstops >= 1);
	NVGcolor icol = getNVGColor(g->stops[0].color);
	NVGcolor ocol = getNVGColor(g->stops[g->nstops - 1].color);

	float inverse[6];
	nvgTransformInverse(inverse, g->xform);
	DEBUG_ONLY(printf("			inverse: %f %f %f %f %f %f\n", inverse[0], inverse[1], inverse[2], inverse[3], inverse[4], inverse[5]);)
	Vec s, e;
	DEBUG_ONLY(printf("			sx: %f sy: %f ex: %f ey: %f\n", s.x, s.y, e.x, e.y);)
	// Is it always the case that the gradient should be transformed from (0, 0) to (0, 1)?
	nvgTransformPoint(&s.x, &s.y, inverse, 0, 0);
	nvgTransformPoint(&e.x, &e.y, inverse, 0, 1);
	DEBUG_ONLY(printf("			sx: %f sy: %f ex: %f ey: %f\n", s.x, s.y, e.x, e.y);)

	NVGpaint paint;
	if (p->type == NSVG_PAINT_LINEAR_GRADIENT)
		paint = nvgLinearGradient(vg, s.x, s.y, e.x, e.y, icol, ocol);
	else
		paint = nvgRadialGradient(vg, s.x, s.y, 0.0, 160, icol, ocol);
	return paint;
}

/** Returns the parameterized value of the line p2--p3 where it intersects with p0--p1 */
static float getLineCrossing(Vec p0, Vec p1, Vec p2, Vec p3) {
	Vec b = p2.minus(p0);
	Vec d = p1.minus(p0);
	Vec e = p3.minus(p2);
	float m = d.x * e.y - d.y * e.x;
	// Check if lines are parallel, or if either pair of points are equal
	if (fabsf(m) < 1e-6)
		return NAN;
	return -(d.x * b.y - d.y * b.x) / m;
}

static void drawSVG(NVGcontext *vg, NSVGimage *svg) {
	DEBUG_ONLY(printf("new image: %g x %g px\n", svg->width, svg->height);)
	int shapeIndex = 0;
	// Iterate shape linked list
	for (NSVGshape *shape = svg->shapes; shape; shape = shape->next, shapeIndex++) {
		DEBUG_ONLY(printf("	new shape: %d id \"%s\", fillrule %d, from (%f, %f) to (%f, %f)\n", shapeIndex, shape->id, shape->fillRule, shape->bounds[0], shape->bounds[1], shape->bounds[2], shape->bounds[3]);)

		// Visibility
		if (!(shape->flags & NSVG_FLAGS_VISIBLE))
			continue;

		nvgSave(vg);

		// Opacity
		if (shape->opacity < 1.0)
			nvgGlobalAlpha(vg, shape->opacity);

		// Build path
		nvgBeginPath(vg);

		// Iterate path linked list
		for (NSVGpath *path = shape->paths; path; path = path->next) {
			DEBUG_ONLY(printf("		new path: %d points, %s, from (%f, %f) to (%f, %f)\n", path->npts, path->closed ? "closed" : "open", path->bounds[0], path->bounds[1], path->bounds[2], path->bounds[3]);)

			nvgMoveTo(vg, path->pts[0], path->pts[1]);
			for (int i = 1; i < path->npts; i += 3) {
				float *p = &path->pts[2*i];
				nvgBezierTo(vg, p[0], p[1], p[2], p[3], p[4], p[5]);
				// nvgLineTo(vg, p[4], p[5]);
				DEBUG_ONLY(printf("			bezier (%f, %f) to (%f, %f)\n", p[-2], p[-1], p[4], p[5]);)
			}

			// Close path
			if (path->closed)
				nvgClosePath(vg);

			// Compute whether this is a hole or a solid.
			// Assume that no paths are crossing (usually true for normal SVG graphics).
			// Also assume that the topology is the same if we use straight lines rather than Beziers (not always the case but usually true).
			// Using the even-odd fill rule, if we draw a line from a point on the path to a point outside the boundary (e.g. top left) and count the number of times it crosses another path, the parity of this count determines whether the path is a hole (odd) or solid (even).
			int crossings = 0;
			Vec p0 = Vec(path->pts[0], path->pts[1]);
			Vec p1 = Vec(path->bounds[0] - 1.0, path->bounds[1] - 1.0);
			// Iterate all other paths
			for (NSVGpath *path2 = shape->paths; path2; path2 = path2->next) {
				if (path2 == path)
					continue;

				// Iterate all lines on the path
				if (path2->npts < 4)
					continue;
				for (int i = 1; i < path2->npts + 3; i += 3) {
					float *p = &path2->pts[2*i];
					// The previous point
					Vec p2 = Vec(p[-2], p[-1]);
					// The current point
					Vec p3 = (i < path2->npts) ? Vec(p[4], p[5]) : Vec(path2->pts[0], path2->pts[1]);
					float crossing = getLineCrossing(p0, p1, p2, p3);
					float crossing2 = getLineCrossing(p2, p3, p0, p1);
					if (0.0 <= crossing && crossing < 1.0 && 0.0 <= crossing2) {
						crossings++;
					}
				}
			}

			if (crossings % 2 == 0)
				nvgPathWinding(vg, NVG_SOLID);
			else
				nvgPathWinding(vg, NVG_HOLE);

/*
			// Shoelace algorithm for computing the area, and thus the winding direction
			float area = 0.0;
			Vec p0 = Vec(path->pts[0], path->pts[1]);
			for (int i = 1; i < path->npts; i += 3) {
				float *p = &path->pts[2*i];
				Vec p1 = (i < path->npts) ? Vec(p[4], p[5]) : Vec(path->pts[0], path->pts[1]);
				area += 0.5 * (p1.x - p0.x) * (p1.y + p0.y);
				printf("%f %f, %f %f\n", p0.x, p0.y, p1.x, p1.y);
				p0 = p1;
			}
			printf("%f\n", area);

			if (area < 0.0)
				nvgPathWinding(vg, NVG_CCW);
			else
				nvgPathWinding(vg, NVG_CW);
*/
		}

		// Fill shape
		if (shape->fill.type) {
			switch (shape->fill.type) {
				case NSVG_PAINT_COLOR: {
					NVGcolor color = getNVGColor(shape->fill.color);
					nvgFillColor(vg, color);
					DEBUG_ONLY(printf("		fill color (%g, %g, %g, %g)\n", color.r, color.g, color.b, color.a);)
				} break;
				case NSVG_PAINT_LINEAR_GRADIENT:
				case NSVG_PAINT_RADIAL_GRADIENT: {
					NSVGgradient *g = shape->fill.gradient;
					(void)g;
					DEBUG_ONLY(printf("		gradient: type: %s xform: %f %f %f %f %f %f spread: %d fx: %f fy: %f nstops: %d\n", (shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT ? "linear" : "radial"), g->xform[0], g->xform[1], g->xform[2], g->xform[3], g->xform[4], g->xform[5], g->spread, g->fx, g->fy, g->nstops);)
					for (int i = 0; i < g->nstops; i++) {
						DEBUG_ONLY(printf("			stop: #%08x\t%f\n", g->stops[i].color, g->stops[i].offset);)
					}
					nvgFillPaint(vg, getPaint(vg, &shape->fill));
				} break;
			}
			nvgFill(vg);
		}

		// Stroke shape
		if (shape->stroke.type) {
			nvgStrokeWidth(vg, shape->strokeWidth);
			// strokeDashOffset, strokeDashArray, strokeDashCount not yet supported
			nvgLineCap(vg, (NVGlineCap) shape->strokeLineCap);
			nvgLineJoin(vg, (int) shape->strokeLineJoin);

			switch (shape->stroke.type) {
				case NSVG_PAINT_COLOR: {
					NVGcolor color = getNVGColor(shape->stroke.color);
					nvgStrokeColor(vg, color);
					DEBUG_ONLY(printf("		stroke color (%g, %g, %g, %g)\n", color.r, color.g, color.b, color.a);)
				} break;
				case NSVG_PAINT_LINEAR_GRADIENT: {
					// NSVGgradient *g = shape->stroke.gradient;
					// printf("		lin grad: %f\t%f\n", g->fx, g->fy);
				} break;
			}
			nvgStroke(vg);
		}

		nvgRestore(vg);

		// if (std::string(shape->id) == "rect19347")
		// 	exit(0);
	}

	DEBUG_ONLY(printf("\n");)
}


void SVGWidget::wrap() {
	if (svg && svg->handle) {
		box.size = Vec(svg->handle->width, svg->handle->height);
	}
	else {
		box.size = Vec();
	}
}

void SVGWidget::setSVG(std::shared_ptr<SVG> svg) {
	this->svg = svg;
	wrap();
}

void SVGWidget::draw(NVGcontext *vg) {
	if (svg && svg->handle) {
		// printf("drawing svg %f %f\n", box.size.x, box.size.y);
		drawSVG(vg, svg->handle);
	}
}


} // namespace rack
