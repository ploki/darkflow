/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include <QApplication>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QGraphicsItem>
#include <QUuid>
#include <QMap>


#include "process.h"
#include "processscene.h"
#include "processnode.h"
#include "processbutton.h"
#include "processport.h"
#include "processconnection.h"
#include "processdropdown.h"
#include "processfilescollection.h"
#include "processslider.h"
#include "processselectivelab.h"
#include "processdirectory.h"

#include "console.h"

#include "oploadraw.h"
#include "opexnihilo.h"
#include "oppassthrough.h"
#include "oprotate.h"
#include "opwhitebalance.h"
#include "opexposure.h"
#include "opmodulate.h"
#include "opigamma.h"
#include "opdesaturateshadows.h"
#include "opshapedynamicrange.h"
#include "opsubtract.h"
#include "opblackbody.h"
#include "opflatfieldcorrection.h"
#include "opintegration.h"
#include "opinvert.h"
#include "opcrop.h"
#ifdef HAVE_FFMPEG
# include "oploadvideo.h"
#endif
#include "opblend.h"
#include "opmultiplexer.h"
#include "opdemultiplexer.h"
#include "oprgbdecompose.h"
#include "oprgbcompose.h"
#include "opcmydecompose.h"
#include "opcmycompose.h"
#include "opequalize.h"
#include "opchannelmixer.h"
#include "opcolorfilter.h"
#include "opmicrocontrasts.h"
#include "opunsharpmask.h"
#include "opgaussianblur.h"
#include "opblur.h"
#include "opthreshold.h"
#include "opdeconvolution.h"
#include "opdebayer.h"
#include "oploadimage.h"
#include "opconvolution.h"
#include "oproll.h"
#include "opscale.h"
#include "opssdreg.h"
#include "opbracketing.h"
#include "opgradientevaluation.h"
#include "oplevel.h"
#include "oplevelpercentile.h"
#include "opflip.h"
#include "opflop.h"
#include "openhance.h"
#include "opdespeckle.h"
#include "opnormalize.h"
#include "opadaptivethreshold.h"
#include "opreducenoise.h"
#include "ophotpixels.h"
#include "opcolor.h"
#include "ophdr.h"
#include "opselectivelabfilter.h"
#include "opsave.h"
#include "opairydisk.h"
#include "opwienerdeconvolution.h"
#include "preferences.h"

QString Process::uuid()
{
    QUuid uuid = uuid.createUuid();
    return uuid.toString();

}

Process::Process(ProcessScene *scene, QObject *parent) :
    QObject(parent),
    m_projectName(),
    m_notes(),
    m_baseDirectory(),
    m_scene(scene),
    m_dirty(false),
    m_availableOperators(),
    m_lastMousePosition(),
    m_lastScreenPosition(),
    m_conn(NULL),
    m_contextMenu(new QMenu)
{
    reset();
    connect(m_scene, SIGNAL(contextMenuSignal(QGraphicsSceneContextMenuEvent*)),
            this, SLOT(contextMenuSignal(QGraphicsSceneContextMenuEvent*)));
    m_scene->installEventFilter(this);

    m_availableOperators.push_back(new OpLoadRaw(this));
    m_availableOperators.push_back(new OpLoadImage(this));
#ifdef HAVE_FFMPEG
    m_availableOperators.push_back(new OpLoadVideo(this));
#endif
    m_availableOperators.push_back(new OpSave(this));

    m_availableOperators.push_back(new OpExposure(this));
    m_availableOperators.push_back(new OpShapeDynamicRange(this));
    m_availableOperators.push_back(new OpIGamma(this));
    m_availableOperators.push_back(new OpHDR(this));
    m_availableOperators.push_back(new OpLevel(this));
    m_availableOperators.push_back(new OpLevelPercentile(this));

    m_availableOperators.push_back(new OpWhiteBalance(this));
    m_availableOperators.push_back(new OpBlackBody(this));
    m_availableOperators.push_back(new OpColor(this));
    m_availableOperators.push_back(new OpDebayer(this));
    m_availableOperators.push_back(new OpRGBDecompose(this));
    m_availableOperators.push_back(new OpRGBCompose(this));
    m_availableOperators.push_back(new OpCMYDecompose(this));
    m_availableOperators.push_back(new OpCMYCompose(this));
    m_availableOperators.push_back(new OpColorFilter(this));
    m_availableOperators.push_back(new OpChannelMixer(this));
    m_availableOperators.push_back(new OpThreshold(this));
    m_availableOperators.push_back(new OpAdaptiveThreshold(this));
    m_availableOperators.push_back(new OpNormalize(this));
    m_availableOperators.push_back(new OpEqualize(this));
    m_availableOperators.push_back(new OpGradientEvaluation(this));

    m_availableOperators.push_back(new OpModulate(this));
    m_availableOperators.push_back(new OpDesaturateShadows(this));
    m_availableOperators.push_back(new OpSelectiveLabFilter(this));
    m_availableOperators.push_back(new OpMicroContrasts(this));
    m_availableOperators.push_back(new OpUnsharpMask(this));
    m_availableOperators.push_back(new OpEnhance(this));
    m_availableOperators.push_back(new OpDespeckle(this));
    m_availableOperators.push_back(new OpReduceNoise(this));
    m_availableOperators.push_back(new OpHotPixels(this));
    m_availableOperators.push_back(new OpDeconvolution(this));
    m_availableOperators.push_back(new OpWienerDeconvolution(this));

    m_availableOperators.push_back(new OpInvert(this));
    m_availableOperators.push_back(new OpBlur(this));
    m_availableOperators.push_back(new OpGaussianBlur(this));
    m_availableOperators.push_back(new OpConvolution(this));

    m_availableOperators.push_back(new OpBlend(this));
    m_availableOperators.push_back(new OpIntegration(this));
    m_availableOperators.push_back(new OpFlatFieldCorrection(this));
    m_availableOperators.push_back(new OpBracketing(this));

    m_availableOperators.push_back(new OpCrop(this));
    m_availableOperators.push_back(new OpFlip(this));
    m_availableOperators.push_back(new OpFlop(this));
    m_availableOperators.push_back(new OpRotate(this));
    m_availableOperators.push_back(new OpScale(this));
    m_availableOperators.push_back(new OpRoll(this));

    m_availableOperators.push_back(new OpSsdReg(this));

    m_availableOperators.push_back(new OpDemultiplexer(2, this));
    m_availableOperators.push_back(new OpDemultiplexer(3, this));
    m_availableOperators.push_back(new OpDemultiplexer(4, this));
    m_availableOperators.push_back(new OpDemultiplexer(5, this));
    m_availableOperators.push_back(new OpDemultiplexer(6, this));
    m_availableOperators.push_back(new OpMultiplexer(2, this));
    m_availableOperators.push_back(new OpMultiplexer(3, this));
    m_availableOperators.push_back(new OpMultiplexer(4, this));
    m_availableOperators.push_back(new OpMultiplexer(5, this));
    m_availableOperators.push_back(new OpMultiplexer(6, this));


    m_availableOperators.push_back(new OpSubtract(this));
    m_availableOperators.push_back(new OpExNihilo(this));
    m_availableOperators.push_back(new OpPassThrough(this));
    addOperatorsToContextMenu();
}

void Process::addOperatorsToContextMenu() {
    QMap<QString, QMenu*> sections;
    QMenu *all = new QMenu("< all >");
    foreach(Operator *op, m_availableOperators) {
        if ( sections.find(op->getClassSection()) == sections.end() ) {
            sections[op->getClassSection()] = m_contextMenu->addMenu(QIcon(), op->getClassSection());
        }
        QString caption = op->getName() + " (";
        if ( op->isCompatible(Operator::Linear) )
            caption+="L";
        else
            caption+=".";
        if ( op->isCompatible(Operator::NonLinear) )
            caption+="N";
        else
            caption+=".";
        if ( op->isCompatible(Operator::HDR) )
            caption += "H";
        else
            caption+=".";
        caption += ")";

        sections[op->getClassSection()]->addAction(QIcon(), caption, op, SLOT(clone()));
        all->addAction(QIcon(), caption, op, SLOT(clone()));
    }
    m_contextMenu->addMenu(all);
}

Process::~Process() {
    foreach(Operator *op, m_availableOperators) {
        delete op;
    }

    disconnect(m_scene, SIGNAL(contextMenuSignal(QGraphicsSceneContextMenuEvent*)),
            this, SLOT(contextMenuSignal(QGraphicsSceneContextMenuEvent*)));
    delete m_contextMenu;
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

QString Process::baseDirectory() const
{
    return m_baseDirectory;
}

void Process::setBaseDirectory(const QString &baseDirectory)
{
    if ( m_baseDirectory != baseDirectory ) {
        m_baseDirectory = baseDirectory;
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
    QDir projectFileDir(QFileInfo(projectFile()).absoluteDir());
    QJsonObject obj;
    QJsonArray nodes;
    QJsonArray connections;
    QJsonDocument doc;
    obj["projectName"]=projectName();
    obj["notes"]=notes();
    obj["baseDirectory"]=projectFileDir.relativeFilePath(baseDirectory());
    foreach (QGraphicsItem *item, m_scene->items()) {
        if ( item->type() == QGraphicsItem::UserType + ProcessScene::UserTypeNode ) {
            ProcessNode *node = dynamic_cast<ProcessNode *>(item);
            dflDebug(tr("Process: saving a node"));
            nodes.push_back(node->save(baseDirectory()));
        }
        else if ( item->type() == QGraphicsItem::UserType + ProcessScene::UserTypeConnection ) {
            ProcessConnection *conn = dynamic_cast<ProcessConnection *>(item);
            connections.push_back(conn->save());
        }
    }
    obj["nodes"] = nodes;
    obj["connections"] = connections;

    doc.setObject(obj);
    QFile saveFile(projectFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
           dflWarning(tr("Process: Couldn't open save file."));
       return;
    }

    //saveFile.write(doc.toBinaryData());
    saveFile.write(doc.toJson());
    setDirty(false);
}

void Process::load(const QString& filename)
{
    QDir projectFileDir(QFileInfo(filename).absoluteDir());
    QFile loadFile(filename);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
            dflWarning(tr("Process::load: Couldn't open file %0").arg(filename));
            return;
    }
    QByteArray data = loadFile.readAll();
    //QJsonDocument doc(QJsonDocument::fromBinaryData(data));
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject obj = doc.object();
    setProjectFile(filename);
    setProjectName(obj["projectName"].toString());
    setNotes(obj["notes"].toString());
    setBaseDirectory(projectFileDir.absoluteFilePath(obj["baseDirectory"].toString()));
    foreach(QJsonValue val, obj["nodes"].toArray()) {
        QJsonObject obj = val.toObject();
        bool operatorFound = false;
        foreach(Operator *op, m_availableOperators) {
            if ( op->getClassIdentifier() == obj["classIdentifier"].toString()) {
                qreal x = obj["x"].toDouble();
                qreal y = obj["y"].toDouble();
                operatorFound = true;
                m_lastMousePosition.setX(x);
                m_lastMousePosition.setY(y);
                Operator *newOp = op->newInstance();
                newOp->setName(obj["name"].toString());
                addOperator(newOp);
                newOp->load(obj);
                break;
            }
        }
        if (!operatorFound) {
            dflWarning(tr("Process: Unknown operator"));
        }
    }
    foreach(QJsonValue val, obj["connections"].toArray()) {
        QJsonObject obj = val.toObject();
        ProcessNode *outNode = findNode(obj["outPortUuid"].toString());
        if ( NULL == outNode ) {
            dflWarning(tr("Process: unknown output node"));
            continue;
        }
        int outIdx = obj["outPortIdx"].toInt();
        ProcessNode *inNode = findNode(obj["inPortUuid"].toString());
        if ( NULL == inNode ) {
            dflWarning(tr("Process: unknown input node"));
            continue;
        }
        int inIdx = obj["inPortIdx"].toInt();

        ProcessConnection *conn = new ProcessConnection(outNode->outPort(outIdx));
        conn->setInputPort(inNode->inPort(inIdx));
        m_scene->addItem(conn);
    }



    setDirty(false);
}

ProcessNode *Process::findNode(const QString &uuid)
{
    QList<QGraphicsItem*> items = m_scene->items();
    foreach(QGraphicsItem *item, items) {
        if ( item->type() == QGraphicsItem::UserType + ProcessScene::UserTypeNode ) {
            ProcessNode *node = dynamic_cast<ProcessNode *>(item);
            if ( node->m_operator->uuid() == uuid )
                return node;
        }
    }
    return NULL;
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
    QGraphicsItem *node = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeNode);
    QGraphicsItem *button = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeButton);
    QGraphicsItem *portItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypePort);
    QGraphicsItem *connItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeConnection);
    QGraphicsItem *dropdownItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeDropDown);
    QGraphicsItem *filesCollectionItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeFilesCollection);
    QGraphicsItem *sliderItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeSlider);
    QGraphicsItem *selectiveLab = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeSelectiveLab);
    QGraphicsItem *directory = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeDirectory);
    QGraphicsItem *progressItem = findItem(me->scenePos(), QGraphicsItem::UserType + ProcessScene::UserTypeProgressBar);
    switch (type)
    {
    case QEvent::GraphicsSceneMousePress:
        m_lastScreenPosition = me->screenPos();
        if (button) {
            dynamic_cast<ProcessButton*>(button)->mousePress();
            button->update();
            event->accept();
            return true;
        }
        if (portItem && NULL == m_conn) {
            ProcessPort *port = dynamic_cast<ProcessPort*>(portItem);
            if ( ProcessPort::OutputPort == port->portType()) {
                m_conn = new ProcessConnection(port);
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
        if (sliderItem) {
            dynamic_cast<ProcessSlider*>(sliderItem)->clicked(me->screenPos());
            event->accept();
            return true;
        }
        if (selectiveLab) {
            dynamic_cast<ProcessSelectiveLab*>(selectiveLab)->clicked(me->screenPos());
            event->accept();
            return true;
        }
        if (directory) {
            dynamic_cast<ProcessDirectory*>(directory)->clicked(me->screenPos());
            event->accept();
            return true;
        }
        break;
    case QEvent::GraphicsSceneMouseRelease:
        if(!m_conn && !button && !node && !sliderItem &&
                !portItem && !dropdownItem && !filesCollectionItem &&
                !selectiveLab && !progressItem &&
                m_lastScreenPosition.x() == me->screenPos().x() &&
                m_lastScreenPosition.y() == me->screenPos().y())
            spawnContextMenu(me->screenPos());
        if (button) {
            dynamic_cast<ProcessButton*>(button)->mouseRelease(me->screenPos());
            resetAllButtonsBut(button);
            button->update();
            event->accept();
            return true;
        }
        if (m_conn) {
            if ( portItem ) {
                ProcessPort *port = dynamic_cast<ProcessPort*>(portItem);
                if ( ProcessPort::InputOnePort == port->portType() ||
                     ProcessPort::InputPort == port->portType() ) {
                    bool connected = m_conn->setInputPort(port);
                    if ( !connected )
                        delete m_conn;
                }
                else {
                    delete m_conn;
                }
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
 *     dflDebug("Process: it's a mouse event");
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
    setBaseDirectory(preferences->baseDir());
    m_scene->clear();
    setDirty(true);
}
void Process::spawnContextMenu(const QPoint& pos)
{
    m_contextMenu->exec(pos);
}

void Process::contextMenuSignal(QGraphicsSceneContextMenuEvent *event)
{
    spawnContextMenu(event->screenPos());
}
ProcessScene *Process::scene() const
{
    return m_scene;
}
