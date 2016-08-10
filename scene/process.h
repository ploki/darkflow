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
#ifndef PROCESS_H
#define PROCESS_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QSet>
#include <QPointF>
#include <QPoint>
#include <QPalette>

class ProcessScene;
class QGraphicsSceneContextMenuEvent;
class QEvent;
class Operator;
class ProcessNode;
class ProcessConnection;
class QGraphicsItem;
class QMenu;

#define PEN_WIDTH 0
#define MARGIN 2

QColor processColor(QPalette::ColorRole role);

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
    QPoint m_lastScreenPosition;
    ProcessConnection *m_conn;
    QMenu *m_contextMenu;


    QGraphicsItem* findItem(const QPointF &pos, int type);
    void resetAllButtonsBut(QGraphicsItem*item=0);
    bool eventFilter(QObject *obj, QEvent *event);
    void addOperatorsToContextMenu();
};

#endif // PROCESS_H
