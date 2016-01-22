#include "oploadvideo.h"
#include "workerloadvideo.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterslider.h"
#include "operatoroutput.h"
#include "process.h"

OpLoadVideo::OpLoadVideo(Process *process) :
    Operator(OP_SECTION_ASSETS, QT_TRANSLATE_NOOP("Operator", "Videos"), Operator::NA, process),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "videoCollection",
                          tr("Videos"),
                          tr("Select Videos to add to the collection"),
                          m_process->baseDirectory(),
                          tr("Videos (*.avi *.mjpg *.mjpeg *.ts *.m2ts *.mov *.mpg *.mpeg *.mp4 *.webm);;"
                          "All Files (*.*)"), this)),
    m_skip(new OperatorParameterSlider("skip", tr("Skip"), tr("Video Frames to skip"), Slider::Value, Slider::Linear, Slider::Integer, 0, 1000, 0, 0, 1000000, Slider::FilterPixels, this)),
    m_count(new OperatorParameterSlider("count", tr("Count"), tr("Video Frames to collect"), Slider::Value, Slider::Linear, Slider::Integer, 0, 1000, 100, 0, 1000000, Slider::FilterPixels, this))
{
    addParameter(m_filesCollection);
    addParameter(m_skip);
    addParameter(m_count);
    addOutput(new OperatorOutput(tr("Video frames"), this));
}

OpLoadVideo::~OpLoadVideo()
{
}

OpLoadVideo *OpLoadVideo::newInstance()
{
    return new OpLoadVideo(m_process);
}

OperatorWorker *OpLoadVideo::newWorker()
{
    return new WorkerLoadVideo(m_thread, this);
}

QStringList OpLoadVideo::getCollection() const
{
    return m_filesCollection->collection();
}

int OpLoadVideo::getSkip() const
{
    return DF_ROUND(m_skip->value());
}

int OpLoadVideo::getCount() const
{
    return DF_ROUND(m_count->value());
}

void OpLoadVideo::filesCollectionChanged()
{
    setOutOfDate();
}
