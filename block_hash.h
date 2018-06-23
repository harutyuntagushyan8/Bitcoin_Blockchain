#ifndef BLOCK_HASH_H
#define BLOCK_HASH_H

#include <cstring>
#include <ctime>
#include <QObject>
#include "mainwindow.h"

class BLOCK : public QObject
{
    Q_OBJECT
private:                                                // BLOCK HEADER
    std::string version;                                // Version number
    std::string prev_hash;                              // Previous hash
    std::string merkle_root;                            // Merkle root
    std::string hex_time;                               // Current Timestamp
    std::string bits;                                   // Difficulty,Bits
    std::string hex_nonce;                              // Initial Nonce
    unsigned long difficulty, bits_decimal;
    bool noncelimit;
    std::string target;
    uint nonce;
	std::string block_without_nonce;
    std::string header;
    uint timestamp;

public:
    BLOCK(QObject *parent = 0);
    ~BLOCK();

private:
	void Reverse_by_Pair(std::string& input, bool reverse);
	std::string DoubleSHA256(std::string text);
    void Copy_To_Block();
    void Bits_To_Target();

private slots:
	void Initialize(Ui::MainWindow* ui);
	double Nonce_Cycle(Ui::MainWindow* ui);
    void Find_Nonce(Ui::MainWindow* ui);
	void Find(Ui::MainWindow* ui, bool b, int zeroes);
};

int Hex2Int(char ch);

#endif // BLOCK_HASH_H
