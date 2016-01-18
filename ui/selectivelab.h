#ifndef SELECTIVELAB_H
#define SELECTIVELAB_H

#include <QDialog>
#include <QRunnable>
#include <QMutex>

namespace Ui {
class SelectiveLab;
}

class Photo;
class Operator;
class QPixmap;

class SelectiveLab : public QDialog, public QRunnable
{
    Q_OBJECT

public:
    explicit SelectiveLab(const QString& windowCaption,
                          int hue,
                          int coverage,
                          bool strict,
                          int level,
                          bool clipToGamut,
                          bool displayGuide,
                          bool preview,
                          const Operator *op,
                          QWidget *parent = 0);
    ~SelectiveLab();

    int hue() const;
    void setHue(int v);

    int coverage() const;
    void setCoverage(int v);

    bool strict() const;
    void setStrict(bool v);

    int level() const;
    void setLevel(int v);

    bool clipToGamut() const;
    void setClipToGamut(bool clipToGamut);

    bool displayGuide() const;
    void setDisplayGuide(bool v);

    bool previewEffect() const;
    void setPreviewEffect(bool v);

    void loadValues(int hue,
                    int coverage,
                    bool strict,
                    int level,
                    bool clipToValue,
                    bool displayGuide,
                    bool preview);
    void getDialogValues();
    void run();

public slots:
    void updateView();
    void updateViewNoEmission();
    void updatePixmap();

signals:
    void updated();
    void finishedPixmap();

protected:
    Photo createPhoto(int level, bool clipToGamut);
    void applyPreview(Photo& photo);
    void drawGuide(Photo& photo, int hue, int coverage, bool strict);

private:
    Ui::SelectiveLab *ui;
    const Operator *m_operator;

    int m_labSelectionSize;
    bool m_updated;
    int m_hue;
    int m_coverage;
    int m_level;
    bool m_strict;
    bool m_clipToGamut;
    bool m_guide;
    bool m_preview;
    bool m_running;
    QPixmap m_pixmap;
    QMutex m_mutex;

};

#endif // SELECTIVELAB_H
