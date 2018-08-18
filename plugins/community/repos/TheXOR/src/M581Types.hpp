
namespace rack_plugin_TheXOR {

struct M581;
struct ParamGetter
{
public:
	ParamGetter() { pModule = NULL; }
	void Set(M581 *module) { pModule = module; }
	bool IsEnabled(int numstep);
	bool IsSlide(int numstep);
	int GateMode(int numstep);
	int PulseCount(int numstep);
	float Note(int numstep);

	// Generic
	int RunMode();
	int NumSteps();
	float SlideTime();
	float GateTime();
	int StepDivision();

private:
	M581 * pModule;
};



struct CV_LINE
{
	void Reset()
	{
		slideNoteIncrement = 0;
		startNoteValue = -1.0;
	}

	void Set(ParamGetter *get) { pGet = get; }

	void Begin(int cur_step)
	{
		curNote = pGet->Note(cur_step);
		if(startNoteValue < 0.0)
			startNoteValue = curNote;

		if(pGet->IsSlide(cur_step))
			slideNoteIncrement = (curNote - startNoteValue) / pGet->SlideTime();
		else
		{
			startNoteValue = curNote;
			slideNoteIncrement = 0;
		}
	}

	float Play(float elapsed)
	{
		float cv = startNoteValue + slideNoteIncrement * elapsed;
		if(slideNoteIncrement < 0 && curNote > cv)		// inc negativo ---> cv potrebbe essere INFERIORE a target
		{
			slideNoteIncrement = 0;
			cv = startNoteValue = curNote;		// cv non puo' essere INFERIORE a curstep.value
		} else if(slideNoteIncrement > 0 && curNote < cv) // inc positivo --> cv potrebbe sovraelongare
		{
			slideNoteIncrement = 0;
			cv = startNoteValue = curNote;		// cv non puo' essere SUPERIORE a curstep.value
		}
		return cv;
	}

private:
	float curNote;
	float startNoteValue;
	float slideNoteIncrement;
	ParamGetter *pGet;
};

struct GATE_LINE
{
private:
	ParamGetter * pGet;
	int curStep;
	float gate_len(float elapsedTime) { return elapsedTime > pGet->GateTime() ? LVL_OFF : LVL_ON; }

public:
	void Reset()
	{
		curStep = 0;
	}

	void Set(ParamGetter *get) { pGet = get; }

	void Begin(int cur_step)
	{
		curStep = cur_step;
	}

	float Play(TIMER *timer, int pulseCount)
	{
		float rv = LVL_OFF;

		switch(pGet->GateMode(curStep))
		{
		case 0:	// off
			break;

		case 1:  //single pulse
			rv = gate_len(timer->Elapsed());
			break;

		case 2: // multiple pulse
		{
			if((pulseCount % pGet->StepDivision()) == 0)
			{
				rv = gate_len(timer->StopWatch());
			} else
			{
				rv = LVL_OFF;
			}
		}
		break;

		case 3:	// continuo
			rv = LVL_ON;
			break;
		}

		return rv;
	}
};

struct STEP_COUNTER
{
	void Reset(TIMER *timer)
	{
		pulseCounter = 0;
		curStep = 0;
		pp_rev = false;
		timer->Reset();
	}

	void Set(ParamGetter *get) { pGet = get; }
	int CurStep() { return curStep % 8; }
	int PulseCounter() { return pulseCounter; }

	bool Play(TIMER *timer, int *cur_step)
	{
		bool play = pulseCounter++ >= pGet->PulseCount(curStep);
		if(play)
		{
			pulseCounter = 0;
			curStep = get_next_step(curStep);
			timer->Begin();
		} else
			timer->RestartStopWatch();

		*cur_step = CurStep();
		return play;
	}

private:
	ParamGetter * pGet;
	int pulseCounter;
	bool pp_rev;
	int curStep;

	bool testaCroce() { return randomUniform() > 0.5; }
	int getRand(int rndMax) { return int(randomUniform() * rndMax); }

	int get_next_step(int current)
	{
		switch(pGet->RunMode())
		{
		case 0: // FWD
			return inc_step(current);

		case 1: // BWD
			return dec_step(current);

		case 2: // ping ed anche pong
			if(pp_rev)
			{
				int step = dec_step(current);
				if(step <= current)
					return step;
				pp_rev = !pp_rev;
				return inc_step(current);
			} else
			{
				int step = inc_step(current);
				if(step >= current)
					return step;
				pp_rev = !pp_rev;
				return dec_step(current);
			}
			break;

		case 3: // BROWNIAN
		{
			if(testaCroce())
			{
				return inc_step(current);
			} else
			{
				return testaCroce() ? dec_step(current) : current;
			}
		}
		break;

		case 4: // At casacc
			current = getRand(pGet->NumSteps()); // OCIO: step off NON funziona con random!
			break;
		}

		return current;
	}

	int inc_step(int step)
	{
		for(int k = 0; k < 8; k++)
		{
			if(++step >= pGet->NumSteps())
				step = 0;
			if(pGet->IsEnabled(step))  // step on?
				break;
		}

		return step;
	}

	int dec_step(int step)
	{
		for(int k = 0; k < 8; k++)
		{
			if(--step < 0)
				step = pGet->NumSteps() - 1;
			if(pGet->IsEnabled(step))  // step on?
				break;
		}

		return step;
	}

};

} // namespace rack_plugin_TheXOR
