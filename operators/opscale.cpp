#include "opscale.h"
#include "operatorworker.h"
#include "operatorparameterdropdown.h"
#include "operatorparameterslider.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>


const char *FunctionStr[] = {
    "Resize",
    "Sample",
    "Scale"
};
const char *ResizeToStr[] = {
    "To Specified",
    "To Smallest Width",
    "To Smallest Height"
    "To Largest Width",
    "To Largest height"
};

class WorkerScale : public OperatorWorker {
public:
    WorkerScale(OpScale::Function function,
                Magick::FilterTypes algorithm,
                OpScale::ResizeTo to,
                qreal scale,
                QThread *thread,
                Operator *op) :
        OperatorWorker(thread, op),
        m_function(function),
        m_algorithm(algorithm),
        m_to(to),
        m_scale(scale),
        max_w(0),
        max_h(0),
        min_w(0),
        min_h(0)
    {
    }
    void play_analyseSources() {

        for (int i = 0, s = m_inputs[0].count() ;
             i < s ;
             ++i ) {
            int w = m_inputs[0][i].image().columns();
            int h = m_inputs[0][i].image().rows();
            if ( 0 == i ) {
                min_w = max_w = w;
                min_h = max_h = h;
            }
            else {
                if ( w < min_w ) min_w = w;
                if ( w > max_w ) max_w = w;
                if ( h < min_h ) min_h = h;
                if ( h > max_h ) max_h = h;
            }
        }
    }

    Photo process(const Photo &photo, int , int ) {
        Photo newPhoto(photo);
        Magick::Image& image = newPhoto.image();
        image.filterType(m_algorithm);
        int w = image.columns(), h = image.rows();
        switch (m_to) {
        case OpScale::ToSpecified:
            w*=m_scale;
            h*=m_scale;
            break;
        case OpScale::ToSmallestWidth:
            h*=qreal(min_w)/w;
            w=min_w;
            break;
        case OpScale::ToSmallestHeight:
            w*=qreal(min_h)/h;
            h=min_h;
            break;
        case OpScale::ToLargestWidth:
            h*=qreal(max_w)/w;
            w=max_w;
            break;
        case OpScale::ToLargestHeight:
            w*=qreal(max_h)/h;
            h=max_h;
            break;
        }
        switch (m_function) {
        case OpScale::Resize:
            image.resize(Magick::Geometry(w,h));
            break;
        case OpScale::Sample:
            image.sample(Magick::Geometry(w,h));
            break;
        case OpScale::Scale:
            image.scale(Magick::Geometry(w,h));
            break;
        }
        return newPhoto;
    }

private:
    OpScale::Function m_function;
    Magick::FilterTypes m_algorithm;
    OpScale::ResizeTo m_to;
    qreal m_scale;
    int max_w, max_h;
    int min_w, min_h;
};


OpScale::OpScale(Process *parent) :
    Operator(OP_SECTION_GEOMETRY, "Scale", parent),
    m_function(new OperatorParameterDropDown("function", "Function", this, SLOT(selectFunction(int)))),
    m_functionValue(Resize),
    m_algorithm(new OperatorParameterDropDown("algorithm", "Algorithm", this, SLOT(selectAlgorithm(int)))),
    m_algorithmValue(Magick::UndefinedFilter),
    m_to(new OperatorParameterDropDown("resizeTo", "Resize to", this, SLOT(selectResizeTo(int)))),
    m_toValue(ToSpecified),
    m_scale(new OperatorParameterSlider("scale", "Scale", "Scale", Slider::Value, Slider::Logarithmic, Slider::Real, 1./4., 4, 1, 1./1024, 8, Slider::FilterPercent, this))
{
    m_function->addOption(FunctionStr[Resize], Resize, true);
    m_function->addOption(FunctionStr[Sample], Sample);
    m_function->addOption(FunctionStr[Scale], Scale);

    m_algorithm->addOption("Undefined",Magick::UndefinedFilter, true);
    m_algorithm->addOption("Point", Magick::PointFilter);
    m_algorithm->addOption("Box", Magick::BoxFilter);
    m_algorithm->addOption("Triangle", Magick::TriangleFilter);
    m_algorithm->addOption("Hermite", Magick::HermiteFilter);
    m_algorithm->addOption("Hanning", Magick::HanningFilter);
    m_algorithm->addOption("Hamming", Magick::HammingFilter);
    m_algorithm->addOption("Blackman", Magick::BlackmanFilter);
    m_algorithm->addOption("Gaussian", Magick::GaussianFilter);
    m_algorithm->addOption("Quadratic", Magick::QuadraticFilter);
    m_algorithm->addOption("Cubic", Magick::CubicFilter);
    m_algorithm->addOption("Catrom", Magick::CatromFilter);
    m_algorithm->addOption("Mitchell", Magick::MitchellFilter);
    m_algorithm->addOption("Jinc", Magick::JincFilter);
    m_algorithm->addOption("Sinc", Magick::SincFilter);
    m_algorithm->addOption("Sinc Fast", Magick::SincFastFilter);
    m_algorithm->addOption("Kaiser", Magick::KaiserFilter);
    m_algorithm->addOption("Welsh", Magick::WelshFilter);
    m_algorithm->addOption("Parzen", Magick::ParzenFilter);
    m_algorithm->addOption("Bohman", Magick::BohmanFilter);
    m_algorithm->addOption("Bartlett", Magick::BartlettFilter);
    m_algorithm->addOption("Lagrange", Magick::LagrangeFilter);
    m_algorithm->addOption("Lanczos", Magick::LanczosFilter);
    m_algorithm->addOption("Lanczos Sharp", Magick::LanczosSharpFilter);
    m_algorithm->addOption("Lanczos2", Magick::Lanczos2Filter);
    m_algorithm->addOption("Lanczos2 Sharp", Magick::Lanczos2SharpFilter);
    m_algorithm->addOption("Robidoux", Magick::RobidouxFilter);
    m_algorithm->addOption("Robidoux Sharp", Magick::RobidouxSharpFilter);
    m_algorithm->addOption("Cosine", Magick::CosineFilter);
    m_algorithm->addOption("Spline", Magick::SplineFilter);
    m_algorithm->addOption("Lanczos Radius", Magick::LanczosRadiusFilter);

    m_to->addOption(ResizeToStr[ToSpecified], ToSpecified, true);
    m_to->addOption(ResizeToStr[ToSmallestWidth], ToSmallestWidth);
    m_to->addOption(ResizeToStr[ToSmallestHeight], ToSmallestHeight);
    m_to->addOption(ResizeToStr[ToLargestWidth], ToLargestWidth);
    m_to->addOption(ResizeToStr[ToLargestHeight], ToLargestHeight);

    addParameter(m_function);
    addParameter(m_algorithm);
    addParameter(m_to);
    addParameter(m_scale);
    addInput(new OperatorInput("Images","Image", OperatorInput::Set, this));
    addOutput(new OperatorOutput("Scaled", "Scaled", this));
}

OpScale *OpScale::newInstance()
{
    return new OpScale(m_process);
}

OperatorWorker *OpScale::newWorker()
{
    return new WorkerScale(m_functionValue, Magick::FilterTypes(m_algorithmValue), m_toValue, m_scale->value(), m_thread, this);
}

void OpScale::selectResizeTo(int v)
{
    if ( m_toValue != v ) {
        m_toValue = ResizeTo(v);
        setOutOfDate();
    }
}

void OpScale::selectAlgorithm(int v)
{
    if ( m_algorithmValue != v ) {
        m_algorithmValue= Magick::FilterTypes(v);
        setOutOfDate();
    }
}

void OpScale::selectFunction(int v)
{
    if ( m_functionValue!= v ) {
        m_functionValue = Function(v);
        setOutOfDate();
    }
}

