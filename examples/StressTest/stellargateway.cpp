#include "stellargateway.h"
#include "network.h"

StellarGateway::StellarGateway(QObject *parent) : QObject(parent)
  , m_server(new Server("https://api.mainnet.minepi.com"))
{
    m_servers.push_back(m_server);
    //Network::useTestNetwork();//our example only works on test network
    Network::usePublicNetwork();
}

StellarGateway::StellarGateway(QList<QString> urls, QObject *parent)
    : QObject(parent), m_server(new Server("https://api.mainnet.minepi.com"))
{
    m_servers.push_back(m_server);
    for(int i = 0; i < urls.size(); i++) {
        m_servers.push_back(new Server(urls.at(i)));
    }
    QRandomGenerator::securelySeeded();
}

Server *StellarGateway::server() const
{
    if (m_servers.size() > 1) {
        return m_servers.at(QRandomGenerator::system()->generate()%m_servers.size());
    }
    return m_server;
}

