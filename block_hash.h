#ifndef BLOCK_HASH_H
#define BLOCK_HASH_H

#include "mainwindow.h"
#include <vector>
#include <cstring>
#include <ctime>
#include <QObject>

class BLOCK : public QObject
{
    Q_OBJECT

public:
    BLOCK(QObject *parent = 0);
    ~BLOCK();

private:
    void Copy_To_Block();
    void Bits_To_Target();

signals:
    void PartialTime(uint);
    void finished();
    void Nonce(QString, uint, QString, QString, uint, QString, QString, QString);

public slots:
	void Initialize(Ui::MainWindow* ui);
    uint Nonce_Cycle(Ui::MainWindow* ui);
    void Find_Nonce(Ui::MainWindow* ui);
    uint Nonce_Cycle(Ui::MainWindow* ui, uint interval_start, uint interval_end);
    void Find_Nonce(Ui::MainWindow* ui, uint interval_start, uint interval_end);
    void Find_Test(Ui::MainWindow* ui, int);
    void ReceiveValues(Ui::MainWindow* ui, uint interval_start, uint interval_end);
    void StartFind();

private:                                                // BLOCK HEADER
    std::string version;                                // Version number
    std::string prev_hash;                              // Previous hash
    std::string merkle_root;                            // Merkle root
    std::string hex_time;                               // Current Timestamp
    std::string bits;                                   // Difficulty,Bits
    std::string hex_nonce;                              // Initial Nonce
    uint difficulty, bits_decimal;
    std::string target;
    uint nonce;
    std::string block_without_nonce;
    std::string header;
    uint timestamp;

    Ui::MainWindow* ui;
    uint interval_start;
    uint interval_end;
};

int Hex2Int(char ch);
void Reverse_by_Pair(std::string& input);
std::string DoubleSHA256(std::string text);

#endif // BLOCK_HASH_H
