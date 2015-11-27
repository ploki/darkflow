#include "oploadvideo.h"
#include "workerloadvideo.h"
#include "operatorparameterfilescollection.h"
#include "operatoroutput.h"
#include "process.h"

OpLoadVideo::OpLoadVideo(Process *process) :
    Operator(OP_SECTION_SOURCE_IMAGES, "Videos", process),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "videoCollection",
                          "Videos",
                          tr("Select Videos to add to the collection"),
                          m_process->outputDirectory(),
                          "Videos (*.avi *.mjpg *.mjpeg *.ts *.mov *.mpg *.mpeg *.mp4 *.webm *);;"
                          "All Files (*.*)", this))
{
    m_parameters.push_back(m_filesCollection);
    m_outputs.push_back(new OperatorOutput("Video frames","Video frames collection",this));
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

void OpLoadVideo::filesCollectionChanged()
{
    setOutOfDate();
}
