#ifndef OPERATORPARAMETERDROPDOWN_H
#define OPERATORPARAMETERDROPDOWN_H

#include <QMap>
#include <QString>
#include <QPoint>

#include "operatorparameter.h"

class QObject;
class QMenu;
class QAction;

class OperatorParameterDropDown : public OperatorParameter
{
    Q_OBJECT
public:
    OperatorParameterDropDown(
            const QString& name,
            const QString& caption,
            Operator *op,
            const char *slot);
    ~OperatorParameterDropDown();

    void addOption(const QString& option, int value, bool selected=false);
    void dropDown(const QPoint& pos);

    QString currentValue() const;

    QJsonObject save();
    void load(const QJsonObject &obj);

signals:
    /* used to notify ProcessDropDown */
    void valueChanged(const QString& value);

    /* used to notify Operator */
    void itemSelected(int v);

private slots:
    /* called by QMenu */
    void actionTriggered(QAction *action);

private:
    QMenu *m_menu;
    QMap<QString, int> m_options;
    QString m_currentValue;
};

#endif // OPERATORPARAMETERDROPDOWN_H
