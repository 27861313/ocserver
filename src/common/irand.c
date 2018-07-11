#include "irand.h"

#define IRAND_N	16
#define IRAND_MASK	((1 << (IRAND_N - 1)) + (1 << (IRAND_N - 1)) - 1)
#define IRAND_LOW(x)	((unsigned)(x) & IRAND_MASK)
#define IRAND_HIGH(x)	IRAND_LOW((x) >> IRAND_N)
#define IRAND_MUL(x, y, z)	{ int32_i l = (long)(x) * (long)(y); \
		(z)[0] = IRAND_LOW(l); (z)[1] = IRAND_HIGH(l); }
#define IRAND_CARRY(x, y)	((int32_t)(x) + (long)(y) > IRAND_MASK)
#define IRAND_ADDEQU(x, y, z)	(z = IRAND_CARRY(x, (y)), x = IRAND_LOW(x + (y)))
#define IRAND_X0	0x330E
#define IRAND_X1	0xABCD
#define IRAND_X2	0x1234
#define IRAND_A0	0xE66D
#define IRAND_A1	0xDEEC
#define IRAND_A2	0x5
#define IRAND_C	    0xB
#define IRAND_SET3(x, x0, x1, x2)	((x)[0] = (x0), (x)[1] = (x1), (x)[2] = (x2))
#define IRAND_SETLOW(x, y, n) IRAND_SET3(x, IRAND_LOW((y)[n]), IRAND_LOW((y)[(n)+1]), IRAND_LOW((y)[(n)+2]))
#define IRAND_SEED(x0, x1, x2) (IRAND_SET3(x, x0, x1, x2), IRAND_SET3(a, IRAND_A0, IRAND_A1, IRAND_A2), c = IRAND_C)
#define IRAND_REST(v)	for (i = 0; i < 3; i++) { xsubi[i] = x[i]; x[i] = temp[i]; } \
		return (v);
#define IRAND_HI_BIT	(1L << (2 * IRAND_N - 1))

static uint32_i x[3] = { IRAND_X0, IRAND_X1, IRAND_X2 }, a[3] = { IRAND_A0, IRAND_A1, IRAND_A2 }, c = IRAND_C;
static void next(void);

int32_i iLrand48() {
	next();
	return (((int32_t)x[2] << (IRAND_N - 1)) + (x[1] >> 1));
}

void iSrand48(int32_i seedval) {
	IRAND_SEED(IRAND_X0, IRAND_LOW(seedval), IRAND_HIGH(seedval));
}

static void next(void) {
	uint32_i p[2], q[2], r[2], carry0, carry1;

	IRAND_MUL(a[0], x[0], p);
	IRAND_ADDEQU(p[0], c, carry0);
	IRAND_ADDEQU(p[1], carry0, carry1);
	IRAND_MUL(a[0], x[1], q);
	IRAND_ADDEQU(p[1], q[0], carry0);
	IRAND_MUL(a[1], x[0], r);
	x[2] = IRAND_LOW(carry0 + carry1 + IRAND_CARRY(p[1], r[0]) + q[1] + r[1] +
		a[0] * x[2] + a[1] * x[1] + a[2] * x[0]);
	x[1] = IRAND_LOW(p[1] + r[0]);
	x[0] = IRAND_LOW(p[0]);
}