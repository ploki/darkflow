#ifndef PROCESS_H
#define PROCESS_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QSet>
#include <QPointF>
#include <QPoint>

class ProcessScene;
class QGraphicsSceneContextMenuEvent;
class QEvent;
class Operator;
class ProcessNode;
class ProcessConnection;
class QGraphicsItem;
class QMenu;


class Process : public QObject
{
    Q_OBJECT
public:

    static QString uuid();


    explicit Process(ProcessScene *scene, QObject *parent = 0);
    ~Process();

    QString projectName() const;
    void setProjectName(const QString &projectName);

    QString projectFile() const;
    void setProjectFile(const QString &projectFile);

    QString notes() const;
    void setNotes(const QString &notes);

    QString baseDirectory() const;
    void setBaseDirectory(const QString &outputDirectory);

    void reset();
    void save();
    void load(const QString& filename);


    bool dirty() const;
    void setDirty(bool dirty);

    void addOperator(Operator *op);
    void spawnContextMenu(const QPoint& pos);

    ProcessNode *findNode(const QString& uuid);

    ProcessScene *scene() const;

signals:
    void stateChanged();

public slots:

private slots:
    void contextMenuSignal(QGraphicsSceneContextMenuEvent *);

private:
    Q_DISABLE_COPY(Process);
    QString m_projectName;
    QString m_projectFile;
    QString m_notes;
    QString m_baseDirectory;
    ProcessScene *m_scene;
    bool m_dirty;
    QVector<Operator*> m_availableOperators;
    QPointF m_lastMousePosition;
    ProcessConnection *m_conn;
    QMenu *m_contextMenu;


    QGraphicsItem* findItem(const QPointF &pos, int type);
    void resetAllButtonsBut(QGraphicsItem*item=0);
    bool eventFilter(QObject *obj, QEvent *event);
    void addOperatorsToContextMenu();
};

#endif // PROCESS_H
