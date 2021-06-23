#ifndef WALLET_H
#define WALLET_H

#include <QObject>
#include <QtQml/QtQml>

#include "account.h"
#include "stellargateway.h"

class Wallet : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString publicAddress READ publicAddress WRITE setPublicAddress NOTIFY publicAddressChanged)
    Q_PROPERTY(QString balance READ balance NOTIFY balanceChanged)
    Q_PROPERTY(bool funded READ funded NOTIFY fundedChanged)
    Q_PROPERTY(StellarGateway *gateway MEMBER m_gateway NOTIFY gatewayChanged)

    StellarGateway * m_gateway;
    Account *m_account;
    QString m_balance;



public:
    explicit Wallet(QObject *parent = nullptr);
    virtual ~Wallet();

    QString publicAddress() const;
    QString balance() const;
    bool funded() const;
    quint64 _interval = 15;
    quint64 _lastPay;
    bool _autoPay = false;
    QString _destination;
    QString _amount;
    QString _memo;

public slots:
    void createRandom();
    void fund();
    void update();
    void pay(QString destination, QString amount, QString memo);
    void startPay(QString destination, QString amount, QString memo);
    void stopPay();
    void create(QString destination, QString amount, QString memo);
    void import(const QString& words);
    void setPublicAddress(QString publicAddress);

signals:
    void publicAddressChanged();
    void balanceChanged();
    void fundedChanged();
    void error(QString lastError);
    void success();
    void gatewayChanged();
protected slots:
    void timerEvent(QTimerEvent *event) override;

private slots:
    void processAccountCheckResponse();
    void processPaymentSuccess();
    void processPaymentError();
};

QML_DECLARE_TYPE(Wallet)

#endif // WALLET_H
