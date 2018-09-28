#ifndef HASH_H
#define HASH_H

#include <QObject>

class HASH : public QObject
{
    Q_OBJECT

signals:
    void HashrateReady(double);
    void finished();

public slots:
    void ReceiveIntervals(uint,uint);
    double ThreadHashrate();
    void Start();

private:
    uint interval_start;
    uint interval_end;
};

#endif // HASH_H
