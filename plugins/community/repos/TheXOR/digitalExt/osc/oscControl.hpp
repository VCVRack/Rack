#pragma once
#ifdef OSC_ENABLE

struct OSCDriver;
struct oscControl
{
public:
	virtual ~oscControl() {};
	void Draw(OSCDriver *drv, bool force = false);

	bool isDirty() { return m_dirty; }
	bool Intersect(std::string address) 
	{ 
		return m_address.compare(address) == 0; 
	}
	void ChangeFromGUI(OSCDriver *drv);  // gui updated: the new value is already in the binded parameter
	void onOscMsg(Module *pModule, OSCMsg msg) 
	{
		setValue(pModule, msg.value); 		
	}
	bool DetectGUIChanges() { return getValue() != m_lastDrawnValue; }

	int ID() { return is_light ? pBindedLight->firstLightId : pBindedParam->paramId; }
	void bindWidget(ModuleLightWidget *p) { pBindedLight = p; is_light = true; }
	void bindWidget(ParamWidget *p) { pBindedParam = p; }

	oscControl(std::string address)
	{
		m_address = address;
		is_light = false;
		pBindedLight = NULL;
		pBindedParam = NULL;
		m_dirty = true;
		m_lastDrawnValue = -10202020;
	}

private:
	float getValue() { return is_light ? pBindedLight->module->lights[pBindedLight->firstLightId].getBrightness() : pBindedParam->value; }	
	void setValue(Module *pModule, float v)
	{		
		if(is_light)
			pBindedLight->module->lights[pBindedLight->firstLightId].value = v;
		else
		{
			v = rescale(v, 0.0, 1.0, pBindedParam->minValue, pBindedParam->maxValue);

			SVGKnob *pk = (SVGKnob *)dynamic_cast<SVGKnob *>(pBindedParam);
			if(pk != NULL)
			{
				pModule->params[pBindedParam->paramId].value = pBindedParam->value = v;
				pk->dirty = true;				
			} else
			{
				SVGFader *pk1 = (SVGFader *)dynamic_cast<SVGFader *>(pBindedParam);
				if(pk1 != NULL)
				{
					pModule->params[pBindedParam->paramId].value = pBindedParam->value = v;
					pk1->dirty = true;
				} else
					pBindedParam->setValue(v);
			}
		}
		m_lastDrawnValue = v;
		m_dirty = false;
	}
	std::string m_address;
	ModuleLightWidget *pBindedLight;
	ParamWidget *pBindedParam;
	bool m_dirty;
	float m_lastDrawnValue;
	bool is_light;
};

#endif // OSC
