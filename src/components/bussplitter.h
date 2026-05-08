#ifndef BUSSPLITTER_H
#define BUSSPLITTER_H

#include "model/component.h"

class BusSplitter : public Component
{
public:
    BusSplitter(int bitWidth = 8);

    int bitWidth() const { return m_bitWidth; }
    void evaluate() override;
    QJsonObject toJson() const override;

private:
    int m_bitWidth;
};

#endif // BUSSPLITTER_H
