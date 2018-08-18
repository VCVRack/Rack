#pragma once

#include "../utils/meta.hpp"
#include "../common/constants.hpp"

/*
http://recherche.ircam.fr/pub/dafx11/Papers/66_e.pdf
*/

namespace rack_plugin_JE {

class Diode
{
public:
	inline float operator() (float v)
	{
		float av = fabs(v);
		if (av <= m_vb)
			return 0.f;

		if (av <= m_vl)
		{
			const float v_minus_vb = av - m_vb;
			return m_h * v_minus_vb * v_minus_vb / m_vl_vb_denom;
		}

		return m_h * av - m_h_vl + m_vl_add;
	}

	inline void setVb(float newVb)
	{
		if (meta::updateIfDifferent(m_vb, newVb))
		{
			m_vl = m_vl_minus_vb + m_vb;
			m_vl_vb_denom = 2.f * (m_vl_minus_vb);
			m_vl_add = m_h * m_vl_minus_vb * m_vl_minus_vb / m_vl_vb_denom;
		}
	}

	inline void setVlMinusVb(float newVlMinusVb)
	{
		if (meta::updateIfDifferent(m_vl_minus_vb, newVlMinusVb))
		{
			m_vl = m_vl_minus_vb + m_vb;
			m_vl_vb_denom = 2.f * (m_vl_minus_vb);
			m_vl_add = m_h * m_vl_minus_vb * m_vl_minus_vb / m_vl_vb_denom;
			m_h_vl = m_h * m_vl;
		}
	}

	inline void setH(float newH)
	{
		if (meta::updateIfDifferent(m_h, newH))
		{
			m_vl_add = m_h * m_vl_minus_vb * m_vl_minus_vb / m_vl_vb_denom;
			m_h_vl = m_h * m_vl;
		}
	}

private:
	float m_vb = 0.2f; // diode forward-bias voltage
	float m_vl = 0.4f; // voltage beyond which the function is linear
	float m_h = 1.f;  // slope of the linear section

	float m_vl_minus_vb = m_vl - m_vb;
	float m_vl_vb_denom = 2.f * (m_vl_minus_vb);
	float m_vl_add = m_h * m_vl_minus_vb * m_vl_minus_vb / m_vl_vb_denom;
	float m_h_vl = m_h * m_vl;
};

} // namespace rack_plugin_JE
