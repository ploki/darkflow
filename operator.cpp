#include <QThread>

#include "operator.h"
#include "process.h"
#include "image.h"
#include "operatorparameter.h"
#include "operatorinput.h"
#include "operatorworker.h"

Operator::Operator(Process *parent) :
    QObject(NULL),
    m_process(parent),
    m_enabled(true),
    m_upToDate(false),
    m_parameters(),
    m_inputs(),
    m_sources(),
    m_sinks(),
    m_thread(new QThread(this)),
    m_worker(NULL)
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

QVector<OperatorParameter *> Operator::getParameters()
{
    return m_parameters;
}

QVector<OperatorInput *> Operator::getInputs()
{
    return m_inputs;
}

void Operator::abort()
{
    m_thread->requestInterruption();
}

void Operator::clone()
{
    Operator *op = newInstance();
    m_process->addOperator(op);
}

void Operator::workerProgress(int p, int c)
{
    emit progress(p,c);
}

void Operator::workerSuccess()
{
    setUpToDate(true);
    m_thread->quit();
    m_worker=NULL;
    emit upToDate();
}

void Operator::workerFailure()
{
    setUpToDate(false);
    m_thread->quit();
    m_worker=NULL;
}

QVector<Image *> Operator::getResult() const
{
    return m_result;
}

void Operator::play() {
    setUpToDate(false);
    if (!m_worker) {
        m_worker = newWorker();
    }
}

bool Operator::isUpToDate() const
{
    return m_upToDate;
}

void Operator::setUpToDate(bool upToDate)
{
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

