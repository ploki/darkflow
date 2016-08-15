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
