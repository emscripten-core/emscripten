/* poppler-optcontent.cc: qt interface to poppler
 *
 * Copyright (C) 2007, Brad Hards <bradh@kde.org>
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, Carlos Garcia Campos <carlosgc@gnome.org>
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

#include "poppler-optcontent.h"

#include "poppler-optcontent-private.h"

#include "poppler-private.h"

#include <QtCore/QDebug>
#include <QtCore/QtAlgorithms>

#include "poppler/OptionalContent.h"

namespace Poppler
{

  RadioButtonGroup::RadioButtonGroup( OptContentModelPrivate *ocModel, Array *rbarray )
  {
    for (int i = 0; i < rbarray->getLength(); ++i) {
      Object ref;
      rbarray->getNF( i, &ref );
      if ( ! ref.isRef() ) {
	qDebug() << "expected ref, but got:" << ref.getType();
      }
      OptContentItem *item = ocModel->itemFromRef( QString::number(ref.getRefNum() ) );
      itemsInGroup.append( item );
    }
    for (int i = 0; i < itemsInGroup.size(); ++i) {
      OptContentItem *item = itemsInGroup.at(i);
      item->appendRBGroup( this );
    }
  }

  RadioButtonGroup::~RadioButtonGroup()
  {
  }

  QSet<OptContentItem *> RadioButtonGroup::setItemOn( OptContentItem *itemToSetOn )
  {
    QSet<OptContentItem *> changedItems;
    for (int i = 0; i < itemsInGroup.size(); ++i) {
      OptContentItem *thisItem = itemsInGroup.at(i);
      if (thisItem != itemToSetOn) {
        QSet<OptContentItem *> newChangedItems;
        thisItem->setState(OptContentItem::Off, newChangedItems);
        changedItems += newChangedItems;
      }
    }
    return changedItems;
  }



  OptContentItem::OptContentItem( OptionalContentGroup *group )
  {
    m_group = group;
    m_parent = 0;
    m_name = UnicodeParsedString( group->getName() );
    if ( group->getState() == OptionalContentGroup::On ) {
      m_state = OptContentItem::On;
    } else {
      m_state = OptContentItem::Off;
    }
    m_stateBackup = m_state;
    m_enabled = true;
  }

  OptContentItem::OptContentItem( const QString &label )
  {
    m_parent = 0;
    m_name = label;
    m_group = 0;
    m_state = OptContentItem::HeadingOnly;
    m_stateBackup = m_state;
    m_enabled = true;
  }

  OptContentItem::OptContentItem() :
    m_parent( 0 ), m_enabled(true)
  {
  }

  OptContentItem::~OptContentItem()
  {
  }

  void OptContentItem::appendRBGroup( RadioButtonGroup *rbgroup )
  {
    m_rbGroups.append( rbgroup );
  }


  bool OptContentItem::setState(ItemState state, QSet<OptContentItem *> &changedItems)
  {
    m_state = state;
    m_stateBackup = m_state;
    changedItems.insert(this);
    QSet<OptContentItem *> empty;
    Q_FOREACH (OptContentItem *child, m_children) {
      ItemState oldState = child->m_stateBackup;
      child->setState(state == OptContentItem::On ? child->m_stateBackup : OptContentItem::Off, empty);
      child->m_enabled = state == OptContentItem::On;
      child->m_stateBackup = oldState;
    }
    if (!m_group) {
      return false;
    }
    if ( state == OptContentItem::On ) {
      m_group->setState( OptionalContentGroup::On );
      for (int i = 0; i < m_rbGroups.size(); ++i) {
	RadioButtonGroup *rbgroup = m_rbGroups.at(i);
        changedItems += rbgroup->setItemOn( this );
      }
    } else if ( state == OptContentItem::Off ) {
      m_group->setState( OptionalContentGroup::Off );
    }
    return true;
  }

  void OptContentItem::addChild( OptContentItem *child )
  {
    m_children += child;
    child->setParent( this );
  }

  QSet<OptContentItem*> OptContentItem::recurseListChildren(bool includeMe) const
  {
    QSet<OptContentItem*> ret;
    if (includeMe) {
      ret.insert(const_cast<OptContentItem*>(this));
    }
    Q_FOREACH (OptContentItem *child, m_children) {
      ret += child->recurseListChildren(true);
    }
    return ret;
  }

  OptContentModelPrivate::OptContentModelPrivate( OptContentModel *qq, OCGs *optContent )
    : q(qq)
  {
    m_rootNode = new OptContentItem();
    GooList *ocgs = optContent->getOCGs();

    for (int i = 0; i < ocgs->getLength(); ++i) {
      OptionalContentGroup *ocg = static_cast<OptionalContentGroup*>(ocgs->get(i));
      OptContentItem *node = new OptContentItem( ocg );
      m_optContentItems.insert( QString::number(ocg->getRef().num), node);
    }

    if ( optContent->getOrderArray() == 0 ) {
      // no Order array, so drop them all at the top level
      QMapIterator<QString, OptContentItem*> i(m_optContentItems);
      while ( i.hasNext() ) {
	i.next();
	qDebug() << "iterator" << i.key() << ":" << i.value();
	addChild( i.value(), m_rootNode );
      }
    } else {
      parseOrderArray( m_rootNode, optContent->getOrderArray() );
    }

    parseRBGroupsArray( optContent->getRBGroupsArray() );
  }

  OptContentModelPrivate::~OptContentModelPrivate()
  {
    qDeleteAll( m_optContentItems );
    qDeleteAll( m_rbgroups );
    delete m_rootNode;
  }

  void OptContentModelPrivate::parseOrderArray( OptContentItem *parentNode, Array *orderArray )
  {
    OptContentItem *lastItem = parentNode;
    for (int i = 0; i < orderArray->getLength(); ++i) {
      Object orderItem;
      orderArray->get(i, &orderItem);
      if ( orderItem.isDict() ) {
	Object item;
	orderArray->getNF(i, &item);
	if (item.isRef() ) {
          OptContentItem *ocItem = m_optContentItems.value(QString::number(item.getRefNum()), 0);
	  if (ocItem) {
	    addChild( parentNode, ocItem );
	    lastItem = ocItem;
	  } else {
            qDebug() << "could not find group for object" << item.getRefNum();
	  }
	}
	item.free();
      } else if ( (orderItem.isArray()) && (orderItem.arrayGetLength() > 0) ) {
	parseOrderArray(lastItem, orderItem.getArray());
      } else if ( orderItem.isString() ) {
	GooString *label = orderItem.getString();
	OptContentItem *header = new OptContentItem ( UnicodeParsedString ( label ) );
	addChild( parentNode, header );
	parentNode = header;
	lastItem = header;
      } else {
	qDebug() << "something unexpected";
      }	
      orderItem.free();
    }
  }

  void OptContentModelPrivate::parseRBGroupsArray( Array *rBGroupArray )
  {
    if (! rBGroupArray) {
      return;
    }
    // This is an array of array(s)
    for (int i = 0; i < rBGroupArray->getLength(); ++i) {
      Object rbObj;
      rBGroupArray->get(i, &rbObj);
      if ( ! rbObj.isArray() ) {
	qDebug() << "expected inner array, got:" << rbObj.getType();
	return;
      }
      Array *rbarray = rbObj.getArray();
      RadioButtonGroup *rbg = new RadioButtonGroup( this, rbarray );
      m_rbgroups.append( rbg );
      rbObj.free();
    }
  }

  OptContentModel::OptContentModel( OCGs *optContent, QObject *parent)
    : QAbstractItemModel(parent)
  {
    d = new OptContentModelPrivate( this, optContent );
  }

  OptContentModel::~OptContentModel()
  {
    delete d;
  }

  void OptContentModelPrivate::setRootNode(OptContentItem *node)
  {
    delete m_rootNode;
    m_rootNode = node;
    q->reset();
  }

  QModelIndex OptContentModel::index(int row, int column, const QModelIndex &parent) const
  {
    if (row < 0 || column != 0) {
      return QModelIndex();
    }

    OptContentItem *parentNode = d->nodeFromIndex( parent );
    if (row < parentNode->childList().count()) {
      return createIndex(row, column, parentNode->childList().at(row));
    }
    return QModelIndex();
  }

  QModelIndex OptContentModel::parent(const QModelIndex &child) const
  {
    OptContentItem *childNode = d->nodeFromIndex( child );
    if (!childNode) {
      return QModelIndex();
    }
    return d->indexFromItem(childNode->parent(), child.column());
  }

  QModelIndex OptContentModelPrivate::indexFromItem(OptContentItem *node, int column) const
  {
    if (!node) {
      return QModelIndex();
    }
    OptContentItem *parentNode = node->parent();
    if (!parentNode) {
      return QModelIndex();
    }
    const int row = parentNode->childList().indexOf(node);
    return q->createIndex(row, column, node);
  }
 
  int OptContentModel::rowCount(const QModelIndex &parent) const
  {
    OptContentItem *parentNode = d->nodeFromIndex( parent );
    if (!parentNode) {
      return 0;
    } else {
      return parentNode->childList().count();
    }
  }

  int OptContentModel::columnCount(const QModelIndex &parent) const
  {
    return 1;
  }


  QVariant OptContentModel::data(const QModelIndex &index, int role) const
  {
    OptContentItem *node = d->nodeFromIndex(index, true);
    if (!node) {
      return QVariant();
    }

    switch (role) {
      case Qt::DisplayRole:
        return node->name();
        break;
      case Qt::EditRole:
        if (node->state() == OptContentItem::On) {
          return true;
        } else if (node->state() == OptContentItem::Off) {
          return false;
        }
        break;
      case Qt::CheckStateRole:
        if (node->state() == OptContentItem::On) {
          return Qt::Checked;
        } else if (node->state() == OptContentItem::Off) {
          return Qt::Unchecked;
        }
        break;
    }

    return QVariant();
  }

  bool OptContentModel::setData ( const QModelIndex & index, const QVariant & value, int role )
  {
    OptContentItem *node = d->nodeFromIndex(index, true);
    if (!node) {
      return false;
    }

    switch (role) {
      case Qt::CheckStateRole:
      {
        const bool newvalue = value.toBool();
        if (newvalue) {
          if (node->state() != OptContentItem::On) {
            QSet<OptContentItem *> changedItems;
            node->setState(OptContentItem::On, changedItems);
            changedItems += node->recurseListChildren(false);
            QModelIndexList indexes;
            Q_FOREACH (OptContentItem *item, changedItems) {
              indexes.append(d->indexFromItem(item, 0));
            }
            qStableSort(indexes);
            Q_FOREACH (const QModelIndex &changedIndex, indexes) {
              emit dataChanged(changedIndex, changedIndex);
            }
            return true;
          }
        } else {
          if (node->state() != OptContentItem::Off) {
            QSet<OptContentItem *> changedItems;
            node->setState(OptContentItem::Off, changedItems);
            changedItems += node->recurseListChildren(false);
            QModelIndexList indexes;
            Q_FOREACH (OptContentItem *item, changedItems) {
              indexes.append(d->indexFromItem(item, 0));
            }
            qStableSort(indexes);
            Q_FOREACH (const QModelIndex &changedIndex, indexes) {
              emit dataChanged(changedIndex, changedIndex);
            }
            return true;
          }
        }
        break;
      }
    }

    return false;
  }

  Qt::ItemFlags OptContentModel::flags ( const QModelIndex & index ) const
  {
    OptContentItem *node = d->nodeFromIndex(index);
    Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    if (node->isEnabled()) {
      itemFlags |= Qt::ItemIsEnabled;
    }
    return itemFlags;
  }

  QVariant OptContentModel::headerData( int section, Qt::Orientation orientation, int role ) const
  {
    return QAbstractItemModel::headerData( section, orientation, role );
  }

  void OptContentModelPrivate::addChild( OptContentItem *parent, OptContentItem *child )
  {
    parent->addChild( child );
  }

  OptContentItem* OptContentModelPrivate::itemFromRef( const QString &ref ) const
  {
    return m_optContentItems.value(ref, 0);
  }

  OptContentItem* OptContentModelPrivate::nodeFromIndex(const QModelIndex &index, bool canBeNull) const
  {
    if (index.isValid()) {
      return static_cast<OptContentItem *>(index.internalPointer());
    } else {
      return canBeNull ? 0 : m_rootNode;
    }
  }
}

#include "poppler-optcontent.moc"
