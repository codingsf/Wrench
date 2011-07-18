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

#include "growl_backend.h"

#include "core/snoreserver.h"

#include <growl++.hpp>

#include <QtCore>

Q_EXPORT_PLUGIN2(growl_backend,Growl_Backend)

Growl_Backend::Growl_Backend(SnoreServer *snore):
Notification_Backend("Growl",snore),
id(0)
{
    const char *n[1] = { "Default Alert"};
    Growl *growl = new Growl(GROWL_TCP,NULL,"SnoreNotify",n,1);
    _applications.insert("SnoreNotify",growl);

}
Growl_Backend::~Growl_Backend(){
    foreach(Application *a,this->snore()->aplications().values()){
        unregisterApplication(a);
    }
}

void Growl_Backend::registerApplication(Application *application){
    QList<Alert*> aList = application->alerts().values();
    int alertCount = application->alerts().count();
    char **n = new char*[alertCount];
    for (int i = 0 ; i < alertCount; ++i){
        QString name = aList.at(i)->name();
        n[i] = new char[name.length()+1];
        strcpy(n[i],name.toUtf8().constData());
    }

    _applications.insert(application->name(),new Growl(GROWL_TCP,NULL,application->name().toUtf8().constData(),(const char**)n,application->alerts().count()));

    for (int i = 0 ; i < alertCount; ++i){
        delete [] n[i];
    }
    delete [] n;
}

void Growl_Backend::unregisterApplication(Application *application){
    Growl *growl = _applications.take(application->name());
    if(growl == NULL)
        return;
    delete growl;
}

int Growl_Backend::notify(Notification notification){
    Growl *growl = _applications.value(notification.application());
    if(growl == NULL)
        return -1;

    QString title=Notification::toPlainText(notification.title());
    QString text=Notification::toPlainText(notification.text());
    qDebug()<<"Notify Growl:"<<notification.application()<<title;
    growl->Notify(notification.alert().toUtf8().constData(),title.toUtf8().constData(),text.toUtf8().constData(),"",notification.icon().toUtf8().constData());
    return ++id;
}

void Growl_Backend::closeNotification(Notification notification){
    Q_UNUSED(notification);
}

#include "growl_backend.moc"
