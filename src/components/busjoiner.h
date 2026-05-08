#ifndef BUSJOINER_H
#define BUSJOINER_H

#include "model/component.h"

class BusJoiner : public Component
{
public:
    BusJoiner(int bitWidth = 8);

    int bitWidth() const { return m_bitWidth; }
    void evaluate() override;
    QJsonObject toJson() const override;

private:
    int m_bitWidth;
};

#endif // BUSJOINER_H
