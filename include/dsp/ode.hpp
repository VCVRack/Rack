#pragma once


namespace rack {

/** The callback function `f` in each of these stepping functions must have the signature
	void f(float x, const float y[], float dydt[])
A capturing lambda is ideal for this.
*/

/** Solves an ODE system using the 1st order Euler method */
template<typename F>
void stepEuler(float x, float dx, float y[], int len, F f) {
	float k[len];

	f(x, y, k);
	for (int i = 0; i < len; i++) {
		y[i] += dx * k[i];
	}
}

/** Solves an ODE system using the 2nd order Runge-Kutta method */
template<typename F>
void stepRK2(float x, float dx, float y[], int len, F f) {
	float k1[len];
	float k2[len];
	float yi[len];

	f(x, y, k1);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k1[i] * dx / 2.f;
	}
	f(x + dx / 2.f, yi, k2);

	for (int i = 0; i < len; i++) {
		y[i] += dx * k2[i];
	}
}

/** Solves an ODE system using the 4th order Runge-Kutta method */
template<typename F>
void stepRK4(float x, float dx, float y[], int len, F f) {
	float k1[len];
	float k2[len];
	float k3[len];
	float k4[len];
	float yi[len];

	f(x, y, k1);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k1[i] * dx / 2.f;
	}
	f(x + dx / 2.f, yi, k2);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k2[i] * dx / 2.f;
	}
	f(x + dx / 2.f, yi, k3);

	for (int i = 0; i < len; i++) {
		yi[i] = y[i] + k3[i] * dx;
	}
	f(x + dx, yi, k4);

	for (int i = 0; i < len; i++) {
		y[i] += dx * (k1[i] + 2.f * k2[i] + 2.f * k3[i] + k4[i]) / 6.f;
	}
}

} // namespace rack
