#include <QPointF>
#include <QRectF>
#include "workerssdreg.h"
#include <Magick++.h>

using Magick::Quantum;

class Region {
public:
  int w;
  int h;
  quantum_t *buffer;

  ~Region()
  {
      delete[] buffer;
  }


  static Region* get(Magick::Image &image, QRectF rect) {


    int px = rect.x();
    int py = rect.y();
    int w = rect.width();
    int h = rect.height();
    Magick::Pixels cache(image);
    Region *region = new Region(w,h);
    #pragma omp parallel for
    for (int y = 0 ; y < h ; ++y)
      {
        const Magick::PixelPacket *pixels = cache.getConst(px,py+y,w,1);
        for (int x = 0; x < w ; ++x )
          {
            region->buffer[y*w+x]=
              //(double(pixels[x].red)+pixels[x].green+pixels[x].blue)/3;
              pow(pow(double(pixels[x].red)*pixels[x].green*pixels[x].blue,1./3.)/QuantumRange, 1/2.2)*QuantumRange;
          }
      }
    return region;
  }


  QPointF lookup(Magick::Image &image) {


      int i_w = image.columns();
      int i_h = image.rows();
      if ( w >= i_w || h >= i_h )
          return QPointF();

      Region *haystack = Region::get(image, QRectF(0, 0, i_w, i_h));
      int dh = i_h-h;
      int dw = i_w-w;
      int ssd_sz = dh*dw;
      double *ssd = new double[ssd_sz];
#pragma omp parallel for
      for ( int dy = 0 ; dy < dh ; ++dy )
          for ( int dx = 0 ; dx < dw ; ++dx )
              //needle window
              for ( int y = 0 ; y < h ; ++y )
                  for ( int x = 0 ; x < w ; ++x ) {
                      double d = double(haystack->buffer[(y+dy)*haystack->w+(x+dx)])/QuantumRange
                              - double(buffer[y*w+x])/QuantumRange;
                      ssd[dy*dw+dx] += d*d;
                  }
      delete haystack;
      double min = ssd[0];
      int min_pos = 0;
      /*
      for ( int i = 0 ; i < ssd_sz ; ++i )
          if ( ssd[i] > min )
              min = ssd[i];
              */
      for ( int i = 1 ; i < ssd_sz ; ++i )
          if ( ssd[i] < min ) {
              min = ssd[i];
              min_pos = i;
          }
      delete[] ssd;
      QPointF res(min_pos%dw,min_pos/dw);
      qDebug("x=%f, y=%f",res.x(), res.y());
      return res;
  }

private:
  Q_DISABLE_COPY(Region)
  Region(int w_, int h_) :
      w(w_),
      h(h_),
      buffer(new quantum_t[w*h])
  {}

};


WorkerSsdReg::WorkerSsdReg(QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_refIdx(0)
{

}


Photo WorkerSsdReg::process(const Photo &photo, int, int)
{
    return photo;
}

void WorkerSsdReg::play_analyseSources()
{
    for (int i = 0, s = m_inputs[0].count() ;
            i < s ;
            ++i ) {
        if ( m_inputs[0][i].getTag("TREAT") == "REFERENCE" ) {
            m_refIdx=i;
            break;
        }
    }
}

bool WorkerSsdReg::play_onInput(int idx)
{
    Q_ASSERT( 0 == idx );
    QRectF roi = m_inputs[0][m_refIdx].getROI();
    if ( roi.isNull() )
        return OperatorWorker::play_onInput(0);

    Region *needle = Region::get(m_inputs[0][m_refIdx].image(), roi);

    for ( int i = 0, s = m_inputs[0].count() ; i < s ; ++i ) {
        if ( aborted() ) continue;
        Photo photo = m_inputs[0][i];
        QPointF off = needle->lookup(photo.image());
        QString points = QString::number(off.x()) +
                "," + QString::number(off.y());
        photo.setTag("POINTS", points);
        outputPush(0, photo);
        emitProgress(i, s, 0, 1);
    }
    delete needle;
    if ( aborted() )
        emitFailure();
    else
        emitSuccess();
    return false;
}
