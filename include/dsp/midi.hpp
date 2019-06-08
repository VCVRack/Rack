#pragma once
#include <dsp/common.hpp>
#include <midi.hpp>


namespace rack {
namespace dsp {


/** Converts gates and CV to MIDI messages.
CHANNELS is the number of polyphony channels. Use 1 for monophonic.
*/
template <int CHANNELS>
struct MidiGenerator {
	int8_t vels[CHANNELS];
	int8_t notes[CHANNELS];
	bool gates[CHANNELS];
	int8_t keyPressures[CHANNELS];
	int8_t channelPressure;
	int8_t ccs[128];
	int16_t pw;
	bool clk;
	bool start;
	bool stop;
	bool cont;

	MidiGenerator() {
		reset();
	}

	void reset() {
		for (int c = 0; c < CHANNELS; c++) {
			vels[c] = 100;
			notes[c] = 60;
			gates[c] = false;
			keyPressures[c] = -1;
		}
		channelPressure = -1;
		for (int i = 0; i < 128; i++) {
			ccs[i] = -1;
		}
		pw = 0x2000;
		clk = false;
		start = false;
		stop = false;
		cont = false;
	}

	void panic() {
		reset();
		// Send all note off commands
		for (int note = 0; note <= 127; note++) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(note);
			m.setValue(0);
			onMessage(m);
		}
	}

	/** Must be called before setNoteGate(). */
	void setVelocity(int8_t vel, int c) {
		vels[c] = vel;
	}

	void setNoteGate(int8_t note, bool gate, int c) {
		bool changedNote = gate && gates[c] && (note != notes[c]);
		bool enabledGate = gate && !gates[c];
		bool disabledGate = !gate && gates[c];
		if (changedNote || disabledGate) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(notes[c]);
			m.setValue(vels[c]);
			onMessage(m);
		}
		if (changedNote || enabledGate) {
			// Note on
			midi::Message m;
			m.setStatus(0x9);
			m.setNote(note);
			m.setValue(vels[c]);
			onMessage(m);
		}
		notes[c] = note;
		gates[c] = gate;
	}

	void setKeyPressure(int8_t val, int c) {
		if (keyPressures[c] == val)
			return;
		keyPressures[c] = val;
		// Polyphonic key pressure
		midi::Message m;
		m.setStatus(0xa);
		m.setNote(notes[c]);
		m.setValue(val);
		onMessage(m);
	}

	void setChannelPressure(int8_t val) {
		if (channelPressure == val)
			return;
		channelPressure = val;
		// Channel pressure
		midi::Message m;
		m.setSize(2);
		m.setStatus(0xd);
		m.setNote(val);
		onMessage(m);
	}

	void setCc(int8_t cc, int id) {
		if (ccs[id] == cc)
			return;
		ccs[id] = cc;
		// Continuous controller
		midi::Message m;
		m.setStatus(0xb);
		m.setNote(id);
		m.setValue(cc);
		onMessage(m);
	}

	void setModWheel(int8_t cc) {
		setCc(cc, 0x01);
	}

	void setVolume(int8_t cc) {
		setCc(cc, 0x07);
	}

	void setBalance(int8_t cc) {
		setCc(cc, 0x08);
	}

	void setPan(int8_t cc) {
		setCc(cc, 0x0a);
	}

	void setSustainPedal(int8_t cc) {
		setCc(cc, 0x40);
	}

	void setPitchWheel(int16_t pw) {
		if (this->pw == pw)
			return;
		this->pw = pw;
		// Pitch wheel
		midi::Message m;
		m.setStatus(0xe);
		m.setNote(pw & 0x7f);
		m.setValue((pw >> 7) & 0x7f);
		onMessage(m);
	}

	void setClock(bool clk) {
		if (this->clk == clk)
			return;
		this->clk = clk;
		if (clk) {
			// Timing clock
			midi::Message m;
			m.setSize(1);
			m.setStatus(0xf);
			m.setChannel(0x8);
			onMessage(m);
		}
	}

	void setStart(bool start) {
		if (this->start == start)
			return;
		this->start = start;
		if (start) {
			// Start
			midi::Message m;
			m.setSize(1);
			m.setStatus(0xf);
			m.setChannel(0xa);
			onMessage(m);
		}
	}

	void setContinue(bool cont) {
		if (this->cont == cont)
			return;
		this->cont = cont;
		if (cont) {
			// Continue
			midi::Message m;
			m.setSize(1);
			m.setStatus(0xf);
			m.setChannel(0xb);
			onMessage(m);
		}
	}

	void setStop(bool stop) {
		if (this->stop == stop)
			return;
		this->stop = stop;
		if (stop) {
			// Stop
			midi::Message m;
			m.setSize(1);
			m.setStatus(0xf);
			m.setChannel(0xc);
			onMessage(m);
		}
	}

	virtual void onMessage(midi::Message message) {}
};


} // namespace dsp
} // namespace rack
