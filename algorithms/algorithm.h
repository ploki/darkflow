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

class Algorithm : public QObject
{
    Q_OBJECT
public:
    explicit Algorithm(QObject *parent = 0);

    virtual Photo apply(const Photo& source);
    virtual void applyOn(Photo& photo);

signals:

public slots:
};

#endif // MANIPULATION_H
