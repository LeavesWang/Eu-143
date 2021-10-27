#ifndef _SETUP_H_
#define _SETUP_H_

const int ADC_MODS=3;
const int ADC_CHANS=32;
const int TDC_MODS=1;
const int TDC_CHANS=64;

struct StrBranch
{
    ULong64_t event_sign; //0:only one signal 1:true coin. 2:OR evnet 3:deadtime event 4:wrong event 
    ULong64_t timestamp[TDC_MODS+ADC_MODS];
    UInt_t tdc[TDC_MODS][TDC_CHANS];
    UShort_t adc[ADC_MODS][ADC_CHANS];
};

#endif
