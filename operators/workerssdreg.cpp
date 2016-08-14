/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
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
  WorkerSsdReg *m_worker;

  ~Region()
  {
      delete[] buffer;
  }


  static Region* get(WorkerSsdReg *worker, Magick::Image &image, QRectF rect) {


    int px = rect.x();
    int py = rect.y();
    int w = rect.width();
    int h = rect.height();
    std::shared_ptr<Ordinary::Pixels> cache(new Ordinary::Pixels(image));
    Region *region = new Region(worker, w, h);
    dfl_parallel_for(y, 0, h, 4, (image), {
        const Magick::PixelPacket *pixels = cache->getConst(px,py+y,w,1);
        for (int x = 0; x < w ; ++x )
          {
            region->buffer[y*w+x]=
              //(double(pixels[x].red)+pixels[x].green+pixels[x].blue)/3;
              pow(pow(double(pixels[x].red)*pixels[x].green*pixels[x].blue,1./3.)/QuantumRange, 1/2.2)*QuantumRange;
          }
      });
    return region;
  }


  QPointF lookup(WorkerSsdReg *worker, Magick::Image &image) {


      int i_w = image.columns();
      int i_h = image.rows();
      if ( w >= i_w || h >= i_h )
          return QPointF();

      Region *haystack = Region::get(worker, image, QRectF(0, 0, i_w, i_h));
      int dh = i_h-h;
      int dw = i_w-w;
      int ssd_sz = dh*dw;
      double *ssd = new double[ssd_sz];
      memset(ssd, 0, ssd_sz * sizeof(*ssd));
      dfl_parallel_for(dy, 0, dh, 1, (), {
          for ( int dx = 0 ; dx < dw ; ++dx )
              //needle window
              for ( int y = 0 ; y < h ; ++y )
                  for ( int x = 0 ; x < w ; ++x ) {
                      double d = double(haystack->buffer[(y+dy)*haystack->w+(x+dx)])/QuantumRange
                              - double(buffer[y*w+x])/QuantumRange;
                      ssd[dy*dw+dx] += d*d;
                  }
      });
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
      m_worker->dflDebug("x=%f, y=%f",res.x(), res.y());
      return res;
  }

private:
  Q_DISABLE_COPY(Region)
  Region(WorkerSsdReg *worker, int w_, int h_) :
      w(w_),
      h(h_),
      buffer(new quantum_t[w*h]),
      m_worker(worker)
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
        if ( m_inputs[0][i].getTag(TAG_TREAT) == TAG_TREAT_REFERENCE ) {
            m_refIdx=i;
            break;
        }
    }
}

bool WorkerSsdReg::play_onInput(int idx)
{
    Q_ASSERT( 0 == idx );
    if (m_inputs[0].count() == 0 )
        return OperatorWorker::play_onInput(idx);
    QRectF roi = m_inputs[0][m_refIdx].getROI();
    if ( roi.isNull() )
        return OperatorWorker::play_onInput(0);

    Region *needle = Region::get(this, m_inputs[0][m_refIdx].image(), roi);

    for ( int i = 0, s = m_inputs[0].count() ; i < s ; ++i ) {
        if ( aborted() ) continue;

        Photo photo = m_inputs[0][i];
        try {
            QPointF off = needle->lookup(this, photo.image());
            QString points = QString::number(off.x()) +
                    "," + QString::number(off.y());
            photo.setTag(TAG_POINTS, points);
            outputPush(0, photo);
            emitProgress(i, s, 0, 1);
        }
        catch (std::exception &e) {
            setError(photo, e.what());
        }
    }
    delete needle;
    if ( aborted() )
        emitFailure();
    else
        emitSuccess();
    return false;
}
