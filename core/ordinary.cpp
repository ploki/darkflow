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
#include <QMutexLocker>
#include "ordinary.h"

#if !defined(_OPENMP) || defined(DF_WINDOWS)

Ordinary::Pixels::Pixels(Magick::Image &image) :
    m_image(&image),
    m_pixels(),
    m_mutex()
{
}

Ordinary::Pixels::~Pixels()
{
}

Magick::PixelPacket *Ordinary::Pixels::get(const ssize_t x_, const ssize_t y_, const size_t columns_, const size_t rows_)
{
    pthread_t self = pthread_self();
    m_mutex.lock();
    std::shared_ptr<Magick::Pixels>& pixels = m_pixels[self];
    m_mutex.unlock();
    if (!pixels) {
        pixels.reset(new Magick::Pixels(*m_image));
    }
    return pixels->get(x_, y_, columns_, rows_);
}

const Magick::PixelPacket *Ordinary::Pixels::getConst(const ssize_t x_, const ssize_t y_, const size_t columns_, const size_t rows_)
{
    pthread_t self = pthread_self();
    m_mutex.lock();
    std::shared_ptr<Magick::Pixels>& pixels = m_pixels[self];
    m_mutex.unlock();
    if (!pixels) {
        pixels.reset(new Magick::Pixels(*m_image));
    }
    return pixels->getConst(x_, y_, columns_, rows_);
}

void Ordinary::Pixels::sync()
{
    pthread_t self = pthread_self();
    m_mutex.lock();
    std::shared_ptr<Magick::Pixels>& pixels = m_pixels[self];
    m_mutex.unlock();
    if (pixels) {
        pixels->sync();
        //pixels.reset();
    }
}

#endif
