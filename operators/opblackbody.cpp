#include "opblackbody.h"
#include "operatorworker.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "whitebalance.h"
#include "photo.h"
#include  <Magick++.h>

using Magick::Quantum;

class WorkerBlackBody : public OperatorWorker {
public:
    WorkerBlackBody(qreal temperature, qreal tint, qreal value, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_temperature(temperature),
        m_tint(tint),
        m_value(value)
    {}
private slots:
    Photo process(const Photo &, int, int) { throw 0; }
    void play() {
        Photo photo;
        photo.setIdentity(m_operator->m_uuid);
        photo.createImage(1,1);
        if ( photo.isComplete() ) {
            double rgb[3];
            WhiteBalance::Temperature_to_RGB(m_temperature, rgb);
            rgb[1]/=m_tint;
            double div=rgb[0];
            if (div < rgb[1] ) div=rgb[1];
            if (div < rgb[2] ) div=rgb[2];
            rgb[0]/=div;
            rgb[1]/=div;
            rgb[2]/=div;
            rgb[0]*=m_value;
            rgb[1]*=m_value;
            rgb[2]*=m_value;
            photo.image().pixelColor(0, 0,
                     Magick::Color(
                        clamp<quantum_t>(rgb[0]*QuantumRange, 0, QuantumRange),
                        clamp<quantum_t>(rgb[1]*QuantumRange, 0, QuantumRange),
                        clamp<quantum_t>(rgb[2]*QuantumRange, 0, QuantumRange)));
            //qWarning(QString("r: %0, g: %1, b: %2").arg(rgb[0]).arg(rgb[1]).arg(rgb[2]).toLatin1());
            photo.setTag("Name", QString("Black Body %0 K").arg(m_temperature));
            m_operator->m_outputs[0]->m_result.push_back(photo);
            emitSuccess();
        }
    }

private:
    qreal m_temperature;
    qreal m_tint;
    qreal m_value;
};

OpBlackBody::OpBlackBody(Process *parent) :
    Operator(OP_SECTION_UTILITY, "Black Body", parent),
    m_temperature(new OperatorParameterSlider("temperature", "Temperature", "Black Body Temperature",
                                              Slider::Value, Slider::Logarithmic, Slider::Integer,
                                              2000, 12000, 6500, 2000, 12000, Slider::FilterNothing,this)),
    m_tint(new OperatorParameterSlider("tint", "Green tint", "Black Body Green Tint",
                                       Slider::Percent, Slider::Logarithmic, Slider::Real,
                                       0.5, 2, 1, 0.01, 100, Slider::FilterNothing, this)),
    m_value(new OperatorParameterSlider("intensity", "Intensity", "Black Body Value",
                                        Slider::ExposureValue, Slider::Logarithmic, Slider::Real,
                                        1./(1<<8), 1, .125, 1./QuantumRange, 1, Slider::FilterExposureFromOne, this))

{
    m_outputs.push_back(new OperatorOutput("Black body", "Black Body", this));
    m_parameters.push_back(m_temperature);
    m_parameters.push_back(m_tint);
    m_parameters.push_back(m_value);

}

OpBlackBody *OpBlackBody::newInstance()
{
    return new OpBlackBody(m_process);
}

OperatorWorker *OpBlackBody::newWorker()
{
    return new WorkerBlackBody(m_temperature->value(),
                               m_tint->value(),
                               m_value->value(),
                               m_thread, this);
}
