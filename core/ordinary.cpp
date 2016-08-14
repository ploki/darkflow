#include <QMutexLocker>
#include "ordinary.h"

#if !defined(_OPENMP)

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
