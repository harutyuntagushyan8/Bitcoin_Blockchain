#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
signals:
    void SigValues(Ui::MainWindow*, uint, uint);
    void SigIntervals(uint, uint);

private:
    void Validations();
    void DeactivateAll();
    void DeactivateCommands();
    void SetLabelsText();
    void ResetLineEditsText();

private slots:
    void readFromFile();
    void Hashrate();
    void SetHashrate(double);
    void ActivateInputs();
    void ActivateCommands();
    void MiddleSignal();
    void Execute();
    void SetZeroes(int zeroes);
    void ClearOutput_Clicked();
    void Reset_Clicked();
    void UpdateTime(uint);
    void PrintText(QString, uint, QString, QString, uint, QString, QString, QString);

private:
    Ui::MainWindow *ui;
    int zeroes;
};

#endif // MAINWINDOW_H
