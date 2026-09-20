#pragma once

#include <cstdint>
#include "samplerate.h"
//#include "TELEXo/CVOutput.h"

float CVOutputs_quant_oraclewrapper(uint8_t output, int16_t value);

enum OracleState : uint8_t
{
    None,
    QuestionAsked,
    QuestionAnswered
};

struct Oracle
{
    volatile OracleState state = None;

    volatile uint8_t question = 0;

    volatile uint8_t output = 0;

    volatile int16_t value = 0;

    volatile float floatAnswer = 0.0f;
    volatile int32_t intAnswer = 0;
};

inline Oracle oracle;

__attribute__((always_inline))
inline void __not_in_flash_func(oracle_think)()
{
    switch (oracle.question)
    {
        case TO_OSC:
        {
            // expensive calculation here
            oracle.floatAnswer = TxHelper::VOct2Frequency(oracle.value);
            //oracle.intAnswer = (int)(freq * FULLPHASE_SAMPLINGRATE);
            break;
        }
        case TO_OSC_SET:
        {
            // expensive calculation here
            oracle.floatAnswer = TxHelper::VOct2Frequency(oracle.value);
            //oracle.intAnswer = (int)(freq * FULLPHASE_SAMPLINGRATE);
            break;
        }
        case TO_OSC_QT:
        { 
            // expensive calculation here
            oracle.floatAnswer = CVOutputs_quant_oraclewrapper(oracle.output, oracle.value);
            break;
        }
        case TO_OSC_QT_SET:
            // expensive calculation here
            oracle.floatAnswer = CVOutputs_quant_oraclewrapper(oracle.output, oracle.value);
            break;

        default:
            break;
    }

    oracle.state = QuestionAnswered;
}

__attribute__((always_inline))
inline void __not_in_flash_func(oracle_ask)(uint8_t cmd, uint8_t out, int16_t value)
{
    oracle.question = cmd;
    oracle.output = out;
    oracle.value = value;
    oracle.state = QuestionAsked;
}

void oracle_answer();
