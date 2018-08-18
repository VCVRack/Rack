#pragma once
#ifdef OSC_ENABLE
#include "oscCommunicator.hpp"
#include "oscControl.hpp"

class OSCDriver
{
public:
	OSCDriver(Module *pmod, int scene)
	{
		m_scene = scene;
		pModule = pmod;
		comm = new oscCommunicator(m_scene);
		lastCheck = 0;
		initConnection(true);
	}

	~OSCDriver()
	{
		initConnection(false);

		for (std::map<int, oscControl *>::iterator it = m_bindings.begin(); it != m_bindings.end(); ++it)
		{
			if (it->second != NULL)
				delete it->second;
		}

		m_bindings.clear();
		delete comm;
	}

	bool Connected() { return comm->Connected(); }
	void Reset(int lp) { comm->Clear(); }

	void ProcessOSC()
	{
		if(Connected())
		{
			processGUI();
			processOSCMsg();
		} else if((GetTickCount() - lastCheck) >= 2000)
		{
			initConnection(true);
		}
	}

	void sendMsg(const char *address, float value)
	{
		OSCMsg msg;
		msg.set(m_scene, address, value);
		comm->Write(&msg);
	}

	void Add(oscControl *ctrl, ParamWidget *p)
	{
		ctrl->bindWidget(p);
		int id = ctrl->ID();
		m_bindings[id] = ctrl;
	}

	void Add(oscControl *ctrl, ModuleLightWidget *p)
	{
		ctrl->bindWidget(p);
		int id = ctrl->ID();
		m_bindings[0x8000 | id] = ctrl;
	}

private:
	oscCommunicator *comm;
	Module *pModule;
	int m_scene;
	uint32_t lastCheck;
	std::map<int, oscControl *>m_bindings;

	void initConnection(bool registr)
	{
		lastCheck = GetTickCount();
		comm->Open();
	}

	void redrawCache()
	{
		for (std::map<int, oscControl *>::iterator it = m_bindings.begin(); it != m_bindings.end(); ++it)
		{
			it->second->Draw(this, true);
		}
	}

	void processGUI()
	{
		for (std::map<int, oscControl *>::iterator it = m_bindings.begin(); it != m_bindings.end(); ++it)
		{
			if (it->second->DetectGUIChanges())
			{
				it->second->ChangeFromGUI(this);
			}
		}
	}

	void processOSCMsg()
	{
		OSCMsg msg;

		while (comm->Read(&msg))
		{
			for (std::map<int, oscControl *>::iterator it = m_bindings.begin(); it != m_bindings.end(); ++it)
			{
				if(msg.scene == m_scene && it->second->Intersect(msg.address))
				{
#ifdef DEBUG
					info("MSG: scene=%i, address= %s", msg.scene, msg.address);
#endif
					it->second->onOscMsg(pModule, msg);
					break;
				}
			}
		}
	}
};

#endif //OSC