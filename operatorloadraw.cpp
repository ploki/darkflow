#include "process.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterdropdown.h"
#include "operatorloadraw.h"


static const char *DebayerStr[] = {
    "None", "Half Size", "Low", "VNG", "PPG", "AHD"
};
static const char *WhiteBalanceStr[] = {
    "None", "Camera", "Daylight"
};

OperatorLoadRaw::OperatorLoadRaw(Process *parent) :
    Operator(parent),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "RAW files",
                          tr("Select files to insert"),
                          m_process->outputDirectory(),
                          "RAW Images (*.nef *.cr2 *.dng *.mef *.3fr *.raf *.x3f *.pef *.arw *.nrw);;"
                          "FITS Images (*.fits *.fit);;"
                          "TIFF Images (*.tif *.tiff);;"
                          "All Files (*.*)", this)),
    m_debayer(new OperatorParameterDropDown("Debayer", DebayerStr[NoDebayer], this)),
    m_whiteBalance(new OperatorParameterDropDown("White Balance", WhiteBalanceStr[Daylight], this)),
    m_debayerValue(PPG),
    m_whiteBalanceValue(Daylight)
{
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
    m_parameters.push_back(m_debayer);
    m_parameters.push_back(m_whiteBalance);


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
