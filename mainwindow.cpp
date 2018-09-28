#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sha2.h"
#include "hash.h"
#include "block_hash.h"
#include <chrono>
#include <cmath>
#include <QString>
#include <QRegExp>
#include <QValidator>
#include <QThread>
#include <iostream>
#include <memory>

uint threadCount = QThread::idealThreadCount();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MainWindow), zeroes(0)
{
    ui->setupUi(this);
    this->setFixedSize(940,860);

    ui->label_versionHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_previousHashHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_merkleRootHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_timestampHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_bitsHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_difficultyHint->setStyleSheet("QLabel { color : blue; }");

    ui->radioButton_version1->setCheckable(true);
    ui->radioButton_version2->setCheckable(true);
    ui->radioButton_realCalc->setCheckable(true);
    ui->radioButton_testCalc->setCheckable(true);

    for (uint i = 1; i <= threadCount; ++i)
    {
        ui->comboBox_threads->addItem(QString::number(i));
    }

    Validations();
    DeactivateAll();

    connect(ui->pushButton_hashrate, SIGNAL(clicked(bool)),
            this, SLOT(Hashrate()));
    connect(ui->pushButton_enterManually, SIGNAL(clicked(bool)),
            this, SLOT(ActivateInputs()));
    connect(ui->pushButton_reset, SIGNAL(clicked(bool)),
            this, SLOT(Reset_Clicked()));
    connect(ui->pushButton_confirm, SIGNAL(clicked(bool)),
            this, SLOT(ActivateCommands()));
    connect(ui->radioButton_realCalc, SIGNAL(clicked(bool)),
            this, SLOT(MiddleSignal()));
    connect(ui->radioButton_testCalc, SIGNAL(clicked(bool)),
            this, SLOT(MiddleSignal()));
    connect(ui->comboBox_zeroes, SIGNAL(activated(int)),
            this, SLOT(SetZeroes(int)));
    connect(ui->pushButton_findRealNonce, SIGNAL(clicked(bool)),
            this, SLOT(Execute()));
    connect(ui->pushButton_findTestNonce, SIGNAL(clicked(bool)),
            this, SLOT(Execute()));
    connect(ui->pushButton_clearOutput, SIGNAL(clicked(bool)),
            this, SLOT(ClearOutput_Clicked()));
}

void MainWindow::Validations()
{
    ui->lineEdit_previousHash->setValidator(new QRegExpValidator(QRegExp("[0-9a-f]{64}"),this));
    ui->lineEdit_merkleRoot->setValidator(new QRegExpValidator(QRegExp("[0-9a-f]{64}"),this));
    ui->lineEdit_timestamp->setValidator(new QRegExpValidator(QRegExp("([2]{1}[0-9]{3})\\-(0?[1-9]|1[012])\\-"
                                         "([0-2]{1}[1-9]{1}|[3][0]|[3][1])\\s(([0-1]{1}[0-9]{1})|[2][0]|[2][1]|[2][2]|[2][3])\\:"
                                         "[0-5]{1}[0-9]{1}\\:[0-5]{1}[0-9]{1}"),this));
    ui->lineEdit_bits->setValidator(new QRegExpValidator(QRegExp("[1-9]{1}[0-9]{,9}"),this));
    ui->lineEdit_difficulty->setValidator(new QRegExpValidator(QRegExp("[1-9]{1}[0-9]{,20}"),this));
}

void MainWindow::Hashrate()
{
    uint range = 50000000 / threadCount;
    uint interval_start = 0, interval_end = range;
    std::vector<std::pair<uint, uint>> ranges;
    for(uint i = 0; i < (threadCount - 1) ; ++i) {
        ranges.push_back(std::make_pair(interval_start, interval_end));
        interval_start += range;
        interval_end += range;
    }
    ranges.push_back(std::make_pair(interval_start, 50000000));

    std::vector<HASH*> hashes;
    for(uint i = 0; i < threadCount; ++i)
    {
        hashes.push_back(new HASH);
    }

    for(uint i = 0; i < threadCount; ++i)
    {
        connect(this, SIGNAL(SigIntervals(uint, uint)),
                hashes[i], SLOT(ReceiveIntervals(uint, uint)));
    }

    std::vector<QThread*> threads;
    for(uint i = 0; i < threadCount; ++i)
    {
        threads.push_back(new QThread);
        emit SigIntervals(ranges[i].first, ranges[i].second);
        hashes[i]->moveToThread(threads[i]);

        connect(threads[i], SIGNAL(started()),
                hashes[i], SLOT(Start()));
        connect(hashes[i], SIGNAL(finished()),
                threads[i], SLOT(quit()));
        connect(hashes[i], SIGNAL(finished()),
                hashes[i], SLOT(deleteLater()));
        connect(threads[i], SIGNAL(finished()),
                threads[i], SLOT(deleteLater()));
        connect(hashes[i], SIGNAL(HashrateReady(double)),
                this, SLOT(SetHashrate(double)));
        threads[i]->start();
    }
}

void MainWindow::SetHashrate(double result)
{
    double x = 0.0;
    if (ui->lineEdit_hashrate->text().isEmpty())
    {
        x = result;
        ui->lineEdit_hashrate->setText(QString::number(x, 'f', 4) + " MH/s");
    }
    else
    {
        QString s = ui->lineEdit_hashrate->text();
        s.chop(5);
        x = result + s.toDouble();
        ui->lineEdit_hashrate->setText(QString::number(x, 'f', 4) + " MH/s");
    }
}

void MainWindow::DeactivateAll()
{
    ui->frame_block->setEnabled(false);
    ui->pushButton_confirm->setEnabled(false);
    ui->pushButton_reset->setEnabled(false);
    DeactivateCommands();
}

void MainWindow::DeactivateCommands()
{
    ResetLineEditsText();
    ui->radioButton_realCalc->setEnabled(false);
    ui->radioButton_testCalc->setEnabled(false);
    ui->label_zeroes->setEnabled(false);
    ui->comboBox_zeroes->setEnabled(false);
    ui->label_target->setEnabled(false);
    ui->lineEdit_target->setEnabled(false);
    ui->pushButton_findRealNonce->setEnabled(false);
    ui->label_threads->setEnabled(false);
    ui->comboBox_threads->setEnabled(false);
    ui->pushButton_findTestNonce->setEnabled(false);
    ui->label_nonce->setEnabled(false);
    ui->lineEdit_nonce->setEnabled(false);
    ui->label_threads->setEnabled(false);
    ui->comboBox_zeroes->setEnabled(false);
    ui->label_elapsedTime->setEnabled(false);
    ui->lineEdit_elapsedTime->setEnabled(false);
    ui->pushButton_clearOutput->setEnabled(false);
    ui->textEdit_output->setEnabled(false);
}

void MainWindow::SetLabelsText()
{
    ui->label_versionHint->setText("");
    ui->label_previousHashHint->setText("64 hex numbers");
    ui->label_merkleRootHint->setText("64 hex numbers");
    ui->label_timestampHint->setText("yyyy-MM-dd HH::mm:ss");
    ui->label_bitsHint->setText("decimal with 10 max digits");
    ui->label_difficultyHint->setText("usually big number");
}

void MainWindow::ResetLineEditsText()
{
    ui->radioButton_realCalc->setChecked(false);
    ui->radioButton_testCalc->setChecked(false);
    if (ui->comboBox_zeroes->currentIndex() != 0)
    {
        ui->comboBox_zeroes->setCurrentIndex(0);
        ui->comboBox_zeroes->setItemText(0,"0");
    }
    if (ui->comboBox_threads->currentIndex() != 0)
    {
        ui->comboBox_threads->setCurrentIndex(0);
        ui->comboBox_threads->setItemText(0,"1");
    }
    ui->lineEdit_target->setText("");
    ui->lineEdit_nonce->setText("");
    ui->lineEdit_elapsedTime->setText("");
    ui->textEdit_output->setText("");
}

void MainWindow::ActivateInputs()
{
    if(!ui->pushButton_confirm->isChecked())
    {
        ui->frame_block->setEnabled(true);
        ui->pushButton_reset->setEnabled(true);
        SetLabelsText();
    }
    ui->pushButton_confirm->setEnabled(true);
    ui->pushButton_reset->setEnabled(false);
    DeactivateCommands();
}

void MainWindow::ActivateCommands()
{
    if(ui->radioButton_version1->isChecked() || ui->radioButton_version2->isChecked())
    {
        ui->label_versionHint->setText("Correct");
        ui->label_versionHint->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_versionHint->setText("Choose version");
        ui->label_versionHint->setStyleSheet("QLabel { color : red; }");
    }
    if(ui->lineEdit_previousHash->hasAcceptableInput())
    {
        ui->label_previousHashHint->setText("Correct");
        ui->label_previousHashHint->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_previousHashHint->setText("Incorrect");
        ui->label_previousHashHint->setStyleSheet("QLabel { color : red; }");
    }
    if(ui->lineEdit_merkleRoot->hasAcceptableInput())
    {
        ui->label_merkleRootHint->setText("Correct");
        ui->label_merkleRootHint->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_merkleRootHint->setText("Incorrect");
        ui->label_merkleRootHint->setStyleSheet("QLabel { color : red; }");
    }
    if(ui->lineEdit_timestamp->hasAcceptableInput())
    {
        ui->label_timestampHint->setText("Correct");
        ui->label_timestampHint->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_timestampHint->setText("Incorrect");
        ui->label_timestampHint->setStyleSheet("QLabel { color : red; }");
    }
    if(ui->lineEdit_bits->hasAcceptableInput())
    {
        ui->label_bitsHint->setText("Correct");
        ui->label_bitsHint->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_bitsHint->setText("Incorrect");
        ui->label_bitsHint->setStyleSheet("QLabel { color : red; }");
    }
    if(ui->lineEdit_difficulty->hasAcceptableInput())
    {
        ui->label_difficultyHint->setText("Correct");
        ui->label_difficultyHint->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_difficultyHint->setText("Incorrect");
        ui->label_difficultyHint->setStyleSheet("QLabel { color : red; }");
    }
    ui->frame_block->setEnabled(false);
    if(ui->label_versionHint->text() == "Incorrect" || ui->label_previousHashHint->text() == "Incorrect" ||  \
         ui->label_merkleRootHint->text() == "Incorrect" || ui->label_timestampHint->text() == "Incorrect" || \
           ui->label_bitsHint->text() == "Incorrect" || ui->label_difficultyHint->text() == "Incorrect" ||  \
             ui->label_versionHint->text() == "Choose version")
    {
        ui->radioButton_realCalc->setEnabled(false);
        ui->radioButton_testCalc->setEnabled(false);
		ui->pushButton_reset->setEnabled(false);
    }
    else
    {
        ui->pushButton_confirm->setEnabled(false);
        ui->pushButton_reset->setEnabled(true);
        ui->radioButton_realCalc->setEnabled(true);
        ui->radioButton_testCalc->setEnabled(true);
    }
}

void MainWindow::MiddleSignal()
{
    QRadioButton* rb = (QRadioButton*)sender();
    if(rb->text() == "Real Calculation")
    {
        ui->label_zeroes->setEnabled(false);
        ui->comboBox_zeroes->setEnabled(false);
        ui->label_target->setEnabled(false);
        ui->lineEdit_target->setEnabled(false);
        ui->pushButton_findTestNonce->setEnabled(false);
        ui->pushButton_findRealNonce->setEnabled(true);
        ui->label_nonce->setEnabled(true);
        ui->lineEdit_nonce->setEnabled(true);
        ui->label_elapsedTime->setEnabled(true);
        ui->lineEdit_elapsedTime->setEnabled(true);
        ui->pushButton_clearOutput->setEnabled(true);
        ui->label_threads->setEnabled(true);
        ui->comboBox_threads->setEnabled(true);

        rb->setChecked(true);
        if(ui->radioButton_testCalc->isChecked())
        {
            ui->radioButton_testCalc->setChecked(false);
        }
    }
    else if(rb->text() == "Test with less target")
    {
        ui->pushButton_findRealNonce->setEnabled(false);
        ui->label_zeroes->setEnabled(true);
        ui->label_target->setEnabled(true);
        ui->lineEdit_target->setEnabled(true);
        ui->pushButton_findTestNonce->setEnabled(true);
        ui->label_nonce->setEnabled(true);
        ui->lineEdit_nonce->setEnabled(true);
        ui->label_elapsedTime->setEnabled(true);
        ui->lineEdit_elapsedTime->setEnabled(true);
        ui->pushButton_clearOutput->setEnabled(true);
        ui->label_threads->setEnabled(false);
        ui->comboBox_threads->setEnabled(false);
        ui->label_threads->setEnabled(false);
        ui->comboBox_threads->setEnabled(false);

        rb->setChecked(true);
        if(ui->radioButton_realCalc->isChecked())
        {
            ui->radioButton_realCalc->setChecked(false);
        }

        {
            uint zeroes = ui->comboBox_zeroes->itemText(ui->comboBox_zeroes->currentIndex()).toInt();
            SetZeroes(zeroes);
        }
        ui->comboBox_zeroes->setEnabled(true);
    }
}

void MainWindow::SetZeroes(int zeroes)
{
    this->zeroes = zeroes;
    QString target = "61e9f800000000000000000000000000"
					 "00000000000000000000000000000000";
    for(int i = 0; i < zeroes; i++)
    {
        target[i] = '0';
    }
    target[zeroes]     = '6';
    target[zeroes + 1] = '1';
    target[zeroes + 2] = 'e';
    target[zeroes + 3] = '9';
    target[zeroes + 4] = 'f';
    target[zeroes + 5] = '8';
    ui->lineEdit_target->setText(target);
}

void MainWindow::Execute()
{
    QPushButton* btn = (QPushButton*)sender();
    if (btn->text() == "Find Real Nonce")
    {
        uint threadCount = ui->comboBox_threads->itemText(ui->comboBox_threads->currentIndex()).toInt();
        std::cout << "Number of threads is\n " << threadCount << "\n";
        uint range = std::numeric_limits<uint>::max() / threadCount;
        uint interval_start = 0, interval_end = range;
        std::vector<std::pair<uint, uint>> ranges;
        for(uint i = 0; i < (threadCount - 1) ; ++i) {
            ranges.push_back(std::make_pair(interval_start, interval_end));
            interval_start += range;
            interval_end += range;
        }
        ranges.push_back(std::make_pair(interval_start, std::numeric_limits<uint>::max()));

        std::vector<BLOCK*> obj;
        for(uint i = 0; i < threadCount; ++i)
        {
            obj.push_back(new BLOCK);
        }
        for(uint i = 0; i < threadCount; ++i)
        {
            connect(this, SIGNAL(SigValues(Ui::MainWindow*, uint, uint)),
                    obj[i], SLOT(ReceiveValues(Ui::MainWindow*, uint, uint)));
        }

        std::vector<QThread*> threads;
        for(uint i = 0; i < threadCount; ++i)
        {
            threads.push_back(new QThread);
            emit SigValues(ui, ranges[i].first, ranges[i].second);
            obj[i]->moveToThread(threads[i]);

            connect(threads[i], SIGNAL(started()),
                    obj[i], SLOT(StartFind()));
            connect(obj[i], SIGNAL(finished()),
                    threads[i], SLOT(quit()));
            connect(obj[i], SIGNAL(finished()),
                    obj[i], SLOT(deleteLater()));
            connect(threads[i], SIGNAL(finished()),
                    threads[i], SLOT(deleteLater()));
            connect(obj[i], SIGNAL(Nonce(QString, uint, QString, QString, uint, QString, QString, QString)),
					this, SLOT(PrintText(QString, uint, QString, QString, uint, QString, QString, QString)));
			connect(obj[i], SIGNAL(PartialTime(uint)),
					this, SLOT(UpdateTime(uint)));
            threads[i]->start();
        }
        btn->setEnabled(false);
        btn->setText("Waiting");
        ui->comboBox_threads->setEnabled(false);
        ui->label_threads->setEnabled(false);
    }
    if (btn->text() == "Find Test Nonce")
    {
        BLOCK block;
        block.Initialize(ui);
        block.Find_Test(ui, zeroes);
    }
    ui->textEdit_output->setEnabled(true);
}

void MainWindow::ClearOutput_Clicked()
{
    ui->textEdit_output->setText("");
    ui->lineEdit_nonce->setText("");
    ui->lineEdit_elapsedTime->setText("");
}

void MainWindow::Reset_Clicked()
{
    SetLabelsText();
    DeactivateCommands();
    ui->pushButton_reset->setEnabled(false);
    ui->pushButton_confirm->setEnabled(true);
}

void MainWindow::UpdateTime(uint p_time)
{
    ui->lineEdit_elapsedTime->setText(QString::number(p_time / threadCount + ui->lineEdit_elapsedTime->text().toInt()));
}

void MainWindow::PrintText(QString hex_time, uint timestamp, QString bits, QString hex_nonce, uint nonce,
                           QString header, QString double_hash, QString target)
{
    ui->textEdit_output->append("Block header parts in little endian form are");
    if(ui->radioButton_version1->isChecked())
        ui->textEdit_output->append("Version = 01000000");
    if(ui->radioButton_version2->isChecked())
        ui->textEdit_output->append("Version = 02000000");
    std::string prev_hash = ui->lineEdit_previousHash->text().toStdString();
    Reverse_by_Pair(prev_hash);
    ui->textEdit_output->append("Previous Hash = " + QString::fromStdString(prev_hash));
    std::string merkle_root = ui->lineEdit_merkleRoot->text().toStdString();
    Reverse_by_Pair(merkle_root);
    ui->textEdit_output->append("Merkle Root = " + QString::fromStdString(merkle_root));
    ui->textEdit_output->append("Timestamp = " + hex_time + " and in decimal is " + QString::number(timestamp));
    ui->textEdit_output->append("Bits = " + bits + " for test target ");
    ui->textEdit_output->append("Nonce = " + hex_nonce + " and in decimal is " + QString::number(nonce));
    ui->textEdit_output->append("Block header data in little endian form is " + header);
    ui->textEdit_output->append("Double hash is   " + double_hash);
    ui->textEdit_output->append("Current target is " + target);
    ui->lineEdit_nonce->setText(ui->lineEdit_nonce->text() + QString::number(nonce) + " ");
}

MainWindow::~MainWindow()
{
    delete ui;
}
