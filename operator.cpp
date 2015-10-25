#include "operator.h"
#include "process.h"
#include "image.h"
#include "operatorparameter.h"
#include "operatorinput.h"

Operator::Operator(Process *parent) :
    QObject(NULL),
    m_process(parent),
    m_enabled(true),
    m_upToDate(false),
    m_complete(1),
    m_progress(0),
    m_parameters(),
    m_inputs(),
    m_sources(),
    m_sinks()
{

}

Operator::~Operator()
{
    foreach(OperatorParameter *p, m_parameters)
        delete p;
    foreach(OperatorInput *i, m_inputs)
        delete i;
    foreach(Image *im, m_result)
        delete im;
}

double Operator::getCompletion()
{
    return double(m_progress)/m_complete;
}

void Operator::play()
{
    bool parentDirty = false;
    foreach(Operator *op, m_sources ) {
        if ( !op->isUpToDate() ) {
            parentDirty = true;
            op->play();
        }
    }
    if ( !parentDirty && isUpToDate())
        return;

    setUpToDate(false);
    foreach(Operator *op, m_sources ) {
        const QVector<Image*> source = op->getResult();
        foreach(const Image *image, source) {
            Image *newResult = process(image);
            m_result.push_back(newResult);
        }
    }
}

void Operator::abort()
{

}

void Operator::clone()
{
    Operator *op = newInstance();
    m_process->addOperator(op);
}

QVector<Image *> Operator::getResult() const
{
    return m_result;
}

bool Operator::isUpToDate() const
{
    return m_upToDate;
}

void Operator::setUpToDate(bool upToDate)
{
    m_complete = 1;
    m_progress = 0;
    m_upToDate = upToDate;
    if (!upToDate) {
        foreach(Operator *op, m_sinks)
            op->setUpToDate(false);
        foreach(Image *image, m_result) {
            image->remove();
            delete image;
        }
        m_result.clear();
    }
}

bool Operator::isEnabled() const
{
    return m_enabled;
}

void Operator::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

