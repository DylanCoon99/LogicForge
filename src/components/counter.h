#ifndef COUNTER_H
#define COUNTER_H

#include "model/component.h"

class Counter : public Component
{
public:
    Counter(int bitWidth = 4);

    int bitWidth() const { return m_bitWidth; }
    int count() const { return m_count; }
    void evaluate() override;
    QJsonObject toJson() const override;

private:
    int m_bitWidth;
    int m_count;
    int m_maxCount;
    Pin *m_clk;
    Pin *m_rst;
};

#endif // COUNTER_H
