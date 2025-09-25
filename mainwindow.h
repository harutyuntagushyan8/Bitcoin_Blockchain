#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "block_hash.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

extern uint threadCount;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void Validations();
    void DeactivateAll();
    void DeactivateCommands();
    void ResetLineEditsText();
    void appendToNonceArray(QString blockNumber, uint newValue);
    QString getFilePath(const QString &filename);
    void PrintText(BlockData, uint, std::string, std::string, std::string);

private slots:
    void readFromFile();
    void ActivateCommands();
    void Execute();
    void ClearOutput_Clicked();
    void calculate();

private:
    Ui::MainWindow* ui;
};

#endif // MAINWINDOW_H
