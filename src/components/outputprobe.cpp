#include "outputprobe.h"

OutputProbe::OutputProbe()
    : Component("Output Probe")
{
    addInputPin("IN");
}

Pin::State OutputProbe::readState() const
{
    return m_inputs[0]->state();
}

void OutputProbe::evaluate()
{
    // Output probe just reads its input — nothing to compute
}
