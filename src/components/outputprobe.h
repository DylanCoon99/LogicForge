#ifndef OUTPUTPROBE_H
#define OUTPUTPROBE_H

#include "model/component.h"

class OutputProbe : public Component
{
public:
    OutputProbe();

    Pin::State readState() const;
    void evaluate() override;
};

#endif // OUTPUTPROBE_H
