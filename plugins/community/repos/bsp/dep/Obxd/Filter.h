/*
	==============================================================================
	This file is part of Obxd synthesizer.

	Copyright © 2013-2014 Filatov Vadim
	
	Contact author via email :
	justdat_@_e1.ru

	This file may be licensed under the terms of of the
	GNU General Public License Version 2 (the ``GPL'').

	Software distributed under the License is distributed
	on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
	express or implied. See the GPL for the specific language
	governing rights and limitations.

	You should have received a copy of the GPL along with this
	program. If not, go to http://www.gnu.org/licenses/gpl.html
	or write to the Free Software Foundation, Inc.,  
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
	==============================================================================
 */
#pragma once
/* #include "ObxdVoice.h" */
#include <math.h>

namespace rack_plugin_bsp {

class Filter
{
private:
	float s1,s2,s3,s4;
	float R;
	float R24;
	float rcor,rcorInv;
	float rcor24,rcor24Inv;

	//24 db multimode
	float mmt;
	int mmch;
public:
	float SampleRate;
	float sampleRateInv;
	bool bandPassSw;
	float mm;
	bool selfOscPush;
	Filter()
	{
		selfOscPush = false;
		bandPassSw = false;
		mm=0;
		s1=s2=s3=s4=0;
		SampleRate = 44000;
		sampleRateInv = 1 / SampleRate;
		rcor = 500.0f / 44000.f;
		rcorInv = 1.0f / rcor;
		rcor24 = 970.0f / 44000.f;
		rcor24Inv = 1.0f / rcor24;
		R=1;
		R24=0;
	}
   void reset(void)
   {
		s1=s2=s3=s4=0;
   }
	void setMultimode(float m)
	{
		mm = m;
		mmch = (int)(mm * 3.0f);
		mmt = mm*3-mmch;
	}
	inline void setSampleRate(float sr)
	{
		SampleRate = sr;
		sampleRateInv = 1/SampleRate;
		float rcrate =sqrtf((44000.f/SampleRate));
		rcor = (500.0f / 44000.f)*rcrate;
		rcor24 = (970.0f / 44000.f)*rcrate;
		rcorInv = 1.f / rcor;
		rcor24Inv = 1.f / rcor24;
	}
	inline void setResonance(float res)
	{
		R = 1.f - res;
		R24 =( 3.5f * res);
	}
	
	inline float diodePairResistanceApprox(float x)
	{
		return (((((0.0103592f)*x + 0.00920833f)*x + 0.185f)*x + 0.05f )*x + 1.0f);
		//Taylor approx of slightly mismatched diode pair
	}
	//resolve 0-delay feedback
	inline float NR(float sample, float g)
	{ 
		//calculating feedback non-linear transconducance and compensated for R (-1)
		//Boosting non-linearity
		float tCfb;
		if(!selfOscPush)
			tCfb = diodePairResistanceApprox(s1*0.0876f) - 1.0f;
		else
			tCfb = diodePairResistanceApprox(s1*0.0876f) - 1.035f;
		//float tCfb = 0;
		//disable non-linearity == digital filter

		//resolve linear feedback
		float y = ((sample - 2*(s1*(R+tCfb)) - g*s1  - s2)/(1+ g*(2*(R+tCfb)+ g)));

		//float y = ((sample - 2*(s1*(R+tCfb)) - g2*s1  - s2)/(1+ g1*(2*(R+tCfb)+ g2)));

		return y;
	}
	inline float Apply(float sample,float g)
   {
			
      float gpw = tanf(g *sampleRateInv * float(M_PI));
      g = gpw;
      //float v = ((sample- R * s1*2 - g2*s1 - s2)/(1+ R*g1*2 + g1*g2));
      float v = NR(sample,g);

      float y1 = v*g + s1;
      s1 = v*g + y1;

      float y2 = y1*g + s2;
      s2 = y1*g + y2;

      float mc;
      if(!bandPassSw)
         mc = (1-mm)*y2 + (mm)*v;
      else
      {

         mc = 2.f * ( mm < 0.5f ? 
                      ((0.5f - mm) * y2 + (mm) * y1):
                      ((1.f-mm) * y1 + (mm-0.5f) * v)
                      );
      }

      return mc;
   }
	inline float NR24(float sample,float g,float lpc)
	{
		float ml = 1.f / (1.f+g);
		float S = (lpc*(lpc*(lpc*s1 + s2) + s3) +s4)*ml;
		float G = lpc*lpc*lpc*lpc;
		float y = (sample - R24 * S) / (1.f + R24*G);
		return y;
	}
	inline float Apply4Pole(float sample,float g)
	{
			float g1 = (float)tan(g *sampleRateInv * M_PI);
			g = g1;


			
			float lpc = g / (1 + g);
			float y0 = NR24(sample,g,lpc);
			//first low pass in cascade
			double v = (y0 - s1) * lpc;
			double res = v + s1;
			s1 = float(res + v);
			//damping
			s1 =atanf(s1*rcor24)*rcor24Inv;

			float y1 = float(res);
			float y2 = tptpc(s2,y1,g);
			float y3 = tptpc(s3,y2,g);
			float y4 = tptpc(s4,y3,g);
			float mc;
			switch(mmch)
			{
			case 0:
				mc = ((1 - mmt) * y4 + (mmt) * y3);
				break;
			case 1:
				mc = ((1 - mmt) * y3 + (mmt) * y2);
				break;
			case 2:
				mc = ((1 - mmt) * y2 + (mmt) * y1);
				break;
			case 3:
				mc = y1;
				break;
			default:
				mc=0;
				break;
			}
			//half volume comp
			return mc * (1.f + R24 * 0.45f);
	}
};

} // namespace rack_plugin_bsp
