#ifndef TROWASOFT_MODULE_TSTEMPOBPM_HPP
#define TROWASOFT_MODULE_TSTEMPOBPM_HPP

#define TROWA_TEMP_BPM_NUM_OPTIONS		4

struct TempoDivisor {
	const char* label;
	float multiplier;
	
	TempoDivisor() 
	{
		return;
	}
	TempoDivisor(const char* label, float mult)
	{
		this->label = label;
		this->multiplier = mult;
	}
};

extern const TempoDivisor* BPMOptions[TROWA_TEMP_BPM_NUM_OPTIONS];

#endif // end if not defined