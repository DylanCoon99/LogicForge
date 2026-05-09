#ifndef TRISTATEBUFFER_H
#define TRISTATEBUFFER_H

#include "model/component.h"

class TriStateBuffer : public Component
{
public:
    TriStateBuffer(bool inverting = false);

    bool isInverting() const { return m_inverting; }
    void evaluate() override;
    QJsonObject toJson() const override;

private:
    bool m_inverting;
};

#endif
