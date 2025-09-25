#include "mainwindow.h"
#include "utils.hpp"
#include "ui_mainwindow.h"
#include "block_hash.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>
#include <QString>
#include <QRegularExpression>
#include <QValidator>
#include <QThread>
#include <QCoreApplication>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <thread>
#include <QThread>
#include <QTimer>

uint threadCount = QThread::idealThreadCount();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(940,860);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#303030"));
    this->setPalette(palette);

    ui->label_versionHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_previousHashHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_merkleRootHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_timestampHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_timestampHint_utc->setStyleSheet("QLabel { color : blue; }");
    ui->label_bitsHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_difficultyHint->setStyleSheet("QLabel { color : blue; }");
    ui->label_real_nonceHint->setStyleSheet("QLabel { color : blue; }");

    QFont font("Courier New");
    ui->lineEdit_version->setFont(font);
    ui->lineEdit_previousHash->setFont(font);
    ui->lineEdit_merkleRoot->setFont(font);
    ui->lineEdit_timestamp->setFont(font);
    ui->lineEdit_difficulty->setFont(font);
    ui->lineEdit_bits->setFont(font);
    ui->textEdit_output->setFont(font);

    for (uint i = 1; i <= threadCount; ++i)
    {
        ui->comboBox_threads->addItem(QString::number(i));
    }

    Validations();
    DeactivateAll();

    connect(ui->pushButton_getBlockInformation, SIGNAL(clicked(bool)),
            this, SLOT(readFromFile()));
    connect(ui->pushButton_validate, SIGNAL(clicked(bool)),
            this, SLOT(ActivateCommands()));
    connect(ui->pushButton_findNonce, SIGNAL(clicked(bool)),
            this, SLOT(Execute()));
    connect(ui->pushButton_clearOutput, SIGNAL(clicked(bool)),
            this, SLOT(ClearOutput_Clicked()));
}

void MainWindow::readFromFile()
{
    if (ui->lineEdit_block->text().isEmpty() || ui->lineEdit_block->text() == "Wrong! input number")
    {
        ui->lineEdit_block->setText("Wrong! input number");
        ui->lineEdit_block->setValidator(new QIntValidator(0, 1000000, this));
        return;
    }
    else
    {
        QString filePath = getFilePath("blockchain.info.json");
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::information(0, "error", file.errorString());
        }

        // Read JSON content
        QByteArray jsonData = file.readAll();
        file.close();

        // Parse JSON
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        QJsonObject jsonObj = jsonDoc.object();
        QString blockNumber = ui->lineEdit_block->text();
        QJsonObject blockData = jsonObj[blockNumber].toObject();

        uint version = blockData["version"].toInteger();
        QString previousHash = blockData["previousHash"].toString();
        QString merkleRoot = blockData["merkleRoot"].toString();
        QString timestamp = blockData["timestamp"].toString();
        uint timestamp_utc = blockData["timestamp_utc"].toInteger();
        uint bits = blockData["bits"].toInteger();
        uint difficulty = blockData["difficulty"].toInteger();
        uint nonce = blockData["nonce"].toInteger();

        ui->lineEdit_version->setText(QString::number(version));
        ui->lineEdit_previousHash->setText(previousHash);
        ui->lineEdit_merkleRoot->setText(merkleRoot);
        ui->lineEdit_timestamp->setText(timestamp);
        ui->lineEdit_timestamp_utc->setText(QString::number(timestamp_utc));
        ui->lineEdit_bits->setText(QString::number(bits));
        ui->lineEdit_difficulty->setText(QString::number(difficulty));
        ui->lineEdit_nonce_real->setText(QString::number(nonce));

        ui->pushButton_validate->setEnabled(true);
    }
}

void MainWindow::Validations()
{
    ui->lineEdit_version->setValidator(new QRegularExpressionValidator(QRegularExpression("^([1-9][0-9]{0,6}$)"), this));
    ui->lineEdit_previousHash->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9a-fA-F]{64}$"),this));
    ui->lineEdit_merkleRoot->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9a-fA-F]{64}$"),this));
    ui->lineEdit_timestamp->setValidator(new QRegularExpressionValidator(QRegularExpression("([2]{1}[0-9]{3})\\-(0?[1-9]|1[012])\\-"
                                         "([0-2]{1}[1-9]{1}|[1][0]|[2][0]|[3][0]|[3][1])\\s(([0-1]{1}[0-9]{1})|[2][0]|[2][1]|[2][2]|[2][3])\\:"
                                         "[0-5]{1}[0-9]{1}\\:[0-5]{1}[0-9]{1}"),this));
    ui->lineEdit_timestamp_utc->setValidator(new QRegularExpressionValidator(QRegularExpression("0|([1-9][0-9]{0,9})"), this));
    ui->lineEdit_bits->setValidator(new QRegularExpressionValidator(QRegularExpression("0|([1-9][0-9]{0,9})"), this));
    ui->lineEdit_difficulty->setValidator(new QRegularExpressionValidator(QRegularExpression("[1-9]{1}[0-9]{0,19}"),this));
     ui->lineEdit_nonce_real->setValidator(new QRegularExpressionValidator(QRegularExpression("0|([1-9][0-9]{0,9})"), this));
}

void MainWindow::DeactivateAll()
{
    ui->frame_block->setEnabled(false);
    ui->pushButton_validate->setEnabled(false);
    DeactivateCommands();
}

void MainWindow::DeactivateCommands()
{
    ResetLineEditsText();
    ui->pushButton_findNonce->setEnabled(false);
    ui->label_threads->setEnabled(false);
    ui->comboBox_threads->setEnabled(false);
    ui->label_nonce->setEnabled(false);
    ui->lineEdit_nonce->setEnabled(false);
    ui->label_threads->setEnabled(false);
    ui->lineEdit_elapsedTime->setEnabled(false);
    ui->pushButton_clearOutput->setEnabled(false);
    ui->textEdit_output->setEnabled(false);
}

void MainWindow::ResetLineEditsText()
{
    if (ui->comboBox_threads->currentIndex() != 0)
    {
        ui->comboBox_threads->setCurrentIndex(0);
        ui->comboBox_threads->setItemText(0,"1");
    }
    ui->lineEdit_nonce->setText("");
    ui->lineEdit_elapsedTime->setText("");
    ui->textEdit_output->setText("");
}

void MainWindow::ActivateCommands()
{
    if(ui->lineEdit_version->hasAcceptableInput())
    {
        ui->label_versionHint->setText("Correct");
        ui->label_versionHint->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_versionHint->setText("Incorrect");
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
    if(ui->lineEdit_timestamp_utc->hasAcceptableInput())
    {
        ui->label_timestampHint_utc->setText("Correct");
        ui->label_timestampHint_utc->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_timestampHint_utc->setText("Incorrect");
        ui->label_timestampHint_utc->setStyleSheet("QLabel { color : red; }");
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
    if(ui->lineEdit_nonce_real->hasAcceptableInput())
    {
        ui->label_real_nonceHint->setText("Correct");
        ui->label_real_nonceHint->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->label_real_nonceHint->setText("Incorrect");
        ui->label_real_nonceHint->setStyleSheet("QLabel { color : red; }");
    }
    ui->lineEdit_block->setEnabled(false);
    ui->frame_block->setEnabled(false);
    ui->pushButton_validate->setEnabled(false);
    ui->pushButton_findNonce->setEnabled(true);
    ui->comboBox_threads->setEnabled(true);
    ui->label_threads->setEnabled(true);
    ui->pushButton_clearOutput->setEnabled(false);
}

void MainWindow::Execute()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        btn->setEnabled(false);
        btn->setText("Waiting...");
        btn->repaint();
    }
    ui->comboBox_threads->setEnabled(false);
    ui->comboBox_threads->repaint();
    ui->label_threads->setEnabled(false);
    ui->label_threads->repaint();
    ui->textEdit_output->setEnabled(true);
    ui->textEdit_output->repaint();

    // Delay the heavy work by 500ms (0.5 seconds)
    QTimer::singleShot(500, this, &MainWindow::calculate);
}

void MainWindow::calculate()
{
    uint version = ui->lineEdit_version->text().toUInt();
    std::string version_hex = uintToHex(version);
    ui->textEdit_output->append("Version       = " + QString::fromStdString(version_hex));
    ui->textEdit_output->append("Previous Hash = " + ui->lineEdit_previousHash->text());
    ui->textEdit_output->append("Merkle Root   = " + ui->lineEdit_merkleRoot->text() + "\n");

    uint threadCount = ui->comboBox_threads->itemText(ui->comboBox_threads->currentIndex()).toInt();
    uint range = std::numeric_limits<uint>::max() / threadCount;
    uint interval_start = 0, interval_end = range;
    std::vector<std::pair<uint, uint>> ranges;
    for (uint i = 0; i < (threadCount - 1); ++i) {
        ranges.push_back(std::make_pair(interval_start, interval_end));

        // Ensure no overflow
        if (interval_end + 1 + range > std::numeric_limits<uint>::max()) {
            interval_end = std::numeric_limits<uint>::max();
        } else {
            interval_start = interval_end + 1;
            interval_end = interval_start + range;
        }
    }
    ranges.push_back(std::make_pair(interval_start, std::numeric_limits<uint>::max()));

    BlockData data;
    data.hex_version = uintToHex(ui->lineEdit_version->text().toUInt());
    data.prev_hash = ui->lineEdit_previousHash->text().toStdString();
    data.merkle_root = ui->lineEdit_merkleRoot->text().toStdString();
    data.hex_time = uintToHex(ui->lineEdit_timestamp_utc->text().toUInt());
    data.bits = uintToHex(ui->lineEdit_bits->text().toUInt());
    data.version = ui->lineEdit_version->text().toUInt();
    data.difficulty = ui->lineEdit_difficulty->text().toUInt();
    data.bits_decimal = ui->lineEdit_bits->text().toUInt();
    data.timestamp = ui->lineEdit_timestamp_utc->text().toUInt();

    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point end;
    std::chrono::duration<double> elapsed;
    std::vector<std::unique_ptr<BLOCK>> blocks;
    blocks.reserve(threadCount);
    std::vector<std::thread> workers;
    workers.reserve(threadCount);
    for (uint i = 0; i < threadCount; i++)
    {
        data.threadId = i;
        blocks.emplace_back(std::make_unique<BLOCK>(data, ranges[i]));
        workers.emplace_back(&BLOCK::FindNonce, std::ref(*blocks[i]));
    }
    for (auto& thread : workers) {
        thread.join();  // Wait for all threads to finish
    }
    qDebug() << "All threads finished";
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    const uint duration = static_cast<uint>(elapsed.count());

    uint mHashPerSecond = 0;
    if (duration > 0) {
        const uint hashCount = ranges.back().second - ranges.front().first;
        mHashPerSecond = hashCount / (duration * 1000000);
    }
    ui->lineEdit_elapsedTime->setText("Duration = " + QString::number(duration) + " seconds " + " HashRate = " + QString::number(mHashPerSecond) + " MH/s");

    // One thread always exist
    const uint len = foundedNonces.size();
    std::string block_without_nonce = blocks[0]->getHeaderWithoutNonce();
    std::string target = blocks[0]->getTarget();
    for (uint i = 0; i < len; i++)
    {
        uint nonce = foundedNonces[i];
        std::string hex_nonce = uintToHex(nonce);
        Reverse_by_Pair(hex_nonce);
        std::string header = block_without_nonce + hex_nonce;
        std::string doubleHash = DoubleSHA256(header);
        Reverse_by_Pair(doubleHash);
        PrintText(data, nonce, header, doubleHash, target);
        appendToNonceArray(ui->lineEdit_block->text(), nonce);
    }

    ui->lineEdit_block->setEnabled(false);
    ui->pushButton_clearOutput->setEnabled(true);
    ui->pushButton_findNonce->setText("Find Nonce");
    ui->label_versionHint->setText("protocol version");
    ui->label_merkleRootHint->setText("64 hex numbers");
    ui->label_previousHashHint->setText("64 hex numbers");
    ui->label_timestampHint->setText("yyyy-MM-dd HH:mm:ss");
    ui->label_timestampHint_utc->setText("seconds since Epoch");
    ui->label_bitsHint->setText("decimal with 10 max digits");
    ui->label_difficultyHint->setText("usually big number");
    ui->label_real_nonceHint->setText("usually big number");
}

void MainWindow::ClearOutput_Clicked()
{
    ui->textEdit_output->setText("");
    ui->lineEdit_nonce->setText("");
    ui->lineEdit_elapsedTime->setText("");
    ui->lineEdit_block->setEnabled(true);
    ui->lineEdit_block->setText("");

    ui->lineEdit_version->setText("");
    ui->lineEdit_previousHash->setText("");
    ui->lineEdit_merkleRoot->setText("");
    ui->lineEdit_timestamp->setText("");
    ui->lineEdit_timestamp_utc->setText("");
    ui->lineEdit_bits->setText("");
    ui->lineEdit_difficulty->setText("");
    ui->lineEdit_nonce_real->setText("");
}

void MainWindow::PrintText(BlockData data, uint nonce, std::string header, std::string double_hash, std::string target)
{
    std::string hex_time = uintToHex(data.timestamp);
    //Reverse_by_Pair(hex_time);
    ui->textEdit_output->append("Timestamp   = " + QString::fromStdString(hex_time) + " and in decimal is " + QString::number(data.timestamp));
    std::string bits = uintToHex(data.bits_decimal);
    //Reverse_by_Pair(bits);
    ui->textEdit_output->append("Bits        = " + QString::fromStdString(bits) + " and in decimal is " + QString::number(data.bits_decimal));
    std::string hex_nonce = uintToHex(nonce);
    //Reverse_by_Pair(hex_nonce);
    ui->textEdit_output->append("Nonce       = " + QString::fromStdString(hex_nonce) + " and in decimal is " + QString::number(nonce));
    ui->textEdit_output->append("Block header data in little endian form is\n" + QString::fromStdString(header));
    ui->textEdit_output->append("Double hash = " + QString::fromStdString(double_hash));
    ui->textEdit_output->append("Target      = " + QString::fromStdString(target) + "\n");

    ui->lineEdit_nonce->setText(ui->lineEdit_nonce->text() + QString::number(nonce) + " ");
}

void MainWindow::appendToNonceArray(QString blockNumber, uint newValue)
{
    QString filePath = getFilePath("data.json");
    QFile file(filePath);
    QJsonObject rootObject;

    // Step 1: Read existing JSON file
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray fileData = file.readAll();
        qDebug() << "Existing JSON file content:\n" << fileData;
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            rootObject = jsonDoc.object();
        }
    }

    // Step 2: Get or create block object
    QJsonObject blockObject = rootObject.value(blockNumber).toObject();

    // Step 3: Get or create nonce array
    QJsonArray nonceArray = blockObject.value("nonce").toArray();

    // Step 4: Prevent duplicate nonce values
    if (!nonceArray.contains(static_cast<double>(newValue))) {
        nonceArray.append(QJsonValue(static_cast<double>(newValue)));
    } else {
        qDebug() << "Nonce already exists, skipping...";
        return;
    }

    // Step 5: Update the block object
    blockObject["nonce"] = nonceArray;
    rootObject[blockNumber] = blockObject;  // Keep existing keys

    // Step 6: Write updated JSON back to file
    qDebug() << "JSON before writing:\n" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QJsonDocument updatedJsonDoc(rootObject);
        file.write(updatedJsonDoc.toJson(QJsonDocument::Indented)); // Pretty print
        file.flush();
        file.close();
        qDebug() << "Value added successfully.";
    } else {
        qDebug() << "Failed to open file for writing.";
    }
}

QString MainWindow::getFilePath(const QString &filename) {
    // __FILE__ gives the absolute path of the current .cpp file
    QDir sourceDir(QFileInfo(__FILE__).absolutePath());
    //sourceDir.cd("../../../");  // Move up 3 levels
    return sourceDir.filePath(filename);
}

MainWindow::~MainWindow()
{
    delete ui;
}
