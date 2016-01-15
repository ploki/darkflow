#ifndef SELECTIVELAB_H
#define SELECTIVELAB_H

#include <QDialog>

namespace Ui {
class SelectiveLab;
}

class Photo;
class Operator;

class SelectiveLab : public QDialog
{
    Q_OBJECT

public:
    explicit SelectiveLab(const QString& windowCaption,
                          int hue,
                          int coverage,
                          bool strict,
                          int level,
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

    bool displayGuide() const;
    void setDisplayGuide(bool v);

    bool previewEffect() const;
    void setPreviewEffect(bool v);

    void loadValues(int hue,
                    int coverage,
                    bool strict,
                    int level,
                    bool displayGuide,
                    bool preview);

public slots:
    void updateView();
    void updateViewNoEmission();

signals:
    void updated();

protected:
    Photo createPhoto(int level);
    void applyPreview(Photo& photo);
    void drawGuide(Photo& photo, int hue, int coverage, bool strict);

private:
    Ui::SelectiveLab *ui;
    const Operator *m_operator;
};

#endif // SELECTIVELAB_H
