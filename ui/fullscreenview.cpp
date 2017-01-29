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
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QKeyEvent>

#include <cmath>
#include "graphicsviewinteraction.h"
#include "fullscreenview.h"
#include "ui_fullscreenview.h"
#include "darkflow.h"

FullScreenView::FullScreenView(QGraphicsScene *scene, QWidget *parent) :
    QMainWindow(parent),
    m_scene(scene),
    ui(new Ui::FullScreenView),
    graphicsViewInteraction(0)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(DF_ICON));
#if defined(Q_OS_OSX)
    /* Expected to be fullscreen on other platforms */
    setWindowFlags(Qt::Tool);
#elif !defined(DF_WINDOWS)
    /* windows with parent set no longer stay on front
     * of there parent. It seems that window handling is fragile
     * over time...
     */
    setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
#endif
    ui->graphicsView->setScene(m_scene);
    graphicsViewInteraction = new GraphicsViewInteraction(ui->graphicsView, this);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

bool FullScreenView::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() == QEvent::KeyRelease ) {
        int key = dynamic_cast<QKeyEvent*>(event)->key();
        if ( key == Qt::Key_Escape ) {
            hide();
            event->accept();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


