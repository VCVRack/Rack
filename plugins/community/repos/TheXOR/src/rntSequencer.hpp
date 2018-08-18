#pragma once

namespace rack_plugin_TheXOR {

struct Renato;
extern bool Access(Renato *pr, bool is_x, int p);

struct rntSequencer
{
public:
	void Reset()
	{
		curPos = 0;
		pp_rev = false;
	}

	int Position() { return curPos; }

	int Step(float clock, float count_mode, bool seek_mode, Renato *pr, bool is_x)
	{
		int clk = clockTrig.process(clock); // 1=rise, -1=fall
		if(clk == 1)
		{
			int attempts = 0;
			do
			{
				switch((int)std::roundf(count_mode))
				{
				case 0: // fwd:
					if(++curPos > 3)
						curPos = 0;
					break;
				case 1: // bwd
					if(--curPos < 0)
						curPos = 3;
					break;

				case 2: //pend
					if(pp_rev)
					{
						if(--curPos < 0)
						{
							curPos = 1;
							pp_rev = !pp_rev;
						}
					} else
					{
						if(++curPos > 3)
						{
							curPos = 2;
							pp_rev = !pp_rev;
						}
					}
					break;
				}
				attempts++;
			} while(!Access(pr, is_x, curPos) && seek_mode && attempts < 4);
		}

		return clk;
	}

	bool Gate(int clk, Output *output, Light *led)
	{
		if(clk == 1)  // rise
		{
			led->value = output->value = LVL_ON;
		} else if(clk == -1) // fall
		{
			led->value = output->value = LVL_OFF;
		}

		return clk == 1;
	}

private:
	SchmittTrigger2 clockTrig;
	int curPos;
	bool pp_rev;
};

} // namespace rack_plugin_TheXOR
