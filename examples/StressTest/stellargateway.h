#ifndef STELLARGATEWAY_H
#define STELLARGATEWAY_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include "server.h"



class StellarGateway : public QObject
{
    Q_OBJECT


    Server *m_server;
    QList<Server*> m_servers;
    int index = 0;
public:
    explicit StellarGateway(QObject *parent = nullptr);
    StellarGateway(QList<QString>urls, QObject *parent = nullptr);

    Server * server() const;


};

#endif // STELLARGATEWAY_H
