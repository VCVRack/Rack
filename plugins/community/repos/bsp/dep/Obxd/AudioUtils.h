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

/* #include "SynthEngine.h" */

namespace rack_plugin_bsp {

const float sq2_12 = 1.0594630943592953f;

const float dc = 1e-18f;
const float ln2 = 0.69314718056f;
const float mult = ln2 / 12.0f;
inline static float getPitch(float index)
{
	//Lookup table is not that effective compared to SSE exp
	//SSE should be on

	//const int mulres = 2;
    //const int lowerBound = -94;
    //const int upperBound = 94;
	//const int lutlen = (upperBound-lowerBound)*2;
   return 440.0f * expf(mult * index);
	//static const float lut [lutlen]={1.929,1.986,2.044,2.104,2.165,2.229,2.294,2.361,2.431,2.502,2.575,2.651,2.728,2.808,2.891,2.975,3.062,3.152,3.245,3.340,3.437,3.538,3.642,3.749,3.858,3.972,4.088,4.208,4.331,4.458,4.589,4.723,4.861,5.004,5.150,5.301,5.457,5.617,5.781,5.951,6.125,6.304,6.489,6.679,6.875,7.076,7.284,7.497,7.717,7.943,8.176,8.415,8.662,8.916,9.177,9.446,9.723,10.008,10.301,10.603,10.913,11.233,11.562,11.901,12.250,12.609,12.978,13.359,13.750,14.153,14.568,14.994,15.434,15.886,16.352,16.831,17.324,17.832,18.354,18.892,19.445,20.015,20.602,21.205,21.827,22.466,23.125,23.802,24.500,25.218,25.957,26.717,27.500,28.306,29.135,29.989,30.868,31.772,32.703,33.661,34.648,35.663,36.708,37.784,38.891,40.030,41.203,42.411,43.654,44.933,46.249,47.605,48.999,50.435,51.913,53.434,55.000,56.612,58.270,59.978,61.735,63.544,65.406,67.323,69.296,71.326,73.416,75.567,77.782,80.061,82.407,84.822,87.307,89.865,92.499,95.209,97.999,100.870,103.826,106.869,110.000,113.223,116.541,119.956,123.471,127.089,130.813,134.646,138.591,142.652,146.832,151.135,155.563,160.122,164.814,169.643,174.614,179.731,184.997,190.418,195.998,201.741,207.652,213.737,220.000,226.446,233.082,239.912,246.942,254.178,261.626,269.292,277.183,285.305,293.665,302.270,311.127,320.244,329.628,339.286,349.228,359.461,369.994,380.836,391.995,403.482,415.305,427.474,440.000,452.893,466.164,479.823,493.883,508.355,523.251,538.584,554.365,570.609,587.330,604.540,622.254,640.487,659.255,678.573,698.456,718.923,739.989,761.672,783.991,806.964,830.609,854.948,880.000,905.786,932.328,959.647,987.767,1016.710,1046.502,1077.167,1108.731,1141.219,1174.659,1209.079,1244.508,1280.975,1318.510,1357.146,1396.913,1437.846,1479.978,1523.344,1567.982,1613.927,1661.219,1709.896,1760.000,1811.572,1864.655,1919.294,1975.533,2033.421,2093.005,2154.334,2217.461,2282.438,2349.318,2418.158,2489.016,2561.950,2637.020,2714.291,2793.826,2875.691,2959.955,3046.689,3135.964,3227.854,3322.438,3419.792,3520.000,3623.144,3729.310,3838.587,3951.066,4066.842,4186.009,4308.668,4434.922,4564.875,4698.636,4836.317,4978.032,5123.899,5274.041,5428.582,5587.652,5751.382,5919.911,6093.377,6271.927,6455.709,6644.875,6839.585,7040.000,7246.288,7458.620,7677.174,7902.133,8133.683,8372.018,8617.337,8869.844,9129.751,9397.273,9672.634,9956.064,10247.798,10548.082,10857.164,11175.304,11502.765,11839.822,12186.755,12543.854,12911.417,13289.750,13679.170,14080.000,14492.576,14917.241,15354.349,15804.266,16267.366,16744.036,17234.674,17739.689,18259.501,18794.545,19345.268,19912.127,20495.597,21096.164,21714.329,22350.607,23005.530,23679.643,24373.510,25087.708,25822.834,26579.501,27358.340,28160.000,28985.151,29834.481,30708.698,31608.532,32534.732,33488.073,34469.348,35479.377,36519.002,37589.091,38690.535,39824.254,40991.194,42192.328,43428.657,44701.214,46011.060,47359.287,48747.020,50175.416,51645.668,53159.002,54716.680,56320.001,57970.303,59668.962,61417.396,63217.063,65069.465,66976.146,68938.697,70958.755,73038.005,75178.182,77381.071,79648.509,81982.388,84384.656,86857.315,89402.429,92022.120,94718.574,97494.040};
	//if(index > 92.0)
	//	return lut[lutlen-1];
	//if(index < -94.0)
	//	return lut[0];
	//index = (index - lowerBound)*mulres;
	//int mi = floor(index);
	//float ofs = index-mi;
	//
 // return lut[mi] + (lut[mi+1] - lut[mi]) * ofs;
};

inline static float tptlpupw(float & state , float inp , float cutoff , float srInv)
{
	cutoff = (cutoff * srInv)*float(M_PI);
	double v = (inp - state) * cutoff / (1 + cutoff);
	double res = v + state;
	state = float(res + v);
	return float(res);
}
inline static float tptlp(float& state,float inp,float cutoff,float srInv)
{
	cutoff = tan(cutoff * (srInv)* float(M_PI)) ;
	double v = (inp - state) * cutoff / (1 + cutoff);
	double res = v + state;
	state = float(res + v);
	return float(res);
};
inline static float tptpc(float& state,float inp,float cutoff)
{
	double v = (inp - state) * cutoff / (1 + cutoff);
	double res = v + state;
	state = float(res + v);
	return float(res);
}
inline static float linsc(float param,const float min,const float max)
{
	 return (param) * (max - min) + min;
}
inline static float logsc(float param, const float min,const float max,const float rolloff = 19.0f)
{
	return ((expf(param * logf(rolloff+1)) - 1.0f) / (rolloff)) * (max-min) + min;
}

} // namespace rack_plugin_bsp
