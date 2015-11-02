#include "process.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterdropdown.h"
#include "operatorloadraw.h"
#include "operatoroutput.h"

static const char *ColorSpaceStr[] = {
  "Linear", "sRGB", "IUT BT.709"
};
static const char *DebayerStr[] = {
    "None", "Half Size", "Low", "VNG", "PPG", "AHD"
};
static const char *WhiteBalanceStr[] = {
    "None", "Camera", "Daylight"
};

OperatorLoadRaw::OperatorLoadRaw(Process *parent) :
    Operator(parent),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "rawCollection",
                          "RAW files",
                          tr("Select RAW files to insert"),
                          m_process->outputDirectory(),
                          "RAW Images (*.nef *.cr2 *.dng *.mef *.3fr *.raf *.x3f *.pef *.arw *.nrw);;"
                          /*"FITS Images (*.fits *.fit);;"*/
                          /*"TIFF Images (*.tif *.tiff);;"*/
                          "All Files (*.*)", this)),
    m_colorSpace(new OperatorParameterDropDown("colorSpace", "Color Space", ColorSpaceStr[Linear], this)),
    m_debayer(new OperatorParameterDropDown("debayer", "Debayer", DebayerStr[NoDebayer], this)),
    m_whiteBalance(new OperatorParameterDropDown("whiteBalance", "White Balance", WhiteBalanceStr[Daylight], this)),
    m_colorSpaceValue(Linear),
    m_debayerValue(PPG),
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
    m_whiteBalance->addOption(WhiteBalanceStr[Camera], this, SLOT(setWhiteBalanceCamera()));
    m_whiteBalance->addOption(WhiteBalanceStr[Daylight], this, SLOT(setWhiteBalanceDaylight()));

    m_parameters.push_back(m_filesCollection);
    m_parameters.push_back(m_colorSpace);
    m_parameters.push_back(m_debayer);
    m_parameters.push_back(m_whiteBalance);

    m_outputs.push_back(new OperatorOutput("RAWs","RAW files colelction",this));
}

OperatorLoadRaw::~OperatorLoadRaw()
{
}

OperatorLoadRaw *OperatorLoadRaw::newInstance()
{
    return new OperatorLoadRaw(m_process);
}

QString OperatorLoadRaw::getClassIdentifier()
{
    return "Load Raw";
}

void OperatorLoadRaw::setColorSpaceLinear()
{
    m_colorSpaceValue = Linear;
    setUpToDate(false);
}

void OperatorLoadRaw::setColorSpacesRGB()
{
    m_colorSpaceValue = sRGB;
    setUpToDate(false);
}

void OperatorLoadRaw::setColorSpaceIUT_BT_709()
{
    m_colorSpaceValue = IUT_BT_709;
    setUpToDate(false);
}

void OperatorLoadRaw::setDebayerNone()
{
    m_debayerValue = NoDebayer;
    setUpToDate(false);
}

void OperatorLoadRaw::setDebayerHalfSize()
{
    m_debayerValue = HalfSize;
    setUpToDate(false);
}

void OperatorLoadRaw::setDebayerLow()
{
    m_debayerValue = Low;
    setUpToDate(false);
}

void OperatorLoadRaw::setDebayerVNG()
{
    m_debayerValue = VNG;
    setUpToDate(false);
}

void OperatorLoadRaw::setDebayerPPG()
{
    m_debayerValue = PPG;
    setUpToDate(false);
}

void OperatorLoadRaw::setDebayerAHD()
{
    m_debayerValue = AHD;
    setUpToDate(false);
}

void OperatorLoadRaw::setWhiteBalanceNone()
{
    m_whiteBalanceValue = NoWhiteBalance;
    setUpToDate(false);
}

void OperatorLoadRaw::setWhiteBalanceCamera()
{
    m_whiteBalanceValue = Camera;
    setUpToDate(false);
}

void OperatorLoadRaw::setWhiteBalanceDaylight()
{
    m_whiteBalanceValue = Camera;
    setUpToDate(false);
}

void OperatorLoadRaw::filesCollectionChanged()
{
    setUpToDate(false);
}
