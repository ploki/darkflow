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
#ifndef PIXELS_H
#define PIXELS_H

#if defined(_OPENMP)
# define Ordinary Magick
#else
# include <Magick++.h>
# include <QMap>
# include <QMutex>
# include <pthread.h>
# include <memory>

/*
 * I'm sorry to inform you that image magick doesn't support access
 * from multiple threads outside OpenMP. by the way, contexts are
 * indexed by openmp threads ids. don't exceed this limit
 */
namespace Ordinary {

class Pixels
{
    Magick::Image *m_image;
    QMap<pthread_t, std::shared_ptr<Magick::Pixels > > m_pixels;
    QMutex m_mutex;
public:
    Pixels(Magick::Image& image);
    ~Pixels(void);
    Magick::PixelPacket *get(const ::ssize_t x_, const ::ssize_t y_,
      const size_t columns_,const  size_t rows_ );
    const Magick::PixelPacket *getConst(const ::ssize_t x_,const ::ssize_t y_,
      const size_t columns_,const size_t rows_);
    void sync(void);

private:
    Pixels(const Pixels&);
};
}
#endif
#endif // PIXELS_H
