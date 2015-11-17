#include "process.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterdropdown.h"
#include "operatorloadraw.h"
#include "operatoroutput.h"
#include "rawconvert.h"

static const char *ColorSpaceStr[] = {
  "Linear", "sRGB", "IUT BT.709"
};
static const char *DebayerStr[] = {
    "None", "Half Size", "Low", "VNG", "PPG", "AHD"
};
static const char *WhiteBalanceStr[] = {
    "None", "Raw colors", "Camera", "Daylight"
};

OperatorLoadRaw::OperatorLoadRaw(Process *parent) :
    Operator(OP_SECTION_SOURCE_IMAGES, "Raw photos", parent),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "rawCollection",
                          "RAW photos",
                          tr("Select RAW photos to add to the collection"),
                          m_process->outputDirectory(),
                          "RAW photos (*.nef *.cr2 *.dng *.mef *.3fr *.raf *.x3f *.pef *.arw *.nrw);;"
                          /*"FITS Images (*.fits *.fit);;"*/
                          /*"TIFF Images (*.tif *.tiff);;"*/
                          "All Files (*.*)", this)),
    m_colorSpace(new OperatorParameterDropDown("colorSpace", "Color Space", ColorSpaceStr[Linear], this)),
    m_debayer(new OperatorParameterDropDown("debayer", "Debayer", DebayerStr[NoDebayer], this)),
    m_whiteBalance(new OperatorParameterDropDown("whiteBalance", "White Balance", WhiteBalanceStr[Daylight], this)),
    m_colorSpaceValue(Linear),
    m_debayerValue(NoDebayer),
    m_whiteBalanceValue(Daylight)
{
    m_colorSpace->addOption(ColorSpaceStr[Linear], this, SLOT(setColorSpaceLinear()));
    m_colorSpace->addOption(ColorSpaceStr[sRGB], this, SLOT(setColorSpacesRGB()));
    m_colorSpace->addOption(ColorSpaceStr[IUT_BT_709], this, SLOT(setColorSpaceIUT_BT_709()));

    m_debayer->addOption(DebayerStr[NoDebayer], this, SLOT(setDebayerNone()));
    m_debayer->addOption(DebayerStr[HalfSize], this, SLOT(setDebayerHalfSize()));
    m_debayer->addOption(DebayerStr[Low], this, SLOT(setDebayerLow()));
    m_debayer->addOption(DebayerStr[VNG], this, SLOT(setDebayerVNG()));
    m_debayer->addOption(DebayerStr[PPG], this, SLOT(setDebayerPPG()));
    m_debayer->addOption(DebayerStr[AHD], this, SLOT(setDebayerAHD()));

    m_whiteBalance->addOption(WhiteBalanceStr[NoWhiteBalance], this, SLOT(setWhiteBalanceNone()));
    m_whiteBalance->addOption(WhiteBalanceStr[RawColors], this, SLOT(setWhiteBalanceRawColors()));
    m_whiteBalance->addOption(WhiteBalanceStr[Camera], this, SLOT(setWhiteBalanceCamera()));
    m_whiteBalance->addOption(WhiteBalanceStr[Daylight], this, SLOT(setWhiteBalanceDaylight()));

    m_parameters.push_back(m_filesCollection);
    m_parameters.push_back(m_colorSpace);
    m_parameters.push_back(m_debayer);
    m_parameters.push_back(m_whiteBalance);

    m_outputs.push_back(new OperatorOutput("RAWs","RAW photos collection",this));
}

OperatorLoadRaw::~OperatorLoadRaw()
{
}

OperatorLoadRaw *OperatorLoadRaw::newInstance()
{
    return new OperatorLoadRaw(m_process);
}

QStringList OperatorLoadRaw::getCollection() const
{
    return m_filesCollection->collection();
}

QString OperatorLoadRaw::getColorSpace() const
{
    return ColorSpaceStr[m_colorSpaceValue];
}

QString OperatorLoadRaw::getDebayer() const
{
    return DebayerStr[m_debayerValue];
}

QString OperatorLoadRaw::getWhiteBalance() const
{
    return WhiteBalanceStr[m_whiteBalanceValue];
}

void OperatorLoadRaw::setColorSpaceLinear()
{
    if ( m_colorSpaceValue != Linear ) {
        m_colorSpaceValue = Linear;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setColorSpacesRGB()
{
    if ( m_colorSpaceValue != sRGB ) {
        m_colorSpaceValue = sRGB;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setColorSpaceIUT_BT_709()
{
    if ( m_colorSpaceValue != IUT_BT_709 ) {
        m_colorSpaceValue = IUT_BT_709;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setDebayerNone()
{
    if ( m_debayerValue != NoDebayer ) {
        m_debayerValue = NoDebayer;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setDebayerHalfSize()
{
    if ( m_debayerValue != HalfSize ) {
        m_debayerValue = HalfSize;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setDebayerLow()
{
    if ( m_debayerValue != Low ) {
        m_debayerValue = Low;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setDebayerVNG()
{
    if ( m_debayerValue != VNG ) {
        m_debayerValue = VNG;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setDebayerPPG()
{
    if ( m_debayerValue != PPG ) {
        m_debayerValue = PPG;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setDebayerAHD()
{
    if ( m_debayerValue != AHD ) {
        m_debayerValue = AHD;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setWhiteBalanceNone()
{
    if ( m_whiteBalanceValue != NoWhiteBalance ) {
        m_whiteBalanceValue = NoWhiteBalance;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setWhiteBalanceRawColors()
{
    if ( m_whiteBalanceValue != RawColors ) {
        m_whiteBalanceValue = RawColors;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setWhiteBalanceCamera()
{
    if ( m_whiteBalanceValue != Camera ) {
        m_whiteBalanceValue = Camera;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::setWhiteBalanceDaylight()
{
    if ( m_whiteBalanceValue != Daylight ) {
        m_whiteBalanceValue = Daylight;
        setUpToDate(false);
    }
}

void OperatorLoadRaw::filesCollectionChanged()
{
    setUpToDate(false);
}

OperatorWorker *OperatorLoadRaw::newWorker() {
    return new RawConvert(m_thread, this);
}
