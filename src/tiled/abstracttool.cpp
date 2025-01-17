/*
 * abstracttool.cpp
 * Copyright 2009-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "abstracttool.h"

#include "mapdocument.h"
#include "toolmanager.h"

#include <QKeyEvent>

namespace Tiled {

AbstractTool::AbstractTool(Id id,
                           const QString &name,
                           const QIcon &icon,
                           const QKeySequence &shortcut,
                           QObject *parent)
    : QObject(parent)
    , mId(id)
    , mName(name)
    , mIcon(icon)
    , mShortcut(shortcut)
    , mEnabled(false)
    , mToolManager(nullptr)
    , mMapDocument(nullptr)
{
}

void AbstractTool::setName(const QString &name)
{
    if (mName == name)
        return;

    mName = name;
    emit changed();
}

void AbstractTool::setIcon(const QIcon &icon)
{
    mIcon = icon;
    emit changed();
}

void AbstractTool::setShortcut(const QKeySequence &shortcut)
{
    if (mShortcut == shortcut)
        return;

    mShortcut = shortcut;
    emit changed();
}

/**
 * Sets the current status information for this tool. This information will be
 * displayed in the status bar.
 */
void AbstractTool::setStatusInfo(const QString &statusInfo)
{
    if (mStatusInfo != statusInfo) {
        mStatusInfo = statusInfo;
        emit statusInfoChanged(mStatusInfo);
    }
}

/**
 * Sets the cursor used by this tool. This will be the cursor set on the
 * viewport of the MapView while the tool is active.
 */
void AbstractTool::setCursor(const QCursor &cursor)
{
    mCursor = cursor;
    emit cursorChanged(cursor);
}

void AbstractTool::setEnabled(bool enabled)
{
    if (mEnabled == enabled)
        return;

    mEnabled = enabled;
    emit enabledChanged(enabled);
}

Tile *AbstractTool::tile() const
{
    return toolManager()->tile();
}

ObjectTemplate *AbstractTool::objectTemplate() const
{
    return toolManager()->objectTemplate();
}

void AbstractTool::keyPressed(QKeyEvent *event)
{
    event->ignore();
}

void AbstractTool::mouseDoubleClicked(QGraphicsSceneMouseEvent *event)
{
    mousePressed(event);
}

void AbstractTool::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument) {
        disconnect(mMapDocument, &MapDocument::changed,
                   this, &AbstractTool::changeEvent);
        disconnect(mMapDocument, &MapDocument::currentLayerChanged,
                   this, &AbstractTool::updateEnabledState);
    }

    MapDocument *oldDocument = mMapDocument;
    mMapDocument = mapDocument;
    mapDocumentChanged(oldDocument, mMapDocument);

    if (mMapDocument) {
        connect(mMapDocument, &MapDocument::changed,
                this, &AbstractTool::changeEvent);
        connect(mMapDocument, &MapDocument::currentLayerChanged,
                this, &AbstractTool::updateEnabledState);
    }
    updateEnabledState();
}

void AbstractTool::changeEvent(const ChangeEvent &event)
{
    switch (event.type) {
    case ChangeEvent::LayerChanged:
        // Enabled state is not actually affected by layer properties, but
        // this includes updating brush visibility...
        updateEnabledState();
        break;
    default:
        break;
    }
}

void AbstractTool::updateEnabledState()
{
    setEnabled(mMapDocument != nullptr);
}

Layer *AbstractTool::currentLayer() const
{
    return mMapDocument ? mMapDocument->currentLayer() : nullptr;
}

} // namespace Tiled
