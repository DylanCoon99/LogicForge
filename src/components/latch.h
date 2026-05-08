#ifndef LATCH_H
#define LATCH_H

#include "model/component.h"

class Latch : public Component
{
public:
    enum Type { SR, D };

    Latch(Type type);

    Type latchType() const { return m_type; }
    bool qState() const { return m_q; }
    void evaluate() override;
    QJsonObject toJson() const override;

    static QString typeName(Type type);

private:
    Type m_type;
    bool m_q;
};

#endif // LATCH_H
