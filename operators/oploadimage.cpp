#include "process.h"
#include "oploadimage.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterdropdown.h"
#include "operatoroutput.h"
#include "operatorworker.h"
#include "workerloadimage.h"

static const char *ColorSpaceStr[] = {
  "Linear", "sRGB", "IUT BT.709"
};

OpLoadImage::OpLoadImage(Process *parent) :
    Operator(OP_SECTION_ASSETS, "Images", parent),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "imageCollection",
                          "Images",
                          tr("Select images to add to the collection"),
                          m_process->outputDirectory(),
                          "FITS Images (*.fits *.fit);;"
                          "TIFF Images (*.tif *.tiff);;"
                          "All Files (*.*)", this)),
    m_colorSpace(new OperatorParameterDropDown("colorSpace", "Color Space", this, SLOT(setColorSpace(int)))),
    m_colorSpaceValue(Linear)
{
    m_colorSpace->addOption(ColorSpaceStr[Linear], Linear, true);
    m_colorSpace->addOption(ColorSpaceStr[sRGB], sRGB);
    m_colorSpace->addOption(ColorSpaceStr[IUT_BT_709], IUT_BT_709);
    addParameter(m_filesCollection);
    addParameter(m_colorSpace);
    addOutput(new OperatorOutput("Images","Images collection",this));
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
