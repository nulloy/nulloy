// clang-format off
/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************
**
** Changelog:
** * Feb 17 2011 Sergey Vlasov <sergey@vlasov.me>
** - Added support of QDataStream operators
**
****************************************************************************/

#ifndef N_CACHE_H
#define N_CACHE_H

#include <QtCore/qhash.h>

template <class Key, class T> class NCache
{
private:
    struct Node {
        inline Node() {}
        inline Node(T *data, int cost) : t(new T(*data)), c(cost) {}
        T *t;
        int c;

        friend inline QDataStream &operator<<(QDataStream &out, const Node &n)
        {
            out << *n.t << n.c;
            return out;
        }

        friend inline QDataStream &operator>>(QDataStream &in, Node &n)
        {
            T obj;
            in >> obj >> n.c;
            n.t = new T(obj);
            return in;
        }

        inline bool operator==(const Node &n) const { return t == n.t && c == n.c; }
    };

    QHash<Key, Node> hash;
    QList <Key> list;
    int mx, total;

    inline void unlink(Node &n)
    {
        Key k = hash.key(n);
        list.removeOne(k);
        total -= n.c;
        T *obj = n.t;
        hash.remove(k);
        delete obj;
    }

    inline T *relink(const Key &key)
    {
        typename QHash<Key, Node>::iterator i = hash.find(key);
        if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd())
            return 0;

        Node &n = *i;
        Key k = hash.key(n);
        if (list.first() != k)
            list.move(list.indexOf(k), 0);
        return n.t;
    }

    Q_DISABLE_COPY(NCache)

public:
    inline explicit NCache(int maxCost = 100);

    inline ~NCache() { clear(); }

    inline int maxCost() const { return mx; }
    void setMaxCost(int m);
    inline int totalCost() const { return total; }

    inline int size() const { return hash.size(); }
    inline int count() const { return hash.size(); }
    inline bool isEmpty() const { return hash.isEmpty(); }
    inline QList<Key> keys() const { return hash.keys(); }

    void clear();

    bool insert(const Key &key, T *object, int cost = 1);
    T *object(const Key &key) const;
    inline bool contains(const Key &key) const { return hash.contains(key); }
    T *operator[](const Key &key) const;

    bool remove(const Key &key);
    T *take(const Key &key);

    friend inline QDataStream &operator<<(QDataStream &out, const NCache<Key, T> &c)
    {
        out << c.hash << c.list << c.mx << c.total;
        return out;
    }

    friend inline QDataStream &operator>>(QDataStream &in, NCache<Key, T> &c)
    {
        c.clear();
        in >> c.hash >> c.list >> c.mx >> c.total;
        return in;
    }

private:
    void trim(int m);
};

template <class Key, class T> inline NCache<Key, T>::NCache(int amaxCost) : mx(amaxCost), total(0) {}

template <class Key, class T> inline void NCache<Key, T>::clear()
{
    list.clear();
    hash.clear();
    total = 0;
}

template <class Key, class T> inline void NCache<Key, T>::setMaxCost(int m) { mx = m; trim(mx); }
template <class Key, class T> inline T *NCache<Key, T>::object(const Key &key) const { return const_cast<NCache<Key, T> *>(this)->relink(key); }
template <class Key, class T> inline T *NCache<Key, T>::operator[](const Key &key) const { return object(key); }

template <class Key, class T> inline bool NCache<Key, T>::remove(const Key &key)
{
    typename QHash<Key, Node>::iterator i = hash.find(key);
    if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd()) {
        return false;
    } else {
        unlink(*i);
        return true;
    }
}

template <class Key, class T> inline T *NCache<Key, T>::take(const Key &key)
{
    typename QHash<Key, Node>::iterator i = hash.find(key);
    if (i == hash.end())
        return 0;

    Node &n = *i;
    T *t = n.t;
    n.t = 0;
    unlink(n);
    return t;
}

template <class Key, class T> bool NCache<Key, T>::insert(const Key &akey, T *aobject, int acost)
{
    remove(akey);
    if (acost > mx) {
        delete aobject;
        return false;
    }
    trim(mx - acost);
    Node sn(aobject, acost);
    hash.insert(akey, sn);
    total += acost;
    list.prepend(akey);
    return true;
}

template <class Key, class T> void NCache<Key, T>::trim(int m)
{
    int i = list.size() - 1;
    if (i < 0)
        return;
    Node *n = &hash[list.at(i)];
    while (n && total > m) {
        Node *u = n;
        --i;
        n = &hash[list.at(i)];
        //if (qIsDetached(*u->t))
            unlink(*u);
    }
}

#endif
// clang-format on
