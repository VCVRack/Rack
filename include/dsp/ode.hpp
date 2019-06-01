#pragma once
#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/** The callback function `f` in each of these stepping functions must have the signature

	void f(T t, const T x[], T dxdt[])

A capturing lambda is ideal for this.
For example, the following solves the system x''(t) = -x(t) using a fixed timestep of 0.01 and initial conditions x(0) = 1, x'(0) = 0.

	float x[2] = {1.f, 0.f};
	float dt = 0.01f;
	for (float t = 0.f; t < 1.f; t += dt) {
		rack::ode::stepRK4(t, dt, x, 2, [&](float t, const float x[], float dxdt[]) {
			dxdt[0] = x[1];
			dxdt[1] = -x[0];
		});
		printf("%f\n", x[0]);
	}

*/

/** Solves an ODE system using the 1st order Euler method */
template <typename T, typename F>
void stepEuler(T t, T dt, T x[], int len, F f) {
	T k[len];

	f(t, x, k);
	for (int i = 0; i < len; i++) {
		x[i] += dt * k[i];
	}
}

/** Solves an ODE system using the 2nd order Runge-Kutta method */
template <typename T, typename F>
void stepRK2(T t, T dt, T x[], int len, F f) {
	T k1[len];
	T k2[len];
	T yi[len];

	f(t, x, k1);

	for (int i = 0; i < len; i++) {
		yi[i] = x[i] + k1[i] * dt / 2.f;
	}
	f(t + dt / 2.f, yi, k2);

	for (int i = 0; i < len; i++) {
		x[i] += dt * k2[i];
	}
}

/** Solves an ODE system using the 4th order Runge-Kutta method */
template <typename T, typename F>
void stepRK4(T t, T dt, T x[], int len, F f) {
	T k1[len];
	T k2[len];
	T k3[len];
	T k4[len];
	T yi[len];

	f(t, x, k1);

	for (int i = 0; i < len; i++) {
		yi[i] = x[i] + k1[i] * dt / 2.f;
	}
	f(t + dt / 2.f, yi, k2);

	for (int i = 0; i < len; i++) {
		yi[i] = x[i] + k2[i] * dt / 2.f;
	}
	f(t + dt / 2.f, yi, k3);

	for (int i = 0; i < len; i++) {
		yi[i] = x[i] + k3[i] * dt;
	}
	f(t + dt, yi, k4);

	for (int i = 0; i < len; i++) {
		x[i] += dt * (k1[i] + 2.f * k2[i] + 2.f * k3[i] + k4[i]) / 6.f;
	}
}


} // namespace dsp
} // namespace rack
