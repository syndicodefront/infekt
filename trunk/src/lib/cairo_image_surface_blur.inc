#if 0

// Steve Hanov, 2009
// Released into the public domain.
// http://stevehanov.ca/blog/index.php?id=53

static void cairo_image_surface_blur( cairo_surface_t* surface, double radius )
{
	// get width, height
	int width = cairo_image_surface_get_width( surface );
	int height = cairo_image_surface_get_height( surface );
	unsigned char* dst = new unsigned char[width*height*4];
	unsigned* precalc = 
		new unsigned[width*height*sizeof(unsigned)];
	unsigned char* src = cairo_image_surface_get_data( surface );
	double mul=1.f/((radius*2)*(radius*2));
	int rint = (int)radius;

	// The number of times to perform the averaging. According to wikipedia,
	// three iterations is good enough to pass for a gaussian.
	const int MAX_ITERATIONS = 3; 
	int iteration;
	int channel;

	memcpy( dst, src, width*height*4 );

	for ( iteration = 0; iteration < MAX_ITERATIONS; iteration++ ) {
		for( channel = 0; channel < 4; channel++ ) {
			int x,y;

			// precomputation step.
			unsigned char* pix = src;
			unsigned* pre = precalc;

			pix += channel;
			for (y=0;y<height;y++) {
				for (x=0;x<width;x++) {
					int tot=pix[0];
					if (x>0) tot+=pre[-1];
					if (y>0) tot+=pre[-width];
					if (x>0 && y>0) tot-=pre[-width-1];
					*pre++=tot;
					pix += 4;
				}
			}

			// blur step.
			pix = dst + rint * width * 4 + rint * 4 + channel;
			for (y=rint;y<height-rint;y++) {
				for (x=rint;x<width-rint;x++) {
					int l = x - rint, t = y - rint;
					int r = x + rint, b = y + rint;
					int tot = precalc[r+b*width] + precalc[l+t*width] - 
						precalc[l+b*width] - precalc[r+t*width];
					*pix=(unsigned char)(tot*mul);
					pix += 4;
				}
				pix += rint * 2 * 4;
			}
		}
		memcpy( src, dst, width*height*4 );
	}

	delete[] dst;
	delete[] precalc;
}

#else
/*
 * Copyright © 2008 Kristian Hagsberg
 * Copyright © 2009 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */


typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

void cairo_blur_image_surface(cairo_surface_t *surface, int radius)
{
	cairo_surface_t *tmp;
	int width, height;
	int src_stride, dst_stride;
	int x, y, z, w;
	uint8_t *src, *dst;
	uint32_t *s, *d, a, p;
	int i, j, k;
	uint8_t kernel[17];
	const int size = sizeof(kernel)/sizeof(kernel[0]);
	const int half = size / 2;

	if (cairo_surface_status (surface))
		return;

	width = cairo_image_surface_get_width (surface);
	height = cairo_image_surface_get_height (surface);

	switch (cairo_image_surface_get_format (surface)) {
	case CAIRO_FORMAT_A1:
	default:
		/* Don't even think about it! */
		return;

	case CAIRO_FORMAT_A8:
		/* Handle a8 surfaces by effectively unrolling the loops by a
		* factor of 4 - this is safe since we know that stride has to be a
		* multiple of uint32_t. */
		width /= 4;
		break;

	case CAIRO_FORMAT_RGB24:
	case CAIRO_FORMAT_ARGB32:
		break;
	}

	tmp = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	if (cairo_surface_status (tmp))
		return;

	src = cairo_image_surface_get_data (surface);
	src_stride = cairo_image_surface_get_stride (surface);

	dst = cairo_image_surface_get_data (tmp);
	dst_stride = cairo_image_surface_get_stride (tmp);

	a = 0;
	for (i = 0; i < size; i++) {
		double f = i - half;
		kernel[i] = (uint8_t)(exp(- f * f / 30.0) * 80.0);
		a += kernel[i];
	}

	/* Horizontally blur from surface -> tmp */
	for (i = 0; i < height; i++) {
		s = (uint32_t *) (src + i * src_stride);
		d = (uint32_t *) (dst + i * dst_stride);
		for (j = 0; j < width; j++) {
			if (!(radius < j && j < width - radius)) {
				d[j] = s[j];
				continue;
			}

			x = y = z = w = 0;
			for (k = 0; k < size; k++) {
				if (j - half + k < 0 || j - half + k >= width)
					continue;

				p = s[j - half + k];

				x += ((p >> 24) & 0xff) * kernel[k];
				y += ((p >> 16) & 0xff) * kernel[k];
				z += ((p >>  8) & 0xff) * kernel[k];
				w += ((p >>  0) & 0xff) * kernel[k];
			}
			d[j] = (x / a << 24) | (y / a << 16) | (z / a << 8) | w / a;
		}
	}

	/* Then vertically blur from tmp -> surface */
	for (i = 0; i < height; i++) {
		s = (uint32_t *) (dst + i * dst_stride);
		d = (uint32_t *) (src + i * src_stride);
		for (j = 0; j < width; j++) {
			if (!(radius <= i && i < height - radius)) {
				d[j] = s[j];
				continue;
			}

			x = y = z = w = 0;
			for (k = 0; k < size; k++) {
				if (i - half + k < 0 || i - half + k >= height)
					continue;

				s = (uint32_t *) (dst + (i - half + k) * dst_stride);
				p = s[j];

				x += ((p >> 24) & 0xff) * kernel[k];
				y += ((p >> 16) & 0xff) * kernel[k];
				z += ((p >>  8) & 0xff) * kernel[k];
				w += ((p >>  0) & 0xff) * kernel[k];
			}
			d[j] = (x / a << 24) | (y / a << 16) | (z / a << 8) | w / a;
		}
	}

	cairo_surface_destroy(tmp);
	cairo_surface_mark_dirty(surface);
}

#endif
