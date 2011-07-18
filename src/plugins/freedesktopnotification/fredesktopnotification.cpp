/****************************************************************************************
 * Copyright (c) 2010 Patrick von Reth <patrick.vonreth@gmail.com>                      *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "fredesktopnotification.h"

#include <QImage>
#include <QtGlobal>

#include <iostream>

FreedesktopNotification::FreedesktopNotification()
{
    registerTypes();
}

FreedesktopNotification::FreedesktopNotification(Notification noti):
        notification(noti)
{
    registerTypes();
}

void FreedesktopNotification::registerTypes() {
    qDBusRegisterMetaType<FreedesktopImageHint>();
    qDBusRegisterMetaType<FreedesktopNotification>();
}

QDBusArgument &operator<<(QDBusArgument &a, const FreedesktopNotification &i) {
    //   Q_ASSERT(!i.notification.isNull());
    qDebug()<<i.notification.toString();
    a<<i.notification.application();
    a<<uint(0);
    a<<i.notification.icon();
    a<<i.notification.title();
    a<<i.notification.text();
    QStringList actions;
    actions<<"1"<<" "<<"2"<<" ";
    a<<actions;
    a.beginMap();
    QImage img(i.notification.icon());
    if (!img.isNull()) {
        img=img.scaledToWidth(50,Qt::FastTransformation);
        a.beginMapEntry();
        a<<"image_data";
        FreedesktopImageHint fh(img);
        a<<fh;
        a.endMapEntry();
    }
    a.endMap();
    a<<i.notification.timeout()*1000;
    return a;
}

const QDBusArgument & operator >>(const QDBusArgument &a,  FreedesktopNotification &) {
    //not supported
    return a;
}


QHash<QString,void*> FreedesktopImageHint::hasedImages;

FreedesktopImageHint::FreedesktopImageHint()
{
    FreedesktopNotification::registerTypes();
}



FreedesktopImageHint::FreedesktopImageHint(const QImage &img)
{
    FreedesktopNotification::registerTypes();
    QImage image(img.convertToFormat(QImage::Format_ARGB32));
    this->imageData.append((char*)image.rgbSwapped().bits(),image.numBytes());
    width=image.width();
    height=image.height();
    rowstride=image.bytesPerLine();
    hasAlpha=image.hasAlphaChannel();
    channels =image.isGrayscale()?1:hasAlpha?4:3;
    bitsPerSample=image.depth()/channels;

}

QString FreedesktopImageHint::computeHash(){
    if(this->_hash.isEmpty()){
        Q_ASSERT(!imageData.isEmpty());
        QCryptographicHash h(QCryptographicHash::Md5);
        h.addData(imageData);
        this->_hash = h.result().toHex();
    }
    return this->_hash;
}
QString FreedesktopImageHint::hash()const{
    Q_ASSERT(!_hash.isEmpty());
    return _hash;
}


QImage FreedesktopImageHint::toQImage() const {
    return QImage((uchar*)imageData.data(),width,height,QImage::Format_ARGB32 ).rgbSwapped();
}

QDBusArgument &operator<<(QDBusArgument &a, const FreedesktopImageHint &i) {
    a.beginStructure();
    a << i.width<<i.height<<i.rowstride<<i.hasAlpha<<i.bitsPerSample<<i.channels<<i.imageData;
    a.endStructure();
    return a;
}

const QDBusArgument & operator >>(const QDBusArgument &a,  FreedesktopImageHint &i) {
    a.beginStructure();    
    a >> i.width>> i.height>> i.rowstride>> i.hasAlpha>> i.bitsPerSample>> i.channels>> i.imageData;
    a.endStructure();
    FreedesktopImageHint::hasedImages.insert(i.computeHash(),NULL);
    return a;
}
