/* poppler-optcontent-private.h: qt interface to poppler
 *
 * Copyright (C) 2007, Brad Hards <bradh@kde.org>
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef POPPLER_OPTCONTENT_PRIVATE_H
#define POPPLER_OPTCONTENT_PRIVATE_H

#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QString>

class Array;
class OCGs;
class OptionalContentGroup;

class QModelIndex;

namespace Poppler
{
  class OptContentItem;
  class OptContentModel;
  class OptContentModelPrivate;

  class RadioButtonGroup
  {
  public:
    RadioButtonGroup( OptContentModelPrivate *ocModel, Array *rbarray);
    ~RadioButtonGroup();
    QSet<OptContentItem *> setItemOn( OptContentItem *itemToSetOn );

  private:
    QList<OptContentItem*> itemsInGroup;
  };

  class OptContentItem
  {
    public:
    enum ItemState { On, Off, HeadingOnly };

    OptContentItem( OptionalContentGroup *group );
    OptContentItem( const QString &label );
    OptContentItem();
    ~OptContentItem();

    QString name() const { return m_name; }
    ItemState state() const { return m_stateBackup; }
    bool setState(ItemState state, QSet<OptContentItem *> &changedItems);

    QList<OptContentItem*> childList() { return m_children; }

    void setParent( OptContentItem* parent) { m_parent = parent; }
    OptContentItem* parent() { return m_parent; }

    void addChild( OptContentItem *child );

    void appendRBGroup( RadioButtonGroup *rbgroup );

    bool isEnabled() const { return m_enabled; }

    QSet<OptContentItem*> recurseListChildren(bool includeMe = false) const;

    private:
    OptionalContentGroup *m_group;
    QString m_name;
    ItemState m_state; // true for ON, false for OFF
    ItemState m_stateBackup;
    QList<OptContentItem*> m_children;
    OptContentItem *m_parent;
    QList<RadioButtonGroup*> m_rbGroups;
    bool m_enabled;
  };

  class OptContentModelPrivate
  {
    public:
    OptContentModelPrivate( OptContentModel *qq, OCGs *optContent );
    ~OptContentModelPrivate();

    void parseRBGroupsArray( Array *rBGroupArray );
    OptContentItem *nodeFromIndex(const QModelIndex &index, bool canBeNull = false) const;
    QModelIndex indexFromItem(OptContentItem *node, int column) const;

    /**
       Get the OptContentItem corresponding to a given reference value.

       \param ref the reference number (e.g. from Object.getRefNum()) to look up

       \return the matching optional content item, or null if the reference wasn't found
    */
    OptContentItem *itemFromRef( const QString &ref ) const;
    void setRootNode(OptContentItem *node);

    OptContentModel *q;

    QMap<QString, OptContentItem*> m_optContentItems;
    QList<RadioButtonGroup*> m_rbgroups;
    OptContentItem *m_rootNode;

    private:
    void addChild( OptContentItem *parent, OptContentItem *child);
    void parseOrderArray( OptContentItem *parentNode, Array *orderArray );
  };
}

#endif
