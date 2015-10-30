#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QGraphicsItem>

#include "process.h"
#include "processscene.h"
#include "processnode.h"
#include "processbutton.h"
#include "processport.h"
#include "processconnection.h"
#include "processdropdown.h"
#include "processfilescollection.h"

#include "operatorloadraw.h"
#include "operatorexnihilo.h"
#include "operatorpassthrough.h"

Process::Process(ProcessScene *scene, QObject *parent) :
    QObject(parent),
    m_projectName(),
    m_notes(),
    m_outputDirectory(),
    m_temporaryDirectory(),
    m_scene(scene),
    m_dirty(false),
    m_availableOperators(),
    m_lastMousePosition(),
    m_conn(NULL)
{
    reset();
    connect(m_scene, SIGNAL(contextMenuSignal(QGraphicsSceneContextMenuEvent*)),
            this, SLOT(contextMenuSignal(QGraphicsSceneContextMenuEvent*)));
    m_scene->installEventFilter(this);

    m_availableOperators.push_back(new OperatorLoadRaw(this));
    m_availableOperators.push_back(new OperatorExNihilo(this));
    m_availableOperators.push_back(new OperatorPassThrough(this));
}


Process::~Process() {
    foreach(Operator *op, m_availableOperators) {
        delete op;
    }

    disconnect(m_scene, SIGNAL(contextMenuSignal(QGraphicsSceneContextMenuEvent*)),
            this, SLOT(contextMenuSignal(QGraphicsSceneContextMenuEvent*)));

}

QString Process::projectName() const
{
    return m_projectName;
}

void Process::setProjectName(const QString &projectName)
{
    if ( m_projectName != projectName ) {
        m_projectName = projectName;
        setDirty(true);
    }
}

QString Process::notes() const
{
    return m_notes;
}

void Process::setNotes(const QString &notes)
{
    if ( m_notes != notes ) {
        m_notes = notes;
     setDirty(true);
    }
}

QString Process::outputDirectory() const
{
    return m_outputDirectory;
}

void Process::setOutputDirectory(const QString &outputDirectory)
{
    if ( m_outputDirectory != outputDirectory ) {
        m_outputDirectory = outputDirectory;
        setDirty(true);
    }
}

QString Process::temporaryDirectory() const
{
    return m_temporaryDirectory;
}

void Process::setTemporaryDirectory(const QString &temporaryDirectory)
{
    if ( m_temporaryDirectory != temporaryDirectory ) {
        m_temporaryDirectory = temporaryDirectory;
        setDirty(true);
    }
}

bool Process::dirty() const
{
    return m_dirty;
}

void Process::setDirty(bool dirty)
{
    m_dirty = dirty;
    emit stateChanged();
}

void Process::addOperator(Operator *op)
{
    ProcessNode *node = new ProcessNode(m_lastMousePosition,
                                        op, this);
    this->m_nodes.insert(node);
    m_scene->addItem(node);
    setDirty(true);
}

QString Process::projectFile() const
{
    return m_projectFile;
}

void Process::setProjectFile(const QString &projectFile)
{
    m_projectFile = projectFile;
}

void Process::save()
{
    QJsonObject obj;
    QJsonDocument doc;
    obj["projectName"]=projectName();
    obj["notes"]=notes();
    obj["outputDirectory"]=outputDirectory();
    obj["temporaryDirectory"]=temporaryDirectory();
    doc.setObject(obj);
    QFile saveFile(projectFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
           qWarning("Couldn't open save file.");
       return;
    }

    saveFile.write(doc.toBinaryData());
    setDirty(false);
}

void Process::open(const QString& filename)
{
    QFile loadFile(filename);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
            qWarning("Couldn't open load file.");
            return;
    }
    QByteArray data = loadFile.readAll();
    QJsonDocument doc(QJsonDocument::fromBinaryData(data));
    QJsonObject obj = doc.object();
    setProjectFile(filename);
    setProjectName(obj["projectName"].toString());
    setNotes(obj["notes"].toString());
    setOutputDirectory(obj["outputDirectory"].toString());
    setTemporaryDirectory(obj["temporaryDirectory"].toString());
    setDirty(false);
}
QGraphicsItem* Process::findItem(const QPointF &pos, int type)
{
    QList<QGraphicsItem*> items = m_scene->items(QRectF(pos - QPointF(1,1), QSize(3,3)));

    foreach(QGraphicsItem *item, items)
        if (item->type() == type &&
                ( !item->isObscured(QRectF(pos,QSize(1,1))) ||
                  QGraphicsItem::UserType + ProcessScene::UserTypeConnection == type ))
            return item;
    return 0;
}

void Process::resetAllButtonsBut(QGraphicsItem *exepted)
{
    QList<QGraphicsItem*> items = m_scene->items();
    foreach(QGraphicsItem *item, items)
        if (item->type() == QGraphicsItem::UserType + ProcessScene::UserTypeButton &&
                item != exepted)
            dynamic_cast<ProcessButton*>(item)->resetMouse();
}

bool Process::eventFilter(QObject *obj, QEvent *event)
{
    QGraphicsSceneMouseEvent *me =
            dynamic_cast<QGraphicsSceneMouseEvent*>(event);
    if ( NULL == me )
    {
        return QObject::eventFilter(obj, event);
    }
    m_lastMousePosition = me->scenePos();
    QEvent::Type type = event->type();
    QGraphicsItem *button = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeButton);
    QGraphicsItem *portItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypePort);
    QGraphicsItem *connItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeConnection);
    QGraphicsItem *dropdownItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeDropDown);
    QGraphicsItem *filesCollectionItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeFilesCollection);
    switch (type)
    {
    case QEvent::GraphicsSceneMousePress:
        if (button) {
            dynamic_cast<ProcessButton*>(button)->mousePress();
            button->update();
            event->accept();
            return true;
        }
        if (portItem && NULL == m_conn) {
            ProcessPort *port = dynamic_cast<ProcessPort*>(portItem);
            if ( ProcessPort::OutputPort == port->portType()) {
                m_conn = new ProcessConnection(port, this);
                m_scene->addItem(m_conn);
            }
        }
        //portItem in the cond permit to move conn only from inPort
        if (connItem && portItem && NULL == m_conn) {
            m_conn = dynamic_cast<ProcessConnection*>(connItem);
            m_conn->unsetInputPort();
            m_conn->updateDanglingPath(me->scenePos());
        }
        if (dropdownItem) {
            dynamic_cast<ProcessDropDown*>(dropdownItem)->clicked(me->screenPos());
            event->accept();
            return true;
        }
        if (filesCollectionItem) {
            dynamic_cast<ProcessFilesCollection*>(filesCollectionItem)->clicked(me->screenPos());
            event->accept();
            return true;
        }
        break;
    case QEvent::GraphicsSceneMouseRelease:
        if (button) {
            dynamic_cast<ProcessButton*>(button)->mouseRelease();
            resetAllButtonsBut(button);
            button->update();
            event->accept();
            return true;
        }
        if (m_conn) {
            if ( portItem ) {
                ProcessPort *port = dynamic_cast<ProcessPort*>(portItem);
                if ( ProcessPort::InputOnePort == port->portType() ||
                     ProcessPort::InputPort == port->portType() )
                    m_conn->setInputPort(port);
                else
                    delete m_conn;
            }
            else {
                delete m_conn;
            }
            m_conn = NULL;
        }
        resetAllButtonsBut();
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
        spawnContextMenu(me->screenPos());
        break;
    case QEvent::GraphicsSceneMouseMove: {
        QGraphicsSceneMouseEvent *me = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        if (m_conn)
        {
            m_conn->updateDanglingPath(me->scenePos());
            return true;
        }
        break;
    }
    default:break;
    }

/*
 *     qWarning("it's a mouse event");
    m_scene->addRect(me->scenePos().x()-5,
                     me->scenePos().y()-5,
                     10,
                     10);
*/
    return QObject::eventFilter(obj, event);

}

void Process::reset()
{
    setProjectName("");
    setNotes("");
    setProjectFile("");
    setOutputDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    setTemporaryDirectory(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    m_scene->clear();
    setDirty(true);
}
void Process::spawnContextMenu(const QPoint& pos)
{
    QMenu menu;
    foreach(Operator *op, m_availableOperators) {
        menu.addAction(QIcon(), op->getClassIdentifier(),op,SLOT(clone()));
    }
    menu.exec(pos);
}

void Process::contextMenuSignal(QGraphicsSceneContextMenuEvent *event)
{
    spawnContextMenu(event->screenPos());
}
ProcessScene *Process::scene() const
{
    return m_scene;
}

