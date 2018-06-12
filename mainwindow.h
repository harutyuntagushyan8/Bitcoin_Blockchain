#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
	void Initialize(Ui::MainWindow*);
	void Find(Ui::MainWindow*, bool, int);

private:
	void Validations();
	void DeactivateAll();
	void DeactivateCommands();
	void SetLabelsText();
	void ResetLineEditsText();

private slots:
    void Hashrate();
	void ActivateInputs();
	void ActivateCommands();
	void MiddleSignal();
	void Execute();
	void SetZeroes(int zeroes);
	void on_pushButton_clearOutput_clicked();
	void on_pushButton_reset_clicked();

private:
    Ui::MainWindow *ui;
	bool b;
	int zeroes;
};

#endif // MAINWINDOW_H
