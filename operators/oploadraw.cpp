#include "process.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterdropdown.h"
#include "oploadraw.h"
#include "operatoroutput.h"
#include "workerloadraw.h"

static const char *ColorSpaceStr[] = {
  "Linear", "sRGB", "IUT BT.709"
};
static const char *DebayerStr[] = {
    "None", "Half Size", "Low", "VNG", "PPG", "AHD"
};
static const char *WhiteBalanceStr[] = {
    "None", "Raw colors", "Camera", "Daylight"
};
static const char *ClippingStr[] = {
"Auto", "16-bit", "15-bit", "14-bit", "13-bit", "12-bit"
};

OpLoadRaw::OpLoadRaw(Process *parent) :
    Operator(OP_SECTION_ASSETS, QT_TRANSLATE_NOOP("Operator", "Raw photos"), Operator::NA, parent),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "rawCollection",
                          "RAW photos",
                          tr("Select RAW photos to add to the collection"),
                          m_process->baseDirectory(),
                          "RAW photos (*.nef *.cr2 *.dng *.mef *.3fr *.raf *.x3f *.pef *.arw *.nrw);;"
                          /*"FITS Images (*.fits *.fit);;"*/
                          /*"TIFF Images (*.tif *.tiff);;"*/
                          "All Files (*.*)", this)),
    m_colorSpace(new OperatorParameterDropDown("colorSpace", "Color Space",this, SLOT(setColorSpace(int)))),
    m_debayer(new OperatorParameterDropDown("debayer", "Debayer", this, SLOT(setDebayer(int)))),
    m_whiteBalance(new OperatorParameterDropDown("whiteBalance", "White Balance", this, SLOT(setWhiteBalance(int)))),
    m_clipping(new OperatorParameterDropDown("clip", "Clip Highlight", this, SLOT(setClipping(int)))),
    m_colorSpaceValue(Linear),
    m_debayerValue(NoDebayer),
    m_whiteBalanceValue(Daylight),
    m_clippingValue(ClipAuto)
{
    m_colorSpace->addOption(ColorSpaceStr[Linear], Linear, true);
    m_colorSpace->addOption(ColorSpaceStr[sRGB], sRGB);
    m_colorSpace->addOption(ColorSpaceStr[IUT_BT_709], IUT_BT_709);

    m_debayer->addOption(DebayerStr[NoDebayer], NoDebayer, true);
    m_debayer->addOption(DebayerStr[HalfSize], HalfSize);
    m_debayer->addOption(DebayerStr[Low], Low);
    m_debayer->addOption(DebayerStr[VNG], VNG);
    m_debayer->addOption(DebayerStr[PPG], PPG);
    m_debayer->addOption(DebayerStr[AHD], AHD);

    m_whiteBalance->addOption(WhiteBalanceStr[NoWhiteBalance], NoWhiteBalance);
    m_whiteBalance->addOption(WhiteBalanceStr[RawColors], RawColors);
    m_whiteBalance->addOption(WhiteBalanceStr[Camera], Camera);
    m_whiteBalance->addOption(WhiteBalanceStr[Daylight], Daylight, true);

    m_clipping->addOption(ClippingStr[ClipAuto], ClipAuto, true);
    m_clipping->addOption(ClippingStr[Clip16bit], Clip16bit);
    m_clipping->addOption(ClippingStr[Clip15bit], Clip15bit);
    m_clipping->addOption(ClippingStr[Clip14bit], Clip14bit);
    m_clipping->addOption(ClippingStr[Clip13bit], Clip13bit);
    m_clipping->addOption(ClippingStr[Clip12bit], Clip12bit);

    addParameter(m_filesCollection);
    addParameter(m_colorSpace);
    addParameter(m_debayer);
    addParameter(m_whiteBalance);
    addParameter(m_clipping);

    addOutput(new OperatorOutput("RAWs","RAW photos collection",this));
}

OpLoadRaw::~OpLoadRaw()
{
}

OpLoadRaw *OpLoadRaw::newInstance()
{
    return new OpLoadRaw(m_process);
}

QStringList OpLoadRaw::getCollection() const
{
    return m_filesCollection->collection();
}

QString OpLoadRaw::getColorSpace() const
{
    return ColorSpaceStr[m_colorSpaceValue];
}

QString OpLoadRaw::getDebayer() const
{
    return DebayerStr[m_debayerValue];
}

QString OpLoadRaw::getWhiteBalance() const
{
    return WhiteBalanceStr[m_whiteBalanceValue];
}

void OpLoadRaw::setColorSpace(int v)
{
    if ( m_colorSpaceValue != v ) {
        m_colorSpaceValue = ColorSpace(v);
        setOutOfDate();
    }
}

void OpLoadRaw::setDebayer(int v)
{
    if ( m_debayerValue != v ) {
        m_debayerValue = Debayer(v);
        setOutOfDate();
    }
}

void OpLoadRaw::setWhiteBalance(int v)
{
    if ( m_whiteBalanceValue != v ) {
        m_whiteBalanceValue = WhiteBalance(v);
        setOutOfDate();
    }
}

void OpLoadRaw::setClipping(int v)
{
    if ( m_clippingValue != v ) {
        m_clippingValue = Clipping(v);
        setOutOfDate();
    }
}


void OpLoadRaw::filesCollectionChanged()
{
    setOutOfDate();
}

OperatorWorker *OpLoadRaw::newWorker() {
    return new WorkerLoadRaw(m_thread, this);
}
