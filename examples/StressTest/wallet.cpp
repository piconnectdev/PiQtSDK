#include "wallet.h"
#include "transaction.h"
#include "paymentoperation.h"
#include "createaccountoperation.h"
#include "claimclaimablebalanceoperation.h"
#include "responses/submittransactionresponse.h"
#include "responses/claimablebalanceresponse.h"

Wallet::Wallet(QObject *parent) : QObject(parent)
  , m_gateway(nullptr)
  , m_account(nullptr)  
{
    startTimer(1000);
}

Wallet::~Wallet()
{
    if(m_account)
        delete m_account;
}

QString Wallet::publicAddress() const
{
    return m_account ? m_account->getKeypair()->getAccountId() : QString();
}

QString Wallet::balance() const
{
    return m_balance;
}

bool Wallet::funded() const
{
    return m_account ? m_account->getSequenceNumber()!=0 : false;
}

void Wallet::createRandom()
{
    //we protect from overwrite the account.
    if(!m_account)
    {
        m_account = new Account(KeyPair::random(),0);
        //we just created m_keypair, it changes publicAddress property and we want to notify everybody about it
        emit publicAddressChanged();
    }
}

void Wallet::import(const QString& words)
{
    //we protect from overwrite the account.
    if(!m_account)
    {
        auto splits = words.split(" ");
        if (splits.size() != 24)\
            return;
        try {
            QByteArray seeds = Util::mnemonicToBIP39Seed(words, "");
            m_account = new Account(KeyPair::fromBip39Seed(seeds,314159,0), 0);
            //we just created m_keypair, it changes publicAddress property and we want to notify everybody about it
            emit publicAddressChanged();
        }
        catch(...) {

        }
    }
}


void Wallet::fund()
{
#if 1
    if(m_account && m_gateway){
       qDebug()<< QDateTime::currentDateTimeUtc().toLocalTime().toString(Qt::ISODateWithMs) << " Wallet::fund " << m_account->getKeypair()->getAccountId() ;
       AccountResponse * accountCheckResponse = m_gateway->server()->accounts().account(m_account->getKeypair());
        //here we connect the signal ready of the response to a private slot, that will be executed as soon response is ready
        connect(accountCheckResponse,&AccountResponse::ready
                ,this,&Wallet::processAccountCheckResponse);
        //claim();
    }
#else
    if(m_account && m_gateway){
        QNetworkRequest request(QString("https://friendbot.stellar.org?addr=%1").arg(publicAddress()));
        QNetworkReply * reply = m_gateway->server()->manager().get(request);
        connect(reply, &QNetworkReply::finished, this, &Wallet::update);//if we get an answer, we update the account
        connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);//we don't manage errors, it is an example!
    }
#endif
}
void Wallet::claim()
{
    Page<ClaimableBalanceResponse> * claimableCheckResponse = m_gateway->server()->claimableBalances().forClaimant(m_account->getKeypair()->getAccountId()).execute();
    connect(claimableCheckResponse,&Page<ClaimableBalanceResponse>::ready
            ,[=]() {
        Page<ClaimableBalanceResponse> *page = static_cast<Page<ClaimableBalanceResponse>*>(sender());
        page = claimableCheckResponse;
        if (page != nullptr) {
            for (auto idx = 0; idx < page->size(); idx++) {
                auto clm = page->at(idx);
                auto claimants = clm->getClaimants();
                qDebug() << clm->getSponsor() << clm->getAmount();
                qDebug() << clm->getID() << clm->getAssetString();
                qDebug() << clm->getLastModifiedLedger() << clm->getLastModifiedTime();
                for (auto cidx = 0; cidx < claimants.size(); cidx++) {
                    auto claimant = claimants.at(cidx);
                    qDebug() << claimant.getDestination() ;
                    if(claimant.getDestination() == m_account->getKeypair()->getAccountId()) {
                        Predicate::Not* pre = (Predicate::Not*)(&claimant.getPredicate());
                        Predicate::AbsBefore* bef = (Predicate::AbsBefore*)&pre->getInner();
                        qDebug() << bef->getTimestampSeconds();
                        if (QDateTime::currentSecsSinceEpoch() > bef->getTimestampSeconds()-10) {
                            Transaction *t = Transaction::Builder(m_account)
                                    .addOperation((new ClaimClaimableBalanceOperation(clm->getID()))->setSourceAccount(m_account->getKeypair()->getAccountId()))
                                    //.addMemo(new MemoText(memo))
                                    .setTimeout(10000)// we have to set a timeout
                                    .setBaseFee(100000)
                                    .build();
                            t->sign(m_account->getKeypair());
                            Server *server= m_gateway->server();
                            SubmitTransactionResponse* response = server->submitTransaction(t);
                            connect(response, &SubmitTransactionResponse::ready, [=]() {

                            });
                            connect(response, &SubmitTransactionResponse::error, [=]() {

                            });
                        }
                    }
                }
                qDebug() << "";
            }
        }
    });
}

void Wallet::update()
{
    if(m_account && m_gateway)
    {
        qDebug()<< QDateTime::currentDateTimeUtc().toLocalTime().toString(Qt::ISODateWithMs) << " Wallet::update " << m_account->getKeypair()->getAccountId() ;
        AccountResponse * accountCheckResponse = m_gateway->server()->accounts().account(m_account->getKeypair());
        //here we connect the signal ready of the response to a private slot, that will be executed as soon response is ready
        connect(accountCheckResponse,&AccountResponse::ready
                ,this,&Wallet::processAccountCheckResponse);

        Page<ClaimableBalanceResponse> * claimableCheckResponse = m_gateway->server()->claimableBalances().forClaimant(m_account->getKeypair()->getAccountId()).execute();
        connect(claimableCheckResponse,&Page<ClaimableBalanceResponse>::ready
                ,[=]() {
            Page<ClaimableBalanceResponse> *page = static_cast<Page<ClaimableBalanceResponse>*>(sender());
        });
    }
}

void Wallet::pay(QString destination, QString amount, QString memo)
{
    try {
        //verify the destination account is valid, building a keypair object, exception will be rised if there is a problem with it.
        //KeyPair* destKeypair = KeyPair::fromAccountId(destination);
        //since protocol 13, this check is performed inside operator constructors, it will rise a runtime exception
        qDebug()<< QDateTime::currentDateTimeUtc().toLocalTime().toString(Qt::ISODateWithMs) << " StartPay " << destination ;
        Server *server= m_gateway->server();

        Transaction *t = Transaction::Builder(m_account)
                .addOperation(new PaymentOperation(destination,new AssetTypeNative(),amount))
                .addMemo(new MemoText(memo))
                .setTimeout(10000)// we have to set a timeout
                .setBaseFee(100000)
                .build();
        t->sign(m_account->getKeypair());
        SubmitTransactionResponse* response = server->submitTransaction(t);
        connect(response, &SubmitTransactionResponse::ready, this, &Wallet::processPaymentSuccess);
        connect(response, &SubmitTransactionResponse::error, this, &Wallet::processPaymentError);
        _lastDestination = destination;
        _totalPay++;
        emit stateChanged();

    } catch (std::exception err) {
        QString lastError(err.what());
        emit error(lastError);
    }

}

void Wallet::startPay(int interval, QString destination, QString amount, QString memo)
{
    return;
    _totalPay = 0;
    _totalSuccess = 0;
    _totalError = 0;
    _autoPay = true;
    _indexDest = 0;
    //_destination = destination;
    if (interval < 5 || interval >= 300) {
        _interval = 5;
    } else {
        _interval = interval;
    }
    _amount = amount;
    bool ok = false;
    double value = _amount.toDouble(&ok);
    if( value < 0.0 || !ok) {
        _amount = "0.0001";
    }
    _memo = memo;
    _destinations.clear();
    _lastPay = QDateTime::currentMSecsSinceEpoch();
    QList<QString> split = destination.split("\n");
    for (int i = 0; i < split.size(); i++) {
        try {
        KeyPair *key = KeyPair::fromAccountId(split.at(i).trimmed());
        if (key != nullptr) {
            _destinations.push_back(split.at(i).trimmed());
            delete key;
        }
        }
        catch(...) {
            continue;
        }
    }
    if (_destinations.size() > 0) {
        _isWorking = true;
        pay(_destinations.at(0), _amount, _memo);
    }
    emit stateChanged();
}

void Wallet::stopPay()
{
    return;
    _isWorking = false;
    _autoPay = false;
    emit stateChanged();
}

void Wallet::create(QString destination, QString amount, QString memo)
{
    try {
        //verify the destination account is valid, building a keypair object, exception will be rised if there is a problem with it.
        KeyPair* destKeypair = KeyPair::fromAccountId(destination);
        //since protocol 13, this check is performed inside operator constructors, it will rise a runtime exception

        Server *server= m_gateway->server();

        Transaction *t = Transaction::Builder(m_account)
                .addOperation((new CreateAccountOperation(destination,amount))->setSourceAccount(m_account->getKeypair()->getAccountId()))
                .addMemo(new MemoText(memo))
                .setTimeout(10000)// we have to set a timeout
                .setBaseFee(100000)
                .build();
        t->sign(m_account->getKeypair());
        SubmitTransactionResponse* response = server->submitTransaction(t);
        connect(response, &SubmitTransactionResponse::ready, this, &Wallet::processPaymentSuccess);
        connect(response, &SubmitTransactionResponse::error, this, &Wallet::processPaymentError);

    } catch (std::exception err) {
        QString lastError(err.what());
        emit error(lastError);
    }

}

void Wallet::setPublicAddress(QString publicAddress)
{
    //if is the same, we don't modify anything
    if(m_account && m_account->getKeypair()->getAccountId()==publicAddress)
        return;
    //if it changed, we delete it
    if(m_account)
        delete m_account;

    try {
         KeyPair *key = KeyPair::fromAccountId(publicAddress);
            m_account = new Account(key, 0);
            update();
    } catch (std::exception err) {
        m_account = nullptr;
    }
    emit publicAddressChanged();
}

void Wallet::processAccountCheckResponse()
{
    //sender() returns who calls the slot.
    AccountResponse *accountCheckResponse = static_cast<AccountResponse*>(sender());
    if(accountCheckResponse->accountID().isEmpty()){
        //account not funded yet
    }
    else{
        //response contains account data
        //update the account with the correct sequence number, we have to do a copy of the original keypair because keypair is deleted with the account
        Account* updatedAccount = new Account(new KeyPair(*(m_account->getKeypair())), accountCheckResponse->getSequenceNumber());
        delete m_account;
        m_account = updatedAccount;
        auto balances = accountCheckResponse->getBalances();
        for(auto balance: balances)
        {
            if(balance.getAssetType() == "native")
            {
                m_balance = balance.getBalance();
                //notify balance changed!
                emit balanceChanged();
            }
        }
    }
    //notify we know now if it is funded or not
    emit fundedChanged();
    //remove the response as we don't need anymore
    delete accountCheckResponse; //we don't need it anymore. It would be deleted anyway by server destruction, but normally you will have to remove it here
}

void Wallet::processPaymentSuccess()
{

    _totalSuccess++;
    if(SubmitTransactionResponse * response = dynamic_cast<SubmitTransactionResponse*>(sender())){

        this->update();//request an update of the account
        delete response;//you have to delete the response
        emit success();
    }
}

void Wallet::processPaymentError()
{
    _totalError++;
    if(SubmitTransactionResponse * response = dynamic_cast<SubmitTransactionResponse*>(sender())){
        emit error(QString("Payment error"));
        this->update();//request an update of the account
        delete response;//you have to delete the response
    }
}

void Wallet::timerEvent(QTimerEvent *event)
{
    //qDebug()<< QDateTime::currentDateTimeUtc().toLocalTime().toString(Qt::ISODateWithMs) << "timerEvent" ;
    if (_autoPay) {
        quint64 now = QDateTime::currentMSecsSinceEpoch();
        if ((now - _lastPay) > _interval*1000) {
            if (_destinations.size() > 0) {
                _indexDest++;
                _destination = _destinations.at(_indexDest%_destinations.size());
                pay(_destination, _amount, _memo);
                _lastPay = now;
            }
        }
    }
}
