#ifndef MANIPULATION_H
#define MANIPULATION_H

#include <QObject>


template<typename t> t clamp(t v,t min = 0, t max = 65535 /* ARgg! */ ) {
  if ( v < min )
    return min;
  else if ( v > max )
    return max;
  else
    return v;
}

class Photo;
namespace Magick {
class Image;
}

class Algorithm : public QObject
{
    Q_OBJECT
public:
    explicit Algorithm(QObject *parent = 0);

    virtual Photo apply(const Photo& source);
    virtual void applyOnImage(Magick::Image& image);
    virtual void applyOn(Photo& photo);

signals:

public slots:
private:
    Q_DISABLE_COPY(Algorithm);
};

#endif // MANIPULATION_H
