// Program to check hash < target for Bitcoin Block
#include "sha2.h"
#include "block_hash.h"
#include "ui_mainwindow.h"
#include <chrono>
#include <iomanip>
#include <map>
#include <utility>
#include <sstream>
#include <QDateTime>

void BLOCK::Initialize(Ui::MainWindow* ui)
{
    if (ui->radioButton_version1->isChecked())
        version = "00000001";
    if (ui->radioButton_version2->isChecked())
        version = "00000002";
    prev_hash = ui->lineEdit_previousHash->text().toStdString();
    merkle_root = ui->lineEdit_merkleRoot->text().toStdString();
    difficulty = ui->lineEdit_difficulty->text().toUInt();

    if (ui->radioButton_realCalc->isChecked())
    {
        bits_decimal = ui->lineEdit_bits->text().toUInt();
        {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(8) << std::hex << bits_decimal;
            bits = ss.str();
        }
        target = "0000000000000000000000000000000000000000000000000000000000000000";
        Bits_To_Target();
    }

    QDateTime dt = QDateTime::fromString(ui->lineEdit_timestamp->text(), Qt::ISODate);
    timestamp = (uint)(dt.toMSecsSinceEpoch() / 1000) + 4 * 3600;
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(8) << std::hex << timestamp;
        hex_time = ss.str();
    }
}

// Due to bitcoin specification data before hashing must be in littlendian
void Reverse_by_Pair(std::string& input)
{
    for(size_t i = 0, middle = input.size()/2, size = input.size(); i < middle ; i+=2 )
    {
        std::swap(input[i], input[size - i- 2]);
        std::swap(input[i+1], input[size -i - 1]);
    }
}

std::string DoubleSHA256(std::string text)
{
    SHA256 sha;
    std::string str1;
    for(uint i = 0; i < text.size(); i += 2)
    {
        std::string byte = text.substr(i, 2);
        char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
        str1.push_back(chr);
    }
    std::string s = sha(str1);
    std::string str2;
    for(uint i = 0; i < s.size(); i += 2)
    {
        std::string byte = s.substr(i, 2);
        char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
        str2.push_back(chr);
    }
    return sha(str2);
}

// Merge all arguments together and give block_without_nonce
void BLOCK::Copy_To_Block()
{
    Reverse_by_Pair(version);
    Reverse_by_Pair(prev_hash);
    Reverse_by_Pair(merkle_root);
    Reverse_by_Pair(hex_time);
    Reverse_by_Pair(bits);
    block_without_nonce = version + prev_hash + merkle_root + hex_time + bits;
}

int Hex2Int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

void BLOCK::Bits_To_Target()
{
    int last_zero = 2 * (16 * Hex2Int(bits[0]) + Hex2Int(bits[1])-3);
    int first_zero = 64 - last_zero - 6;
    target[first_zero]   =   bits[2];
    target[first_zero + 1] = bits[3];
    target[first_zero + 2] = bits[4];
    target[first_zero + 3] = bits[5];
    target[first_zero + 4] = bits[6];
    target[first_zero + 5] = bits[7];
}

uint BLOCK::Nonce_Cycle(Ui::MainWindow* ui)
{
    Copy_To_Block();
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point end;
    std::chrono::duration<double> elapsed;

    for( nonce = 0; nonce <= std::numeric_limits<uint>::max() ; ++nonce)
    {
        // Convert nonce to hex format
        {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(8) << std::hex << nonce;
            hex_nonce = ss.str();
        }

        // Finalize block
        Reverse_by_Pair(hex_nonce);
        header = block_without_nonce + hex_nonce;
        std::string double_hash = DoubleSHA256(header);
        Reverse_by_Pair(double_hash);

        // Check validity
        if (double_hash < target)
        {
            ui->textEdit_output->append("Block header parts in little endian form are");
            ui->textEdit_output->append("Version = " + QString::fromStdString(version));
            ui->textEdit_output->append("Previous Hash = " + QString::fromStdString(prev_hash));
            ui->textEdit_output->append("Merkle Root = " + QString::fromStdString(merkle_root));
            ui->textEdit_output->append("Timestamp = " + QString::fromStdString(hex_time) +
                                        " and in decimal is " + QString::number(timestamp));
            ui->textEdit_output->append("Bits = " + QString::fromStdString(bits) +
                                        " for test target ");
            ui->textEdit_output->append("Nonce = " + QString::fromStdString(hex_nonce) +
                                        " and in decimal is " + QString::number(nonce) + "\n");
            ui->textEdit_output->append("Block header data in little endian form is " +
                                        QString::fromStdString(header));
            ui->textEdit_output->append("Double hash is    " +
                                        QString::fromStdString(double_hash));
            ui->textEdit_output->append("Current target is " +
                                        QString::fromStdString(target) + "\n");
            ui->lineEdit_nonce->setText(QString::number(nonce));
            break;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    return (uint)elapsed.count();
}

uint BLOCK::Nonce_Cycle(Ui::MainWindow* ui, uint interval_start, uint interval_end)
{
    Initialize(ui);
    Copy_To_Block();
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point end;
    std::chrono::duration<double> elapsed;
    for( nonce = interval_start; nonce <= interval_end ; ++nonce)
    {
        // Convert nonce to hex format
        {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(8) << std::hex << nonce;
            hex_nonce = ss.str();
        }

        // Finalize block
        Reverse_by_Pair(hex_nonce);
        header = block_without_nonce + hex_nonce;
        std::string double_hash = DoubleSHA256(header);
        Reverse_by_Pair(double_hash);

        // Check validity
        if (double_hash < target)
        {
            emit Nonce(QString::fromUtf8(hex_time.c_str()), timestamp, QString::fromUtf8(bits.c_str()), QString::fromUtf8(hex_nonce.c_str()),
                          nonce,QString::fromUtf8(header.c_str()),QString::fromUtf8(double_hash.c_str()), QString::fromUtf8(target.c_str()));
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    return (uint)elapsed.count();
}

void BLOCK::Find_Nonce(Ui::MainWindow* ui)
{
    double time_interval = 0;
    time_interval = Nonce_Cycle(ui);
    ui->lineEdit_elapsedTime->setText(QString::number(time_interval));
    if(ui->textEdit_output->toPlainText().isEmpty())
    {
        ui->textEdit_output->append("With this parameters Nonce doesn't exists");
    }
}

void BLOCK::Find_Nonce(Ui::MainWindow* ui, uint interval_start, uint interval_end)
{
    uint time_interval = 0;
    time_interval = Nonce_Cycle(ui, interval_start, interval_end);
    emit PartialTime(time_interval);
}

void BLOCK::Find_Test(Ui::MainWindow* ui, int zeroes)
{
    int first_zeroes = zeroes;
    target = ui->lineEdit_target->text().toUtf8().constData();
    int last_zeroes = 64 - first_zeroes - 6;
    int hex_num = last_zeroes / 2 + 3;
    bits = "0061e9f8";
    {
        std::stringstream ss;
        ss << std::hex << hex_num;
        bits[0] = ss.str()[0];
        bits[1] = ss.str()[1];
    }
    Find_Nonce(ui);
}

void BLOCK::ReceiveValues(Ui::MainWindow *ui, uint interval_start, uint interval_end)
{
    this->ui = ui;
    this->interval_start = interval_start;
    this->interval_end = interval_end;
}

void BLOCK::StartFind()
{
    Find_Nonce(ui, interval_start, interval_end);
    emit finished();
}
