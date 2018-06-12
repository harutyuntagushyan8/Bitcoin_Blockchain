#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sha2.h"
#include "block_hash.h"
#include <chrono>
#include <cmath>
#include <QString>
#include <QRegExp>
#include <QValidator>

typedef std::chrono::milliseconds ms;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),b(true),zeroes(0)
{
	ui->setupUi(this);
	BLOCK* block = new BLOCK(this);

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

	Validations();
	DeactivateAll();

	connect(ui->pushButton_hashrate, SIGNAL(clicked(bool)),
			this, SLOT(Hashrate()));
	connect(ui->pushButton_enterManually, SIGNAL(clicked(bool)),
			this, SLOT(ActivateInputs()));
	connect(ui->pushButton_confirm, SIGNAL(clicked(bool)),
			this, SLOT(ActivateCommands()));
	connect(this, SIGNAL(Initialize(Ui::MainWindow*)),
			block, SLOT(Initialize(Ui::MainWindow*)));
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
	connect(this, SIGNAL(Find(Ui::MainWindow*, bool, int)),
			block, SLOT(Find(Ui::MainWindow*, bool, int)));
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
	uint count = 100;
	uint iterations = 10000;
	double hashrate[count];
	std::string input = "Bitcoin";
	SHA256 sha;
	for(uint i = 0; i < count; i++)
	{
		auto start = std::chrono::high_resolution_clock::now();
		uint j = 0;
		unsigned int tot = 0; // number of hashes calculated
		do
		{
			sha(input);
			tot++;
			++j;
		}
		while(j < iterations);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration <double> elapsed = end - start;
		ms d = std::chrono::duration_cast<ms> (elapsed);
		double avg = iterations * 1000 / d.count();
		hashrate[i] = avg;
	 }

	 double sum = 0;
	 for(uint i = 0; i < count; i++)
		sum += hashrate[i];

	 double avg_rate = sum / count;
	 ui->lineEdit_hashrate->setText(QString::number(avg_rate/pow(10,6),'f',2) + " MH/s");
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
	ui->pushButton_findTestNonce->setEnabled(false);
	ui->label_nonce->setEnabled(false);
	ui->lineEdit_nonce->setEnabled(false);
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
	if(ui->comboBox_zeroes->currentIndex() != 0)
	{
		ui->comboBox_zeroes->setCurrentIndex(0);
		ui->comboBox_zeroes->setItemText(0,"0");
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
	if(ui->label_versionHint->text() == "Incorrect" || ui->label_previousHashHint->text() == "Incorrect" || \
		ui->label_merkleRootHint->text() == "Incorrect" || ui->label_timestampHint->text() == "Incorrect" || \
			ui->label_bitsHint->text() == "Incorrect" || ui->label_difficultyHint->text() == "Incorrect")

	{
		ui->radioButton_realCalc->setEnabled(false);
		ui->radioButton_testCalc->setEnabled(false);
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

		rb->setChecked(true);
		if(ui->radioButton_testCalc->isChecked())
		{
			ui->radioButton_testCalc->setChecked(false);
		}
		b = true;
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
		b = false;
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
	target[zeroes]   = '6';
	target[zeroes+1] = '1';
	target[zeroes+2] = 'e';
	target[zeroes+3] = '9';
	target[zeroes+4] = 'f';
	target[zeroes+5] = '8';
	ui->lineEdit_target->setText(target);
}

void MainWindow::Execute()
{
	QPushButton* btn =(QPushButton*)sender();
	if(btn->text() == "Find Real Nonce")
	{
		emit Initialize(ui);
	}
	else if(btn->text() == "Find Test Nonce")
	{
		emit Initialize(ui);
	}
	ui->textEdit_output->setEnabled(true);
	emit Find(ui, b, zeroes);
}

void MainWindow::on_pushButton_clearOutput_clicked()
{
	ui->textEdit_output->setText("");
	ui->lineEdit_nonce->setText("");
	ui->lineEdit_elapsedTime->setText("");
}

void MainWindow::on_pushButton_reset_clicked()
{
	SetLabelsText();
	DeactivateCommands();
	ui->pushButton_reset->setEnabled(false);
	ui->pushButton_confirm->setEnabled(true);
}

MainWindow::~MainWindow()
{
	delete ui;
}
