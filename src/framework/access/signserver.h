/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2018 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#ifndef NGFRAMEWORK_SIGNSERVER_H
#define NGFRAMEWORK_SIGNSERVER_H

#include <QProgressDialog>
#include <QTcpServer>

class Q_DECL_HIDDEN NGSignServer : public QProgressDialog
{
    Q_OBJECT
public:
    explicit NGSignServer(const QString &clientId, const QString &scope,
                          QWidget *parent = nullptr);
    virtual ~NGSignServer();

    QString code() const;
    QString redirectUri() const;

signals:

public slots:

private slots:
    void onIncomingConnection();
    void onGetReply();

private:
    QString m_code;
    QString m_redirectUri;
    QString m_replyContent;
    QString m_clientId, m_scope;
    QTcpServer *m_listenServer;

    // QDialog interface
public slots:
#if QT_VERSION >= 0x050000
    virtual int exec() override;
#else
    int exec();
#endif // QT_VERSION >= 0x050000
};

#endif // NGFRAMEWORK_SIGNSERVER_H
