#include "opmicrocontrasts.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include <Magick++.h>

static void
normalizeKernel(int order, double *kernel)
{
  double div = 0;
  int i;
  int n = order * order;
  for ( i = 0 ; i < n ; ++i )
    div+=kernel[i];
  for ( i = 0 ; i < n ; ++i )
    kernel[i]/=div;
}

class WorkerMicroContrasts : public OperatorWorker {
public:
    WorkerMicroContrasts(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        double kernel[]={ 0 , -1.,  0,
                         -1.,  8., -1.,
                          0 , -1.,  0};
        normalizeKernel(3, kernel);
        newPhoto.image().convolve(3, kernel);
        return newPhoto;
    }
};

OpMicroContrasts::OpMicroContrasts(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Micro Contrasts", parent)
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
}

OpMicroContrasts *OpMicroContrasts::newInstance()
{
    return new OpMicroContrasts(m_process);
}

OperatorWorker *OpMicroContrasts::newWorker()
{
    return new WorkerMicroContrasts(m_thread, this);
}
