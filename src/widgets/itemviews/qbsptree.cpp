/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbsptree_p.h"

QT_BEGIN_NAMESPACE

QBspTree::QBspTree() : depth(6), visited(0) {}

void QBspTree::create(int n, int d)
{
    // simple heuristics to find the best tree depth
    if (d == -1) {
        int c;
        for (c = 0; n; ++c)
            n = n / 10;
        depth = c << 1;
    } else {
        depth = d;
    }
    depth = qMax(depth, uint(1));

    nodes.resize((1 << depth) - 1); // resize to number of nodes
    leaves.resize(1 << depth); // resize to number of leaves
}

void QBspTree::destroy()
{
    leaves.clear();
    nodes.clear();
}

void QBspTree::climbTree(const QRect &rect, callback *function, QBspTreeData data)
{
    if (nodes.isEmpty())
        return;
    ++visited;
    climbTree(rect, function, data, 0);
}

void QBspTree::climbTree(const QRect &area, callback *function, QBspTreeData data, int index)
{
    if (index >= nodes.count()) { // the index points to a leaf
        Q_ASSERT(!nodes.isEmpty());
        function(leaf(index - nodes.count()), area, visited, data);
        return;
    }

    Node::Type t = (Node::Type) nodes.at(index).type;

    int pos = nodes.at(index).pos;
    int idx = firstChildIndex(index);
    if (t == Node::VerticalPlane) {
        if (area.left() < pos)
            climbTree(area, function, data, idx); // back
        if (area.right() >= pos)
            climbTree(area, function, data, idx + 1); // front
    } else {
        if (area.top() < pos)
            climbTree(area, function, data, idx); // back
        if (area.bottom() >= pos)
            climbTree(area, function, data, idx + 1); // front
    }
}

void QBspTree::init(const QRect &area, int depth, NodeType type, int index)
{
    Node::Type t = Node::None; // t should never have this value
    if (type == Node::Both) // if both planes are specified, use 2d bsp
        t = (depth & 1) ? Node::HorizontalPlane : Node::VerticalPlane;
    else
        t = type;
    QPoint center = area.center();
    nodes[index].pos = (t == Node::VerticalPlane ? center.x() : center.y());
    nodes[index].type = t;

    QRect front = area;
    QRect back = area;

    if (t == Node::VerticalPlane) {
        front.setLeft(center.x());
        back.setRight(center.x() - 1); // front includes the center
    } else { // t == Node::HorizontalPlane
        front.setTop(center.y());
        back.setBottom(center.y() - 1);
    }

    int idx = firstChildIndex(index);
    if (--depth) {
        init(back, depth, type, idx);
        init(front, depth, type, idx + 1);
    }
}

void QBspTree::insert(QVector<int> &leaf, const QRect &, uint, QBspTreeData data)
{
    leaf.append(data.i);
}

void QBspTree::remove(QVector<int> &leaf, const QRect &, uint, QBspTreeData data)
{
    int i = leaf.indexOf(data.i);
    if (i != -1)
        leaf.remove(i);
}

QT_END_NAMESPACE
