#ifndef OPERATORPARAMETERDROPDOWN_H
#define OPERATORPARAMETERDROPDOWN_H

#include <QMap>
#include <QString>
#include <QPoint>

#include "operatorparameter.h"

#define DF_TR_AND_C(str) (str), tr(str)

class QObject;
class QMenu;
class QAction;

class DropDownPair {
public:
    DropDownPair();
    DropDownPair(const char *str, int v);
    QString C_option;
    int value;
};

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

    void addOption(const char *option, const QString& optionLocalized, int value, bool selected=false);
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
    QMap<QString, DropDownPair> m_options;
    QString m_currentValue;
};

#endif // OPERATORPARAMETERDROPDOWN_H
