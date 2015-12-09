#ifndef OPSHAPEDYNAMICRANGE_H
#define OPSHAPEDYNAMICRANGE_H
#include <QObject>
#include "operator.h"
#include "shapedynamicrange.h"

class OperatorParameterSlider;
class OperatorParameterDropDown;

class OpShapeDynamicRange : public Operator
{
    Q_OBJECT
public:
    OpShapeDynamicRange(Process *parent);
    OpShapeDynamicRange *newInstance();
    OperatorWorker *newWorker();
private slots:
    void selectShape(int);
    void selectLab(int v);

private:
    ShapeDynamicRange::Shape m_shape;
    OperatorParameterDropDown *m_shapeDialog;
    OperatorParameterSlider *m_dynamicRange;
    OperatorParameterSlider *m_exposure;
    bool m_labDomain;
    OperatorParameterDropDown *m_labDomainDialog;
};

#endif // OPSHAPEDYNAMICRANGE_H
