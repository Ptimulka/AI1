#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qstring.h>
#include <qdir.h>
#include "settingsdialog.h"
#include <qprocess.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	QStringList args;

private slots:
    void on_trainingDataButton_clicked();
	void on_testDataButton_clicked();
	void on_actionZmie_ustawienia_triggered();
    void on_pushButton_3_clicked();
	void runProgram();
    void on_testujButton_clicked();
	void addAllArgsFromWindow();
	void showImages();
	void addArgsForTesting();
    void addArgsForLearning();
    void on_wyswietlWPrzegladarce_clicked();
    void on_prawo_clicked();
    void on_lewo_clicked();
    void on_actionSpecyfikacja_triggered();
	void onProcessFinished(int exitCode, QProcess::ExitStatus status);
	void onBtnCancelClicked();
	void readWriteLog();
	void disableButtons();

private:
    Ui::MainWindow *ui;
	QDir dir;
	QString workingDirPath;
    SettingsDialog * dialog;
	QString logName;
	QProcess *process;
	QString path;
	QStringList images;
	QString markedCars;
	int curPic;
	bool test;
};

#endif // MAINWINDOW_H
