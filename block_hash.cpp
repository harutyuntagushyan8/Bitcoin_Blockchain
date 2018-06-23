// Program to check hash < target for Bitcoin Block
#include "sha2.h"
#include "block_hash.h"
#include "ui_mainwindow.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <QDateTime>
#include <QDebug>

BLOCK::BLOCK(QObject* parent):QObject(parent)
{

}

void BLOCK::Initialize(Ui::MainWindow* ui)
{
	noncelimit = false;
	if(ui->radioButton_version1->isChecked())
		version = "00000001";
	if(ui->radioButton_version2->isChecked())
		version = "00000002";
	prev_hash = ui->lineEdit_previousHash->text().toStdString();
    merkle_root = ui->lineEdit_merkleRoot->text().toStdString();
	difficulty = ui->lineEdit_difficulty->text().toUInt();

	if(ui->radioButton_realCalc->isChecked())
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
void BLOCK::Reverse_by_Pair(std::string& input, bool reverse)
{
	if(reverse)
	{
		for(size_t i = 0, middle = input.size()/2, size = input.size(); i < middle ; i+=2 )
		{
			std::swap(input[i], input[size - i- 2]);
			std::swap(input[i+1], input[size -i - 1]);
		}
	}
}

std::string BLOCK::DoubleSHA256(std::string text)
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
	Reverse_by_Pair(version, !noncelimit);
	Reverse_by_Pair(prev_hash, !noncelimit);
	Reverse_by_Pair(merkle_root, !noncelimit);
	Reverse_by_Pair(hex_time, true);
	Reverse_by_Pair(bits, !noncelimit);
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
    target[first_zero]   = bits[2];
    target[first_zero+1] = bits[3];
    target[first_zero+2] = bits[4];
    target[first_zero+3] = bits[5];
    target[first_zero+4] = bits[6];
	target[first_zero+5] = bits[7];
}

double BLOCK::Nonce_Cycle(Ui::MainWindow* ui)
{
	Copy_To_Block();
	auto start = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point end, end_local;
	std::chrono::duration<double> elapsed, elapsed_local;

	for( nonce = 0; nonce <= std::numeric_limits<uint>::max(); ++nonce)
	{
        // Convert nonce to hex format
		{
			std::stringstream ss;
			ss << std::setfill('0') << std::setw(8) << std::hex << nonce;
			hex_nonce = ss.str();
		}

        // Finalize block
		Reverse_by_Pair(hex_nonce, true);
		header = block_without_nonce + hex_nonce;
		std::string double_hash = DoubleSHA256(header);
		Reverse_by_Pair(double_hash, true);

		// Check validity
		if(double_hash < target)
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
			ui->textEdit_output->append("Double hash of block is " +
								  QString::fromStdString(double_hash));
			ui->textEdit_output->append("Current target is " +
								  QString::fromStdString(target) + "\n");

			ui->lineEdit_nonce->setText(QString::number(nonce));
			noncelimit = false;
			end_local = std::chrono::high_resolution_clock::now();
			elapsed_local = end_local - start;
			break;
		}
		noncelimit = true;
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed = end - start;
	if(noncelimit)
		return (double)elapsed.count();
	return (double)elapsed_local.count();
}

void BLOCK::Find_Nonce(Ui::MainWindow* ui)
{
	int i = 0;
	double time_interval = 0;
	do
	{
		time_interval += Nonce_Cycle(ui);
		if(noncelimit == false)
		{
			break;
		}
		else
		{
			++i;
			timestamp += 1;
			{
				std::stringstream ss;
				ss << std::setfill('0') << std::setw(8) << std::hex << timestamp;
				hex_time = ss.str();
			}
			ui->textEdit_output->append("With this timestamp Nonce doesn't exists\n");
			ui->textEdit_output->append("Corellated timestamp is " +
						(QDateTime::fromTime_t(timestamp - 4 * 3600)).toString(Qt::ISODate).replace(QString("T"),QString(" ")) + "\n");
			ui->lineEdit_elapsedTime->setText(QString::number(time_interval) + " seconds");
		}
	}
	while(i < 720);   // Corellated time must not be more than 2 hours
	ui->lineEdit_elapsedTime->setText(QString::number(time_interval) + " seconds");
	if(i >= 720)
		ui->textEdit_output->append("With this parameters Nonce doesn't exists");
}

void BLOCK::Find(Ui::MainWindow* ui, bool b, int zeroes)
{
	bool state = b;

	if(state)
	{
        Find_Nonce(ui);
    }
	else
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
}

BLOCK::~BLOCK()
{

}
