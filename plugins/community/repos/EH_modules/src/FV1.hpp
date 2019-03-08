/* FV1 Emulator
 * Copyright (C)2018 - Eduard Heidt
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//http://www.spinsemi.com/knowledge_base/arch.html
//http://www.spinsemi.com/Products/appnotes/spn1001/AN-0001.pdf
//http://www.spinsemi.com/Products/datasheets/spn1001-dev/SPINAsmUserManual.pdf
//http://www.spinsemi.com/programs.php
//http://www.spinsemi.com/knowledge_base/cheat.html
//http://www.spinsemi.com/knowledge_base/inst_syntax.html
//http://www.spinsemi.com/forum/viewtopic.php?f=3&t=110&p=2227&hilit=CHO#p2227

#include <assert.h>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>

namespace rack_plugin_EH_modules {

const int OP_AND = 0x0e;
const int OP_OR = 0x0f;
const int OP_XOR = 0x10;
const int OP_LOG = 0x0b;
const int OP_EXP = 0x0c;
const int OP_SOF = 0x0d;
const int OP_SKP = 0x11;
const int OP_RDAX = 0x04;
const int OP_WRAX = 0x06;
const int OP_MAXX = 0x09;
const int OP_MULX = 0x0a;
const int OP_RDFX = 0x05;
const int OP_WRLX = 0x08;
const int OP_WRHX = 0x07;
const int OP_RDA = 0x00;
const int OP_RMPA = 0x01;
const int OP_WRA = 0x02;
const int OP_WRAP = 0x03;
const int OP_WLDS = 0x12;
const int OP_WLDR = 0x40000013;
const int OP_JAM = 0x00000413;
const int OP_CHO_RDA = 0x00000014;
const int OP_CHO_SOF = 0x80000014;
const int OP_CHO_RDAL = 0xc0000014;
//const int OP_CLR = 0x0e;
//const int OP_NOT = 0xffffff10;
//const int OP_ABSA = 0x09;
//const int OP_LDAX = 0x05;

const int SIN0_RATE = 0x00;
const int SIN0_RANGE = 0x01;
const int SIN1_RATE = 0x02;
const int SIN1_RANGE = 0x03;
const int RMP0_RATE = 0x04;
const int RMP0_RANGE = 0x05;
const int RMP1_RATE = 0x06;
const int RMP1_RANGE = 0x07;
const int POT0 = 0x10;
const int POT1 = 0x11;
const int POT2 = 0x12;
const int ADCL = 0x14;
const int ADCR = 0x15;
const int DACL = 0x16;
const int DACR = 0x17;
const int ADDR_PTR = 0x18;
const int REG0 = 0x20;
const int REG31 = 0x3f;
const int MAX_DELAY_ADDR = 32767;

const int SKP_NEG = 0x01;
const int SKP_GEZ = 0x02;
const int SKP_ZRO = 0x04;
const int SKP_ZRC = 0x08;
const int SKP_RUN = 0x10;

const int CHO_LFO_SIN0 = 0;
const int CHO_LFO_SIN1 = 1;
const int CHO_LFO_RMP0 = 2;
const int CHO_LFO_RMP1 = 3;
const int CHO_LFO_COS0 = 4;
const int CHO_LFO_COS1 = 5;
const int CHO_SIN = 0x00;
const int CHO_COS = 0x01;
const int CHO_REG = 0x02;
const int CHO_COMPC = 0x04;
const int CHO_COMPA = 0x08;
const int CHO_RPTR2 = 0x10;
const int CHO_NA = 0x20;

template <typename T>
inline T clamp(T value, T min, T max)
{
	return value > max ? max : value < min ? min : value;
}

class FixedPoint
{
  private:
	int value; // : 24;
  public:
	static const int F = 1 << 23;
	static const int S = 2;
	static const int MAX = (F - 1);

	FixedPoint()
	{
		set(0);
	}

	FixedPoint(int value)
	{
		set(value);
	}

	FixedPoint(const FixedPoint &reg)
	{
		set(reg.getValue());
	}

	static FixedPoint fromFloat(float value)
	{
		return FixedPoint(value * F);
	}

	float toFloat() const
	{
		return (float)this->value / F;
	}

	int getValue() const
	{
		return value;
	}

	void set(int newValue)
	{
		value = clamp(newValue, -F, F - 1);
	}

	static int fixSign(int newValue)
	{
		if (newValue & ~(F - 1))
			newValue |= ~(F - 1);

		return clamp(newValue, -F, F - 1);
	}

	static FixedPoint mul(FixedPoint a, FixedPoint b, int scale = 1)
	{
		return FixedPoint(((long long)a.getValue() * (long long)b.getValue() * scale) / FixedPoint::F);
	}
};

std::ostream &operator<<(std::ostream &os, const FixedPoint &m)
{
	return os << std::hex << "(" << std::setfill('0') << std::setw(8) << m.getValue() << "|" << m.toFloat() << ")";
}

class Reg : public FixedPoint
{
  public:
	Reg() : FixedPoint() {}
	Reg(const FixedPoint &value) : FixedPoint(value) {}

	void clear()
	{
		set(0);
	}

	void add(FixedPoint b)
	{
		set(getValue() + b.getValue());
	}

	void sub(FixedPoint b)
	{
		set(getValue() - b.getValue());
	}

	void mul(FixedPoint b)
	{
		set(FixedPoint::mul(*this, b).getValue());
	}

	void mul2(FixedPoint b)
	{
		set(FixedPoint::mul(*this, b, 2).getValue());
	}

	void abs()
	{
		set(std::abs(getValue()));
	}

	void not_()
	{
		set(FixedPoint::fixSign(~getValue()));
	}

	void and_(int mask)
	{
		set(FixedPoint::fixSign(getValue() & mask));
	}

	void or_(int mask)

	{
		set(FixedPoint::fixSign(getValue() | mask));
	}

	void xor_(int mask)
	{
		set(FixedPoint::fixSign(getValue() ^ mask));
	}
};

class SinLFO
{
	Reg sin_ = FixedPoint(0);
	Reg cos_ = FixedPoint(-FixedPoint::MAX);

	const Reg *regRange;
	const Reg *regRate;

  public:
	void init(const Reg &rate, const Reg &range)
	{
		regRange = &range;
		regRate = &rate;
	}

	void increment()
	{
		FixedPoint coeff(regRate->getValue() >> 8);
		cos_.add(FixedPoint::mul(sin_, coeff));
		sin_.sub(FixedPoint::mul(cos_, coeff));
	}

	void jam()
	{
		sin_.set(0);
		cos_.set(-FixedPoint::MAX);
	}

	int getValue(bool cosVal = false)
	{
		return FixedPoint::mul(cosVal ? cos_ : sin_, *regRange).getValue();
	}
};

class RampLFO
{
	const int AMP_4096 = 0x3fffff;
	//	const int AMP_2048 = 0x1fffff;
	//	const int AMP_1024 = 0x0fffff;
	//	const int AMP_512 =  0x07ffff;

	int pos = 0;

	const Reg *regRange;
	const Reg *regRate;

  public:
	static const int AMP_OFFSET = 21;

	void init(const Reg &rate, const Reg &range)
	{
		regRange = &range;
		regRate = &rate;
	}

	void increment()
	{
		int freq = regRate->getValue() >> 12;
		pos = (pos - freq) & this->getRange();
	}

	void jam()
	{
		pos = 0;
	}

	int getValue(bool ptr2value = false)
	{
		int range = this->getRange();

		if (ptr2value)
			return (pos + (range / 2)) & range;
		else
			return pos;
	}

	int getRange()
	{
		return AMP_4096 >> (regRange->getValue() >> (AMP_OFFSET));
	}

	int getXFade()
	{
		int range = this->getRange();

		int halfAmp = range >> 1;
		int xfade = pos > halfAmp ? range - pos : pos;

		return xfade << (regRange->getValue() >> AMP_OFFSET) << 0; // push Amplitude to MAX ?
	}
};

class DelayMemory
{
	int mem[MAX_DELAY_ADDR + 1] = {};
	unsigned int p = 0;

  public:
	int get(int offset, Reg &lr)
	{
		int val = mem[(offset + p) & MAX_DELAY_ADDR];
		lr.set(val);
		return val;
	}

	void set(int offset, int value)
	{
		mem[(offset + p) & MAX_DELAY_ADDR] = value;
	}

	void decrPtr()
	{
		--p;
	}
};

class FV1
{
  private:
	unsigned int pc = 0;
	Reg acc;
	Reg pacc;
	Reg lr;
	Reg regs[REG31 + 1] = {};
	DelayMemory delay;
	SinLFO sinLFO[2] = {};
	RampLFO rampLFO[2] = {};
	bool firstRun = true;
	std::vector<std::vector<int>> prog;

	int ACC()
	{
		return acc.getValue();
	}

	int PACC()
	{
		return pacc.getValue();
	}

	const FixedPoint &REG(int reg)
	{
		return regs[reg];
	}

	void REG_SET(int reg, int value)
	{
		regs[reg].set(value);
	}

	void SOF(FixedPoint scale, FixedPoint offset)
	{
		this->acc.mul2(scale);
		this->acc.add(offset);
	}

	void AND(int mask)
	{
		this->acc.and_(mask);
	}

	void OR(int mask)
	{
		this->acc.or_(mask);
	}

	void XOR(int mask)
	{
		this->acc.xor_(mask);
	}

	void LOG(FixedPoint scale, FixedPoint offset)
	{
		Reg tmp(this->acc);
		tmp.abs();

		if (tmp.getValue() == 0)
			tmp.set(1);

		float f = (log(tmp.toFloat()) / log(2.0f)) / 16.0f;
		this->acc.set(FixedPoint::fromFloat(f).getValue());
		this->acc.mul2(scale);
		this->acc.add(offset);
	}

	void EXP(FixedPoint scale, FixedPoint offset)
	{
		if (this->ACC() >= 0.0)
		{
			this->acc.set(FixedPoint::MAX);
			this->acc.mul2(scale);
			this->acc.add(offset);
		}
		else
		{
			float val = this->acc.toFloat() * 16.0f;
			val = pow(2.0f, val);

			this->acc.set(FixedPoint::fromFloat(val).getValue());
			this->acc.mul2(scale);
			this->acc.add(offset);
		}
	}

	void SKP(int flags, int nskip)
	{
		bool skip = false;

		if (flags & SKP_RUN)
			skip = firstRun == false;
		else if (flags & SKP_ZRO)
			skip = this->ACC() == 0;
		else if (flags & SKP_GEZ)
			skip = this->ACC() > 0;
		else if (flags & SKP_NEG)
			skip = this->ACC() < 0;
		else if (flags & SKP_ZRC)
			skip = (this->ACC() & FixedPoint::F) != (this->PACC() & FixedPoint::F);

		if (skip)
			pc += nskip;
	}

	void RDAX(int addr, FixedPoint scale)
	{
		Reg reg(this->REG(addr));
		reg.mul2(scale);
		this->acc.add(reg);
	}

	void WRAX(int addr, FixedPoint scale)
	{
		this->REG_SET(addr, this->ACC());
		this->acc.mul2(scale);
	}

	void MAXX(int addr, FixedPoint scale)
	{
		Reg temp(this->REG(addr));
		temp.abs();
		temp.mul2(scale);

		auto a = std::abs(this->ACC());
		auto b = std::abs(temp.getValue());
		this->acc.set(a > b ? a : b);
	}

	void MULX(int addr)
	{
		auto m = this->REG(addr);
		this->acc.mul(m);
	}

	void RDFX(int addr, FixedPoint scale)
	{
		this->acc.sub(this->REG(addr));
		this->acc.mul2(scale);
		this->acc.add(this->REG(addr));
	}

	void WRLX(int addr, FixedPoint scale)
	{
		this->REG_SET(addr, this->ACC());
		Reg tmp(this->pacc);
		tmp.sub(this->acc);
		tmp.mul2(scale);
		tmp.add(this->acc);
		this->acc.set(tmp.getValue());
	}

	void WRHX(int addr, FixedPoint scale)
	{
		this->REG_SET(addr, this->ACC());
		this->acc.mul2(scale);
		this->acc.add(this->pacc);
	}

	void RDA(int addr, FixedPoint scale)
	{
		Reg tmp;
		tmp.set(this->delay.get(addr, this->lr));
		tmp.mul2(scale);
		this->acc.add(tmp);
	}

	void RMPA(FixedPoint scale)
	{
		Reg tmp;
		tmp.set(this->delay.get(this->REG(ADDR_PTR).getValue() >> 8, this->lr));
		tmp.mul2(scale);
		this->acc.add(tmp);
	}

	void WRA(int addr, FixedPoint scale)
	{
		this->delay.set(addr, this->ACC());
		this->acc.mul2(scale);
	}

	void WRAP(int addr, FixedPoint scale)
	{
		this->delay.set(addr, this->ACC());
		this->acc.mul2(scale);
		this->acc.add(this->lr);
	}

	void WLDS(int lfo, int freq, int amp)
	{
		if (lfo == 0)
		{
			this->REG_SET(SIN0_RATE, freq << 14);
			this->REG_SET(SIN0_RANGE, amp << 8);
			this->sinLFO[0].jam();
		}
		else
		{
			this->REG_SET(SIN1_RATE, freq << 14);
			this->REG_SET(SIN1_RANGE, amp << 8);
			this->sinLFO[1].jam();
		}
	}

	void WLDR(int lfo, int freq, int amp)
	{
		if (lfo == 0)
		{
			this->REG_SET(RMP0_RATE, freq << 8);
			this->REG_SET(RMP0_RANGE, amp);
			this->rampLFO[0].jam();
		}
		else
		{
			this->REG_SET(RMP1_RATE, freq << 8);
			this->REG_SET(RMP1_RANGE, amp);
			this->rampLFO[1].jam();
		}
	}

	void JAM(int lfo)
	{
		this->rampLFO[lfo].jam();
	}

	void CHO(int lfo, int flags, std::function<void(int, FixedPoint)> subOP)
	{
		int lfoval = 0;
		FixedPoint scale;
		auto ONE = FixedPoint::MAX / 2;

		if (lfo == CHO_LFO_SIN0 || lfo == CHO_LFO_SIN1)
		{
			lfoval = this->sinLFO[lfo].getValue(flags & CHO_COS);

			scale.set(flags & CHO_COMPC ? ONE - lfoval : lfoval);

			if (flags & CHO_COMPA)
				lfoval = -lfoval;
		}
		else if (lfo == CHO_LFO_RMP0 || lfo == CHO_LFO_RMP1)
		{
			auto range = this->rampLFO[lfo - CHO_LFO_RMP0].getRange();
			lfoval = this->rampLFO[lfo - CHO_LFO_RMP0].getValue(flags & CHO_RPTR2);

			if (flags & CHO_COMPA)
				lfoval = range - lfoval;

			if (flags & CHO_NA)
				lfoval = this->rampLFO[lfo - CHO_LFO_RMP0].getXFade();

			scale.set(flags & CHO_COMPC ? ONE - lfoval : lfoval);
		}

		//scale.set(scale.getValue() / 2);

		subOP(lfoval, scale);
	}

	void CHO_RDA(int lfo, int flags, int addr)
	{
		CHO(lfo, flags, [&](int lfoval, FixedPoint scale) {
			if ((flags & CHO_NA) == 0)
				addr += (lfoval >> 10);

			RDA(addr, scale);
		});
	}

	void CHO_SOF(int lfo, int flags, FixedPoint offset)
	{
		CHO(lfo, flags, [&](int lfoval, FixedPoint scale) {
			SOF(scale, offset);
		});
	}

	void CHO_RDAL(int lfo)
	{
		int lfoval = 0;

		if (lfo == CHO_LFO_SIN0)
			lfoval = this->sinLFO[0].getValue();
		else if (lfo == CHO_LFO_SIN1)
			lfoval = this->sinLFO[1].getValue();
		else if (lfo == CHO_LFO_RMP0)
			lfoval = this->rampLFO[0].getValue();
		else if (lfo == CHO_LFO_RMP1)
			lfoval = this->rampLFO[1].getValue();
		else if (lfo == CHO_LFO_COS0)
			lfoval = this->sinLFO[0].getValue(true);
		else if (lfo == CHO_LFO_COS1)
			lfoval = this->sinLFO[1].getValue(true);
		else
		{
			assert(!"invalid lfo");
		}

		this->acc.set(lfoval);
	}

	void CLR()
	{
		this->acc.clear();
	}

	void NOT()
	{
		this->acc.not_();
	}

	void ABSA()
	{
		this->acc.abs();
	}

	void LDAX(int addr)
	{
		this->acc.set(this->REG(addr).getValue());
	}

  public:
	FV1()
	{
		this->sinLFO[0].init(this->regs[SIN0_RATE], this->regs[SIN0_RANGE]);
		this->sinLFO[1].init(this->regs[SIN1_RATE], this->regs[SIN1_RANGE]);
		this->rampLFO[0].init(this->regs[RMP0_RATE], this->regs[RMP0_RANGE]);
		this->rampLFO[1].init(this->regs[RMP1_RATE], this->regs[RMP1_RANGE]);
	}

	void dump(std::string &str, const std::string &endl = "")
	{
		std::ostringstream out;

		out << "PC: " << this->pc << endl
			<< " ACC: " << std::hex << this->acc.getValue() << endl
			<< " POT0: " << this->REG(POT0) << endl
			<< " POT1: " << this->REG(POT1) << endl
			<< " POT2: " << this->REG(POT2) << endl
			<< " RMP0_RATE: " << this->REG(RMP0_RATE) << endl
			<< " RMP0_RANGE: " << this->REG(RMP0_RANGE) << endl
			<< " RMP0_AMP: " << FixedPoint(this->rampLFO[0].getRange()) << endl
			<< " RMP0_VALUE: " << FixedPoint(this->rampLFO[0].getValue()) << endl
			<< " RMP0_XFADE " << FixedPoint(this->rampLFO[0].getXFade()) << endl
			<< " RMP1_RATE: " << FixedPoint(this->REG(RMP1_RATE)) << endl
			<< " RMP1_RANGE: " << FixedPoint(this->REG(RMP1_RANGE)) << endl
			<< " RMP1_VALUE: " << FixedPoint(this->rampLFO[1].getValue()) << endl
			<< " RMP1_XFADE " << FixedPoint(this->rampLFO[1].getXFade()) << endl
			<< " SIN0_RATE: " << this->REG(SIN0_RATE) << endl
			<< " SIN0_RANGE: " << this->REG(SIN0_RANGE) << endl
			<< " SIN0_SIN: " << FixedPoint(this->sinLFO[0].getValue()) << endl
			<< " SIN1_RATE: " << this->REG(SIN1_RATE) << endl
			<< " SIN1_RANGE: " << this->REG(SIN1_RANGE) << endl
			<< " SIN1_SIN: " << FixedPoint(this->sinLFO[1].getValue()) << endl
			<< " DACL: " << this->REG(DACL) << endl
			<< " DACR: " << this->REG(DACR) << endl;

		for (int i = REG0; i < REG31; i++)
			out << "REG" << i << ": " << this->regs[i] << endl;

		out << endl;

		str += out.str();
	}

	void execute(float inL, float inR, float pot0, float pot1, float pot2, float &outL, float &outR)
	{
		this->pc = 0;

		this->REG_SET(ADCL, FixedPoint::fromFloat(inL).getValue());
		this->REG_SET(ADCR, FixedPoint::fromFloat(inR).getValue());
		this->REG_SET(POT0, FixedPoint::fromFloat(pot0).getValue());
		this->REG_SET(POT1, FixedPoint::fromFloat(pot1).getValue());
		this->REG_SET(POT2, FixedPoint::fromFloat(pot2).getValue());

		while (this->pc < (unsigned int)prog.size())
		{
			const auto &op = prog[this->pc++];

			switch (op[0])
			{
			case OP_SKP:
				SKP(op[1], op[2]);
				break;
				//			case OP_CLR:
				//				CLR();
				//				break;
				//			case OP_NOT:
				//				NOT();
				//				break;
				//			case OP_ABSA:
				//				ABSA();
				//				break;
			case OP_SOF:
				SOF(op[1], op[2]);
				break;
			case OP_AND:
				AND(op[1]);
				break;
			case OP_OR:
				OR(op[1]);
				break;
			case OP_XOR:
				XOR(op[1]);
				break;
			case OP_LOG:
				LOG(op[1], op[2]);
				break;
			case OP_EXP:
				EXP(op[1], op[2]);
				break;
			case OP_WRAX:
				WRAX(op[1], op[2]);
				break;
			case OP_MAXX:
				MAXX(op[1], op[2]);
				break;
			case OP_RDAX:
				RDAX(op[1], op[2]);
				break;
				//			case OP_LDAX:
				//				LDAX(op[1]);
				//				break;
			case OP_RDFX:
				RDFX(op[1], op[2]);
				break;
			case OP_WRLX:
				WRLX(op[1], op[2]);
				break;
			case OP_WRHX:
				WRHX(op[1], op[2]);
				break;
			case OP_MULX:
				MULX(op[1]);
				break;
			case OP_WLDS:
				WLDS(op[1], op[2], op[3]);
				break;
			case OP_WLDR:
				WLDR(op[1], op[2], op[3]);
				break;
			case OP_JAM:
				JAM(op[1]);
				break;
			case OP_RDA:
				RDA(op[1], op[2]);
				break;
			case OP_RMPA:
				RMPA(op[1]);
				break;
			case OP_WRA:
				WRA(op[1], op[2]);
				break;
			case OP_WRAP:
				WRAP(op[1], op[2]);
				break;
			case OP_CHO_RDAL:
				CHO_RDAL(op[1]);
				break;
			case OP_CHO_SOF:
				CHO_SOF(op[1], op[2], op[3]);
				break;
			case OP_CHO_RDA:
				CHO_RDA(op[1], op[2], op[3]);
				break;
			default:
				std::cerr << std::hex << op[0] << std::endl;
				assert(!"Unknown OP!");
			}

#ifdef TEST
			std::string tmp;
			dump(tmp);
			std::cout << tmp << std::endl;
#endif
		}

		firstRun = false;

		this->delay.decrPtr();

		pacc.set(acc.getValue());
		this->acc.clear();

		this->sinLFO[0].increment();
		this->sinLFO[1].increment();
		this->rampLFO[0].increment();
		this->rampLFO[1].increment();

		outL = this->REG(DACL).toFloat();
		outR = this->REG(DACR).toFloat();
	}

	void loadFx(const std::vector<std::vector<int>> &prog)
	{
		this->prog = prog;

		this->firstRun = true;
		this->acc.set(0);
		this->pacc.set(0);
		this->pc = 0;

		for (int i = REG0; i < REG31; i++)
			this->regs[i].set(0);
	}
};

} // namespace rack_plugin_EH_modules
