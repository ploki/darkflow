#include "process.h"
#include "oploadimage.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterdropdown.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "workerloadimage.h"

static const char *ColorSpaceStr[] = {
    QT_TRANSLATE_NOOP("OpLoadImage", "Linear"),
    QT_TRANSLATE_NOOP("OpLoadImage", "sRGB"),
    QT_TRANSLATE_NOOP("OpLoadImage", "IUT BT.709")
};

OpLoadImage::OpLoadImage(Process *parent) :
    Operator(OP_SECTION_ASSETS, QT_TRANSLATE_NOOP("Operator", "Images"), Operator::NA, parent),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "imageCollection",
                          tr("Images"),
                          tr("Select images to add to the collection"),
                          m_process->baseDirectory(),
                          tr("FITS Images (*.fits *.fit);;"
                          "TIFF Images (*.tif *.tiff);;"
                          "All Files (*.*)"), this)),
    m_colorSpace(new OperatorParameterDropDown("colorSpace", tr("Color Space"), this, SLOT(setColorSpace(int)))),
    m_colorSpaceValue(Linear)
{
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[Linear]), Linear, true);
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[sRGB]), sRGB);
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[IUT_BT_709]), IUT_BT_709);
    addParameter(m_filesCollection);
    addParameter(m_colorSpace);
    addOutput(new OperatorOutput(tr("Images"),this));
}

OpLoadImage *OpLoadImage::newInstance()
{
    return new OpLoadImage(m_process);
}

OperatorWorker *OpLoadImage::newWorker()
{
    QVector<QString> filesCollection = m_filesCollection->collection().toVector();
    return new WorkerLoadImage(filesCollection, m_colorSpaceValue, m_thread, this);
}

void OpLoadImage::setColorSpace(int v)
{
    if ( m_colorSpaceValue != v ) {
        m_colorSpaceValue = ColorSpace(v);
        setOutOfDate();
    }
}

void OpLoadImage::filesCollectionChanged()
{
    setOutOfDate();
}
