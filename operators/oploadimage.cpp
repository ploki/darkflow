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
    m_colorSpace(new OperatorParameterDropDown("colorSpace", "Color Space", ColorSpaceStr[Linear], this)),
    m_colorSpaceValue(Linear)
{
    m_colorSpace->addOption(ColorSpaceStr[Linear], this, SLOT(setColorSpaceLinear()));
    m_colorSpace->addOption(ColorSpaceStr[sRGB], this, SLOT(setColorSpacesRGB()));
    m_colorSpace->addOption(ColorSpaceStr[IUT_BT_709], this, SLOT(setColorSpaceIUT_BT_709()));
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

void OpLoadImage::setColorSpaceLinear()
{
    if ( m_colorSpaceValue != Linear ) {
        m_colorSpaceValue = Linear;
        setOutOfDate();
    }
}

void OpLoadImage::setColorSpacesRGB()
{
    if ( m_colorSpaceValue != sRGB ) {
        m_colorSpaceValue = sRGB;
        setOutOfDate();
    }
}

void OpLoadImage::setColorSpaceIUT_BT_709()
{
    if ( m_colorSpaceValue != IUT_BT_709 ) {
        m_colorSpaceValue = IUT_BT_709;
        setOutOfDate();
    }
}

void OpLoadImage::filesCollectionChanged()
{
    setOutOfDate();
}
