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
#ifndef PROCESSNODE_H
#define PROCESSNODE_H

#include <QGraphicsPathItem>
#include <QPointF>
#include <QSet>
#include <QJsonObject>
#include <QVector>

#include "processscene.h"

class Process;
class ProcessConnection;
class ProcessPort;
class Operator;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class OperatorInput;
class OperatorOutput;
class OperatorParameter;
class Visualization;

class ProcessNode : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:

    enum { Type = QGraphicsItem::UserType + ProcessScene::UserTypeNode };

    explicit ProcessNode(QPointF pos,
                         Operator *op,
                         Process *process,
                         QGraphicsItem *parent = 0);
    ~ProcessNode();
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    int type() const;
    void addConnection(ProcessConnection *connection);
    void removeConnection(ProcessConnection *connection);
    QJsonObject save();

    ProcessPort *inPort(int idx);
    ProcessPort *outPort(int idx);

signals:

private slots:
    void operatorStateChanged();
    void closeButtonClicked(QPoint screenPos);
    void helpClicked(QPoint screenPos);
    void visualizationClicked(QPoint screenPos);
    void playClicked(QPoint screenPos);
    void abortClicked(QPoint screenPos);
    void refreshClicked(QPoint screenPos);
    void operatorNameChanged(QString text);

public:
    Operator *m_operator;
private:
    Process *m_process;
    double m_completion;
    bool m_enabled;
    QGraphicsTextItem *m_caption;
    QSet<ProcessConnection*> m_connections;
    QVector<ProcessPort*> m_inPorts;
    QVector<ProcessPort*> m_outPorts;
    Visualization *m_visualization;

    void addButtons(qreal size);
    void addPorts(QVector<OperatorOutput*>& outputs,
                  QVector<OperatorInput*>& inputs,
                  qreal size);
    void addParameters(QVector<OperatorParameter*>& parameters, qreal size, qreal offset);
};

#endif // PROCESSNODE_H
