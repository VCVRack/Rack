#pragma once


namespace rack {

typedef void (*stepCallback)(float x, const float y[], float dydt[]);

/** Solve an ODE system using the 1st order Euler method */
inline void stepEuler(stepCallback f, float x, float dx, float y[], int len) {
	float k[len];

	f(x, y, k);
	for (int i = 0; i < len; i++) {
		y[i] += dx * k[i];
	}
}

/** Solve an ODE system using the 4th order Runge-Kutta method */
inline void stepRK4(stepCallback f, float x, float dx, float y[], int len) {
	float k1[len];
	float k2[len];
	float k3[len];
	float k4[len];
	float yi[len];

	f(x, y, k1);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k1[i] * dx / 2.0;
	}
	f(x + dx / 2.0, yi, k2);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k2[i] * dx / 2.0;
	}
	f(x + dx / 2.0, yi, k3);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k3[i] * dx;
	}
	f(x + dx, yi, k4);

	for (int i = 0; i < len; i++) {
		y[i] += dx * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]) / 6.0;
	}
}

} // namespace rack
