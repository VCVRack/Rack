/*                                                                     *\
**       __   ___  ______                                              **
**      / /  / _ \/_  __/                                              **
**     / /__/ , _/ / /    Lindenberg                                   **
**    /____/_/|_| /_/  Research Tec.                                   **
**                                                                     **
**                                                                     **
**	  https://github.com/lindenbergresearch/LRTRack	                   **
**    heapdump@icloud.com                                              **
**		                                                               **
**    Sound Modules for VCV Rack                                       **
**    Copyright 2017/2018 by Patrick Lindenberg / LRT                  **
**                                                                     **
**    For Redistribution and use in source and binary forms,           **
**    with or without modification please see LICENSE.                 **
**                                                                     **
\*                                                                     */

#include <cstring>
#include <cmath>
#include "RateConverter.hpp"


namespace dsp {

RateConverter::RateConverter(void) {
    irBuffer = nullptr;

    inputBuffer = nullptr;

    ratio = 1;
    length = 0;

    // reset all indices
    delayPos = 0;
    irBufferPos = 0;
    inputPos = 0;
    osPos = 0;
}


RateConverter::~RateConverter(void) {
    delete[] inputBuffer;
    delete[] irBuffer;
}


void RateConverter::init(int _ratio, int _length, float *pIRBuffer) {
    length = _length;
    ratio = _ratio;

    delete[] irBuffer;

    irBuffer = new float[_length];

    memset(irBuffer, 0, _length * sizeof(float));

    // memcpy(pIRBuffer, irBuffer, _length);

    for (int i = 0; i < _length; i++) {
        irBuffer[i] = pIRBuffer[i];
    }

    // dynamically allocate the input x buffers and save the pointers
    delete[] inputBuffer;

    //  delete[] m_pRightInputBuffer;

    inputBuffer = new float[_length];
//    m_pRightInputBuffer = new float[_length];

    // flush x buffers
    memset(inputBuffer, 0, _length * sizeof(float));
    //memset(m_pRightInputBuffer, 0, _length * sizeof(float));

    // reset all indices
    delayPos = 0;
    irBufferPos = 0;
    inputPos = 0;
    osPos = 0;

}


void RateConverter::reset() {
    // flush x buffers
    if (inputBuffer)
        memset(inputBuffer, 0, length * sizeof(float));
    /* if (m_pRightInputBuffer)
         memset(m_pRightInputBuffer, 0, length * sizeof(float));*/

    // reset all indices
    delayPos = 0;
    irBufferPos = 0;
    inputPos = 0;
    osPos = 0;
}


Decimator::Decimator(void) {
}


Decimator::~Decimator(void) {
}


/* L-Point Decimator
	Input: Left and Right Input Buffers with L samples per buffer
	Output:	Left and Right Input Samples ynL, ynR

	This function loops L times, decimating the outputs and returning the LAST output calculated

	CALLER SUPPLIES INPUT BUFFERS!
*/
void Decimator::decimateSamples(float *buffer, float &out) {
    if (!buffer)
        return;

    // counter for decimator optimization
    osPos = -1;

    for (int i = 0; i < ratio; i++) {
        float tmp = 0;

        // decimate next sample
        //
        // out and ynR are valid (and returned) after the last call to decimateNextOutputSample()
        decimateNextOutputSample(buffer[i], tmp);

        if (i == 0) {
            out = tmp;
        }
    }
}


bool Decimator::decimateNextOutputSample(float x, float &out) {
    inputBuffer[inputPos] = x;

    osPos++;

    if (osPos != 0) {
        inputPos++;
        if (inputPos >= length)
            inputPos = 0;

        return true;
    }

    // reset: read index for Delay Line -> write index
    delayPos = inputPos;

    // reset: read index for IR - > top (0)
    irBufferPos = 0;

    // accumulator
    float yn_accumL = 0;

    // This can be optimized!! Don't have to convolve on the first L-1, only need one convolution at the end
    for (int i = 0; i < length; i++) {
        // do the sum of products
        yn_accumL += inputBuffer[delayPos] * irBuffer[irBufferPos];

        // advance the IR index
        irBufferPos++;

        // decrement the Delay Line index
        delayPos--;

        // check for wrap of delay line (no need to check IR out)
        if (delayPos < 0)
            delayPos = length - 1;
    }

    // write out
    out = yn_accumL;

    // incremnent the pointers and wrap if necessaryx
    inputPos++;
    if (inputPos >= length)
        inputPos = 0;

    return true;
}


Interpolator::Interpolator(void) {

}


Interpolator::~Interpolator(void) {

}


/**
 * @brief
 * @param x input
 * @param buffer buffer for oversampled values (4x = 4) etc.
 */
void Interpolator::interpolateSamples(float x, float *buffer) {
    if (!buffer)
        return;

    for (int i = 0; i < ratio; i++) {
        interpolateNextOutputSample(x, buffer[i]);
    }
}

/* interpolateNextOutputSample
	This does the work:
		- first time through the loop apply inputs xnL and xnR and convolve
		- other L-1 times, insert a zero into the delay and convolve
*/
// sepcial kind of convolution!
void Interpolator::interpolateNextOutputSample(float x, float &out) {
    // Read the Input
    // if current L == 0 read xn
    // else insert 0.0
    inputBuffer[inputPos] = osPos == 0 ? x : 0.0;

    // reset: read index for Delay Line -> write index
    delayPos = inputPos;

    // reset: read index for IR - > top (0)
    irBufferPos = 0;

    // accumulator
    float sum = 0;

    // convolve: OPTIMIZE THIS; can skip over delay elements with 0s in them, there are L-1 of them
    for (int i = 0; i < length; i++) {
        // do the sum of products
        sum += inputBuffer[delayPos] * irBuffer[irBufferPos];

        // advance the IR index
        irBufferPos++;

        // decrement the Delay Line index
        delayPos--;

        // check for wrap of delay line (no need to check IR buffer)
        if (delayPos < 0)
            delayPos = length - 1;
    }

    // out
    out = sum * (float) ratio;

    // incremnent the pointers and wrap if necessary
    inputPos++;
    if (inputPos >= length)
        inputPos = 0;

    osPos++;
    if (osPos > ratio - 1)
        osPos = 0;
}


NeoOversampler::NeoOversampler() {
    enabled = true;

    init();
}


/**
 * @brief Initialize oversampler and fir filter
 */
void NeoOversampler::init() {
    ratio = 4;
    irSize = 19;


    memset(&irBuffer, 0, 1024 * sizeof(float));

    /*irBuffer[0] = 0.0000005445158422;
    irBuffer[1] = 0.0000005707121318;
    irBuffer[2] = -0.0000002019931742;
    irBuffer[3] = -0.0000027811349810;
    irBuffer[4] = -0.0000079230067058;
    irBuffer[5] = -0.0000157045305968;
    irBuffer[6] = -0.0000250798548223;
    irBuffer[7] = -0.0000337998826581;
    irBuffer[8] = -0.0000389039269066;
    irBuffer[9] = -0.0000377439646400;
    irBuffer[10] = -0.0000292139411613;
    irBuffer[11] = -0.0000146474785652;
    irBuffer[12] = 0.0000021514131276;
    irBuffer[13] = 0.0000159797546075;
    irBuffer[14] = 0.0000221549125854;
    irBuffer[15] = 0.0000185557037184;
    irBuffer[16] = 0.0000068430767897;
    irBuffer[17] = -0.0000078581069829;
    irBuffer[18] = -0.0000189057973330;
    irBuffer[19] = -0.0000210210473597;
    irBuffer[20] = -0.0000128407873490;
    irBuffer[21] = 0.0000021356875095;
    irBuffer[22] = 0.0000168605420185;
    irBuffer[23] = 0.0000239175660681;
    irBuffer[24] = 0.0000191849812836;
    irBuffer[25] = 0.0000041909465835;
    irBuffer[26] = -0.0000141687769428;
    irBuffer[27] = -0.0000266804745479;
    irBuffer[28] = -0.0000263464771706;
    irBuffer[29] = -0.0000122119981825;
    irBuffer[30] = 0.0000095742261692;
    irBuffer[31] = 0.0000282986147795;
    irBuffer[32] = 0.0000338566060236;
    irBuffer[33] = 0.0000221132413571;
    irBuffer[34] = -0.0000023518437047;
    irBuffer[35] = -0.0000278249426628;
    irBuffer[36] = -0.0000409598324040;
    irBuffer[37] = -0.0000336766352120;
    irBuffer[38] = -0.0000079384271885;
    irBuffer[39] = 0.0000243380400207;
    irBuffer[40] = 0.0000466642231913;
    irBuffer[41] = 0.0000463036813017;
    irBuffer[42] = 0.0000213879866351;
    irBuffer[43] = -0.0000170829262061;
    irBuffer[44] = -0.0000499074158142;
    irBuffer[45] = -0.0000591524149058;
    irBuffer[46] = -0.0000378185868612;
    irBuffer[47] = 0.0000054448237279;
    irBuffer[48] = 0.0000495497297379;
    irBuffer[49] = 0.0000711093161954;
    irBuffer[50] = 0.0000567109709664;
    irBuffer[51] = 0.0000109462253022;
    irBuffer[52] = -0.0000444776051154;
    irBuffer[53] = -0.0000808542754385;
    irBuffer[54] = -0.0000772124767536;
    irBuffer[55] = -0.0000321749212162;
    irBuffer[56] = 0.0000336578195856;
    irBuffer[57] = 0.0000868840143085;
    irBuffer[58] = 0.0000980972254183;
    irBuffer[59] = 0.0000579376974201;
    irBuffer[60] = -0.0000162688975252;
    irBuffer[61] = -0.0000876213744050;
    irBuffer[62] = -0.0001178134771180;
    irBuffer[63] = -0.0000875171608641;
    irBuffer[64] = -0.0000082237074821;
    irBuffer[65] = 0.0000814914092189;
    irBuffer[66] = 0.0001345096825389;
    irBuffer[67] = 0.0001197328674607;
    irBuffer[68] = 0.0000399582386308;
    irBuffer[69] = -0.0000670430599712;
    irBuffer[70] = -0.0001461128558731;
    irBuffer[71] = -0.0001529414148536;
    irBuffer[72] = -0.0000786032978795;
    irBuffer[73] = 0.0000430524851254;
    irBuffer[74] = 0.0001504003303126;
    irBuffer[75] = 0.0001850285043474;
    irBuffer[76] = 0.0001232606882695;
    irBuffer[77] = -0.0000086719082901;
    irBuffer[78] = -0.0001451331336284;
    irBuffer[79] = -0.0002134631940862;
    irBuffer[80] = -0.0001724150206428;
    irBuffer[81] = -0.0000364416373486;
    irBuffer[82] = 0.0001281958102481;
    irBuffer[83] = 0.0002353741147090;
    irBuffer[84] = 0.0002239009336336;
    irBuffer[85] = 0.0000919931262615;
    irBuffer[86] = -0.0000977704112302;
    irBuffer[87] = -0.0002476811932866;
    irBuffer[88] = -0.0002749281702563;
    irBuffer[89] = -0.0001569569285493;
    irBuffer[90] = 0.0000524965180375;
    irBuffer[91] = 0.0002472368068993;
    irBuffer[92] = 0.0003221260849386;
    irBuffer[93] = 0.0002294945443282;
    irBuffer[94] = 0.0000083545846792;
    irBuffer[95] = -0.0002310073177796;
    irBuffer[96] = -0.0003616396570578;
    irBuffer[97] = -0.0003069138329010;
    irBuffer[98] = -0.0000847206174512;
    irBuffer[99] = 0.0001962697569979;
    irBuffer[100] = 0.0003892597160302;
    irBuffer[101] = 0.0003856586990878;
    irBuffer[102] = 0.0001755936391419;
    irBuffer[103] = -0.0001408394600730;
    irBuffer[104] = -0.0004006113158539;
    irBuffer[105] = -0.0004613618366420;
    irBuffer[106] = -0.0002789076534100;
    irBuffer[107] = 0.0000632931623841;
    irBuffer[108] = 0.0003913739928976;
    irBuffer[109] = 0.0005289468099363;
    irBuffer[110] = 0.0003914636326954;
    irBuffer[111] = 0.0000368107284885;
    irBuffer[112] = -0.0003575389855541;
    irBuffer[113] = -0.0005827903514728;
    irBuffer[114] = -0.0005089115002193;
    irBuffer[115] = -0.0001587389997439;
    irBuffer[116] = 0.0002956803364214;
    irBuffer[117] = 0.0006169307744130;
    irBuffer[118] = 0.0006257825298235;
    irBuffer[119] = 0.0003004157624673;
    irBuffer[120] = -0.0002032436459558;
    irBuffer[121] = -0.0006253321189433;
    irBuffer[122] = -0.0007355912821367;
    irBuffer[123] = -0.0004583036352415;
    irBuffer[124] = 0.0000788305187598;
    irBuffer[125] = 0.0006021905574016;
    irBuffer[126] = 0.0008310037665069;
    irBuffer[127] = 0.0006273413309827;
    irBuffer[128] = 0.0000775332082412;
    irBuffer[129] = -0.0005422755493782;
    irBuffer[130] = -0.0009040769655257;
    irBuffer[131] = -0.0008009540033527;
    irBuffer[132] = -0.0002641671453603;
    irBuffer[133] = 0.0004412865964696;
    irBuffer[134] = 0.0009465577895753;
    irBuffer[135] = 0.0009711356833577;
    irBuffer[136] = 0.0004775526758749;
    irBuffer[137] = -0.0002962144499179;
    irBuffer[138] = -0.0009502378525212;
    irBuffer[139] = -0.0011286106891930;
    irBuffer[140] = -0.0007122103124857;
    irBuffer[141] = 0.0001056927067111;
    irBuffer[142] = 0.0009073557448573;
    irBuffer[143] = 0.0012630778364837;
    irBuffer[144] = 0.0009606519597583;
    irBuffer[145] = 0.0001296803529840;
    irBuffer[146] = -0.0008110329508781;
    irBuffer[147] = -0.0013635336654261;
    irBuffer[148] = -0.0012134213466197;
    irBuffer[149] = -0.0004070691065863;
    irBuffer[150] = 0.0006557236192748;
    irBuffer[151] = 0.0014186699409038;
    irBuffer[152] = 0.0014592278748751;
    irBuffer[153] = 0.0007211973425001;
    irBuffer[154] = -0.0004376557189971;
    irBuffer[155] = -0.0014173235977069;
    irBuffer[156] = -0.0016851695254445;
    irBuffer[157] = -0.0010642196284607;
    irBuffer[158] = 0.0001552503381390;
    irBuffer[159] = 0.0013489727862179;
    irBuffer[160] = 0.0018770446768031;
    irBuffer[161] = 0.0014256818685681;
    irBuffer[162] = 0.0001905011304189;
    irBuffer[163] = -0.0012042585294694;
    irBuffer[164] = -0.0020197455305606;
    irBuffer[165] = -0.0017925744177774;
    irBuffer[166] = -0.0005957146058790;
    irBuffer[167] = 0.0009755205828696;
    irBuffer[168] = 0.0020977298263460;
    irBuffer[169] = 0.0021494934335351;
    irBuffer[170] = 0.0010533761233091;
    irBuffer[171] = -0.0006573111750185;
    irBuffer[172] = -0.0020955423824489;
    irBuffer[173] = -0.0024788931477815;
    irBuffer[174] = -0.0015531908720732;
    irBuffer[175] = 0.0002468732418492;
    irBuffer[176] = 0.0019983709789813;
    irBuffer[177] = 0.0027614291757345;
    irBuffer[178] = 0.0020815343596041;
    irBuffer[179] = 0.0002554488019086;
    irBuffer[180] = -0.0017925972351804;
    irBuffer[181] = -0.0029763551428914;
    irBuffer[182] = -0.0026214842218906;
    irBuffer[183] = -0.0008458612719551;
    irBuffer[184] = 0.0014663392212242;
    irBuffer[185] = 0.0031019740272313;
    irBuffer[186] = 0.0031529506668448;
    irBuffer[187] = 0.0015168727841228;
    irBuffer[188] = -0.0010099314386025;
    irBuffer[189] = -0.0031160889193416;
    irBuffer[190] = -0.0036528608761728;
    irBuffer[191] = -0.0022571331355721;
    irBuffer[192] = 0.0004163304984104;
    irBuffer[193] = 0.0029964295681566;
    irBuffer[194] = 0.0040953969582915;
    irBuffer[195] = 0.0030513887759298;
    irBuffer[196] = 0.0003186264366377;
    irBuffer[197] = -0.0027209594845772;
    irBuffer[198] = -0.0044521889649332;
    irBuffer[199] = -0.0038805003277957;
    irBuffer[200] = -0.0011961093405262;
    irBuffer[201] = 0.0022680191323161;
    irBuffer[202] = 0.0046924264170229;
    irBuffer[203] = 0.0047215311788023;
    irBuffer[204] = 0.0022144161630422;
    irBuffer[205] = -0.0016161559615284;
    irBuffer[206] = -0.0047827241942286;
    irBuffer[207] = -0.0055478112772107;
    irBuffer[208] = -0.0033693523146212;
    irBuffer[209] = 0.0007435118313879;
    irBuffer[210] = 0.0046866005286574;
    irBuffer[211] = 0.0063289487734437;
    irBuffer[212] = 0.0046550347469747;
    irBuffer[213] = 0.0003735880600289;
    irBuffer[214] = -0.0043631680309772;
    irBuffer[215] = -0.0070305466651917;
    irBuffer[216] = -0.0060652270913124;
    irBuffer[217] = -0.0017632801318541;
    irBuffer[218] = 0.0037645003758371;
    irBuffer[219] = 0.0076133953407407;
    irBuffer[220] = 0.0075955823995173;
    irBuffer[221] = 0.0034630466252565;
    irBuffer[222] = -0.0028304771985859;
    irBuffer[223] = -0.0080314734950662;
    irBuffer[224] = -0.0092473234981298;
    irBuffer[225] = -0.0055286134593189;
    irBuffer[226] = 0.0014789294218645;
    irBuffer[227] = 0.0082277162000537;
    irBuffer[228] = 0.0110338740050793;
    irBuffer[229] = 0.0080511523410678;
    irBuffer[230] = 0.0004142033576500;
    irBuffer[231] = -0.0081247519701719;
    irBuffer[232] = -0.0129936235025525;
    irBuffer[233] = -0.0111927380785346;
    irBuffer[234] = -0.0030573662370443;
    irBuffer[235] = 0.0076039703562856;
    irBuffer[236] = 0.0152177270501852;
    irBuffer[237] = 0.0152677698060870;
    irBuffer[238] = 0.0068449787795544;
    irBuffer[239] = -0.0064526787027717;
    irBuffer[240] = -0.0179194156080484;
    irBuffer[241] = -0.0209582298994064;
    irBuffer[242] = -0.0126397209241986;
    irBuffer[243] = 0.0042087305337191;
    irBuffer[244] = 0.0216464996337891;
    irBuffer[245] = 0.0300200320780277;
    irBuffer[246] = 0.0227684658020735;
    irBuffer[247] = 0.0004358185979072;
    irBuffer[248] = -0.0281703099608421;
    irBuffer[249] = -0.0485937669873238;
    irBuffer[250] = -0.0463617891073227;
    irBuffer[251] = -0.0134116141125560;
    irBuffer[252] = 0.0474678650498390;
    irBuffer[253] = 0.1222959980368614;
    irBuffer[254] = 0.1901284903287888;
    irBuffer[255] = 0.2303789854049683;
    irBuffer[256] = 0.2303789854049683;
    irBuffer[257] = 0.1901284903287888;
    irBuffer[258] = 0.1222959980368614;
    irBuffer[259] = 0.0474678650498390;
    irBuffer[260] = -0.0134116141125560;
    irBuffer[261] = -0.0463617891073227;
    irBuffer[262] = -0.0485937669873238;
    irBuffer[263] = -0.0281703099608421;
    irBuffer[264] = 0.0004358185979072;
    irBuffer[265] = 0.0227684658020735;
    irBuffer[266] = 0.0300200320780277;
    irBuffer[267] = 0.0216464996337891;
    irBuffer[268] = 0.0042087305337191;
    irBuffer[269] = -0.0126397209241986;
    irBuffer[270] = -0.0209582298994064;
    irBuffer[271] = -0.0179194156080484;
    irBuffer[272] = -0.0064526787027717;
    irBuffer[273] = 0.0068449787795544;
    irBuffer[274] = 0.0152677698060870;
    irBuffer[275] = 0.0152177270501852;
    irBuffer[276] = 0.0076039703562856;
    irBuffer[277] = -0.0030573662370443;
    irBuffer[278] = -0.0111927380785346;
    irBuffer[279] = -0.0129936235025525;
    irBuffer[280] = -0.0081247519701719;
    irBuffer[281] = 0.0004142033576500;
    irBuffer[282] = 0.0080511523410678;
    irBuffer[283] = 0.0110338740050793;
    irBuffer[284] = 0.0082277162000537;
    irBuffer[285] = 0.0014789294218645;
    irBuffer[286] = -0.0055286134593189;
    irBuffer[287] = -0.0092473234981298;
    irBuffer[288] = -0.0080314734950662;
    irBuffer[289] = -0.0028304771985859;
    irBuffer[290] = 0.0034630466252565;
    irBuffer[291] = 0.0075955823995173;
    irBuffer[292] = 0.0076133953407407;
    irBuffer[293] = 0.0037645003758371;
    irBuffer[294] = -0.0017632801318541;
    irBuffer[295] = -0.0060652270913124;
    irBuffer[296] = -0.0070305466651917;
    irBuffer[297] = -0.0043631680309772;
    irBuffer[298] = 0.0003735880600289;
    irBuffer[299] = 0.0046550347469747;
    irBuffer[300] = 0.0063289487734437;
    irBuffer[301] = 0.0046866005286574;
    irBuffer[302] = 0.0007435118313879;
    irBuffer[303] = -0.0033693523146212;
    irBuffer[304] = -0.0055478112772107;
    irBuffer[305] = -0.0047827241942286;
    irBuffer[306] = -0.0016161559615284;
    irBuffer[307] = 0.0022144161630422;
    irBuffer[308] = 0.0047215311788023;
    irBuffer[309] = 0.0046924264170229;
    irBuffer[310] = 0.0022680191323161;
    irBuffer[311] = -0.0011961093405262;
    irBuffer[312] = -0.0038805003277957;
    irBuffer[313] = -0.0044521889649332;
    irBuffer[314] = -0.0027209594845772;
    irBuffer[315] = 0.0003186264366377;
    irBuffer[316] = 0.0030513887759298;
    irBuffer[317] = 0.0040953969582915;
    irBuffer[318] = 0.0029964295681566;
    irBuffer[319] = 0.0004163304984104;
    irBuffer[320] = -0.0022571331355721;
    irBuffer[321] = -0.0036528608761728;
    irBuffer[322] = -0.0031160889193416;
    irBuffer[323] = -0.0010099314386025;
    irBuffer[324] = 0.0015168727841228;
    irBuffer[325] = 0.0031529506668448;
    irBuffer[326] = 0.0031019740272313;
    irBuffer[327] = 0.0014663392212242;
    irBuffer[328] = -0.0008458612719551;
    irBuffer[329] = -0.0026214842218906;
    irBuffer[330] = -0.0029763551428914;
    irBuffer[331] = -0.0017925972351804;
    irBuffer[332] = 0.0002554488019086;
    irBuffer[333] = 0.0020815343596041;
    irBuffer[334] = 0.0027614291757345;
    irBuffer[335] = 0.0019983709789813;
    irBuffer[336] = 0.0002468732418492;
    irBuffer[337] = -0.0015531908720732;
    irBuffer[338] = -0.0024788931477815;
    irBuffer[339] = -0.0020955423824489;
    irBuffer[340] = -0.0006573111750185;
    irBuffer[341] = 0.0010533761233091;
    irBuffer[342] = 0.0021494934335351;
    irBuffer[343] = 0.0020977298263460;
    irBuffer[344] = 0.0009755205828696;
    irBuffer[345] = -0.0005957146058790;
    irBuffer[346] = -0.0017925744177774;
    irBuffer[347] = -0.0020197455305606;
    irBuffer[348] = -0.0012042585294694;
    irBuffer[349] = 0.0001905011304189;
    irBuffer[350] = 0.0014256818685681;
    irBuffer[351] = 0.0018770446768031;
    irBuffer[352] = 0.0013489727862179;
    irBuffer[353] = 0.0001552503381390;
    irBuffer[354] = -0.0010642196284607;
    irBuffer[355] = -0.0016851695254445;
    irBuffer[356] = -0.0014173235977069;
    irBuffer[357] = -0.0004376557189971;
    irBuffer[358] = 0.0007211973425001;
    irBuffer[359] = 0.0014592278748751;
    irBuffer[360] = 0.0014186699409038;
    irBuffer[361] = 0.0006557236192748;
    irBuffer[362] = -0.0004070691065863;
    irBuffer[363] = -0.0012134213466197;
    irBuffer[364] = -0.0013635336654261;
    irBuffer[365] = -0.0008110329508781;
    irBuffer[366] = 0.0001296803529840;
    irBuffer[367] = 0.0009606519597583;
    irBuffer[368] = 0.0012630778364837;
    irBuffer[369] = 0.0009073557448573;
    irBuffer[370] = 0.0001056927067111;
    irBuffer[371] = -0.0007122103124857;
    irBuffer[372] = -0.0011286106891930;
    irBuffer[373] = -0.0009502378525212;
    irBuffer[374] = -0.0002962144499179;
    irBuffer[375] = 0.0004775526758749;
    irBuffer[376] = 0.0009711356833577;
    irBuffer[377] = 0.0009465577895753;
    irBuffer[378] = 0.0004412865964696;
    irBuffer[379] = -0.0002641671453603;
    irBuffer[380] = -0.0008009540033527;
    irBuffer[381] = -0.0009040769655257;
    irBuffer[382] = -0.0005422755493782;
    irBuffer[383] = 0.0000775332082412;
    irBuffer[384] = 0.0006273413309827;
    irBuffer[385] = 0.0008310037665069;
    irBuffer[386] = 0.0006021905574016;
    irBuffer[387] = 0.0000788305187598;
    irBuffer[388] = -0.0004583036352415;
    irBuffer[389] = -0.0007355912821367;
    irBuffer[390] = -0.0006253321189433;
    irBuffer[391] = -0.0002032436459558;
    irBuffer[392] = 0.0003004157624673;
    irBuffer[393] = 0.0006257825298235;
    irBuffer[394] = 0.0006169307744130;
    irBuffer[395] = 0.0002956803364214;
    irBuffer[396] = -0.0001587389997439;
    irBuffer[397] = -0.0005089115002193;
    irBuffer[398] = -0.0005827903514728;
    irBuffer[399] = -0.0003575389855541;
    irBuffer[400] = 0.0000368107284885;
    irBuffer[401] = 0.0003914636326954;
    irBuffer[402] = 0.0005289468099363;
    irBuffer[403] = 0.0003913739928976;
    irBuffer[404] = 0.0000632931623841;
    irBuffer[405] = -0.0002789076534100;
    irBuffer[406] = -0.0004613618366420;
    irBuffer[407] = -0.0004006113158539;
    irBuffer[408] = -0.0001408394600730;
    irBuffer[409] = 0.0001755936391419;
    irBuffer[410] = 0.0003856586990878;
    irBuffer[411] = 0.0003892597160302;
    irBuffer[412] = 0.0001962697569979;
    irBuffer[413] = -0.0000847206174512;
    irBuffer[414] = -0.0003069138329010;
    irBuffer[415] = -0.0003616396570578;
    irBuffer[416] = -0.0002310073177796;
    irBuffer[417] = 0.0000083545846792;
    irBuffer[418] = 0.0002294945443282;
    irBuffer[419] = 0.0003221260849386;
    irBuffer[420] = 0.0002472368068993;
    irBuffer[421] = 0.0000524965180375;
    irBuffer[422] = -0.0001569569285493;
    irBuffer[423] = -0.0002749281702563;
    irBuffer[424] = -0.0002476811932866;
    irBuffer[425] = -0.0000977704112302;
    irBuffer[426] = 0.0000919931262615;
    irBuffer[427] = 0.0002239009336336;
    irBuffer[428] = 0.0002353741147090;
    irBuffer[429] = 0.0001281958102481;
    irBuffer[430] = -0.0000364416373486;
    irBuffer[431] = -0.0001724150206428;
    irBuffer[432] = -0.0002134631940862;
    irBuffer[433] = -0.0001451331336284;
    irBuffer[434] = -0.0000086719082901;
    irBuffer[435] = 0.0001232606882695;
    irBuffer[436] = 0.0001850285043474;
    irBuffer[437] = 0.0001504003303126;
    irBuffer[438] = 0.0000430524851254;
    irBuffer[439] = -0.0000786032978795;
    irBuffer[440] = -0.0001529414148536;
    irBuffer[441] = -0.0001461128558731;
    irBuffer[442] = -0.0000670430599712;
    irBuffer[443] = 0.0000399582386308;
    irBuffer[444] = 0.0001197328674607;
    irBuffer[445] = 0.0001345096825389;
    irBuffer[446] = 0.0000814914092189;
    irBuffer[447] = -0.0000082237074821;
    irBuffer[448] = -0.0000875171608641;
    irBuffer[449] = -0.0001178134771180;
    irBuffer[450] = -0.0000876213744050;
    irBuffer[451] = -0.0000162688975252;
    irBuffer[452] = 0.0000579376974201;
    irBuffer[453] = 0.0000980972254183;
    irBuffer[454] = 0.0000868840143085;
    irBuffer[455] = 0.0000336578195856;
    irBuffer[456] = -0.0000321749212162;
    irBuffer[457] = -0.0000772124767536;
    irBuffer[458] = -0.0000808542754385;
    irBuffer[459] = -0.0000444776051154;
    irBuffer[460] = 0.0000109462253022;
    irBuffer[461] = 0.0000567109709664;
    irBuffer[462] = 0.0000711093161954;
    irBuffer[463] = 0.0000495497297379;
    irBuffer[464] = 0.0000054448237279;
    irBuffer[465] = -0.0000378185868612;
    irBuffer[466] = -0.0000591524149058;
    irBuffer[467] = -0.0000499074158142;
    irBuffer[468] = -0.0000170829262061;
    irBuffer[469] = 0.0000213879866351;
    irBuffer[470] = 0.0000463036813017;
    irBuffer[471] = 0.0000466642231913;
    irBuffer[472] = 0.0000243380400207;
    irBuffer[473] = -0.0000079384271885;
    irBuffer[474] = -0.0000336766352120;
    irBuffer[475] = -0.0000409598324040;
    irBuffer[476] = -0.0000278249426628;
    irBuffer[477] = -0.0000023518437047;
    irBuffer[478] = 0.0000221132413571;
    irBuffer[479] = 0.0000338566060236;
    irBuffer[480] = 0.0000282986147795;
    irBuffer[481] = 0.0000095742261692;
    irBuffer[482] = -0.0000122119981825;
    irBuffer[483] = -0.0000263464771706;
    irBuffer[484] = -0.0000266804745479;
    irBuffer[485] = -0.0000141687769428;
    irBuffer[486] = 0.0000041909465835;
    irBuffer[487] = 0.0000191849812836;
    irBuffer[488] = 0.0000239175660681;
    irBuffer[489] = 0.0000168605420185;
    irBuffer[490] = 0.0000021356875095;
    irBuffer[491] = -0.0000128407873490;
    irBuffer[492] = -0.0000210210473597;
    irBuffer[493] = -0.0000189057973330;
    irBuffer[494] = -0.0000078581069829;
    irBuffer[495] = 0.0000068430767897;
    irBuffer[496] = 0.0000185557037184;
    irBuffer[497] = 0.0000221549125854;
    irBuffer[498] = 0.0000159797546075;
    irBuffer[499] = 0.0000021514131276;
    irBuffer[500] = -0.0000146474785652;
    irBuffer[501] = -0.0000292139411613;
    irBuffer[502] = -0.0000377439646400;
    irBuffer[503] = -0.0000389039269066;
    irBuffer[504] = -0.0000337998826581;
    irBuffer[505] = -0.0000250798548223;
    irBuffer[506] = -0.0000157045305968;
    irBuffer[507] = -0.0000079230067058;
    irBuffer[508] = -0.0000027811349810;
    irBuffer[509] = -0.0000002019931742;
    irBuffer[510] = 0.0000005707121318;
    irBuffer[511] = 0.0000005445158422;*/

    irBuffer[0] = 0.0000000000000001;
    irBuffer[1] = 0.0076966499909759;
    irBuffer[2] = 0.0251346025615931;
    irBuffer[3] = 0.0478625856339931;
    irBuffer[4] = 0.0784611627459526;
    irBuffer[5] = 0.1136875599622726;
    irBuffer[6] = 0.1501039862632751;
    irBuffer[7] = 0.1820416748523712;
    irBuffer[8] = 0.2040369659662247;
    irBuffer[9] = 0.2117607295513153;
    irBuffer[10] = 0.2040369659662247;
    irBuffer[11] = 0.1820416748523712;
    irBuffer[12] = 0.1501039862632751;
    irBuffer[13] = 0.1136875599622726;
    irBuffer[14] = 0.0784611627459526;
    irBuffer[15] = 0.0478625856339931;
    irBuffer[16] = 0.0251346025615931;
    irBuffer[17] = 0.0076966499909759;
    irBuffer[18] = 0.0000000000000001;

    // now init the units
    interpolator.init(ratio, irSize, &irBuffer[0]);
    decimator.init(ratio, irSize, &irBuffer[0]);

    // dynamically allocate the input x buffers and save the pointers
    interpBuffer = new float[ratio];

    // flush interp buffers
    memset(interpBuffer, 0, ratio * sizeof(float));

    // dynamically allocate the input x buffers and save the pointers
    decimBuffer = new float[ratio];

    // flush deci buffers
    memset(decimBuffer, 0, ratio * sizeof(float));


    // oversampling
    interpolator.reset();
    decimator.reset();
}


/**
 * @brief
 * @param x input
 * @return output
 */
float NeoOversampler::compute(float x) {
    float y;

    /* just pass a single computation, if os is turned off */
    if (!enabled) return process(x);

    interpolator.interpolateSamples(x, interpBuffer);

    for (int i = 0; i < ratio; ++i) {
        // do computation n times out of the interpolated buffer and write back to decimator buffer
        decimBuffer[i] = process(interpBuffer[i]);
    }

    decimator.decimateSamples(decimBuffer, y);

    return y;
}


float TanhOS::process(float x) {
    return erff(x * gain) * 5.f;
}


TanhOS::TanhOS() {}
}

