#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDialog>
#include <qtextstream.h>
#include <qdebug.h>
#include "settingsdialog.h"
#include "windows.h"
#include "cmdopt.h"
#include <QDesktopServices>
#include <QGraphicsPixmapItem>
#include <qthread.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->setWindowTitle("Rozpoznawanie samochodów przy użyciu sztucznej sieci neuronowej.");
    ui->setupUi(this);
	QDir d("../../bin/vs120/x86/Release");
	workingDirPath = d.absolutePath();
	Opts::log_file = workingDirPath + "/AI.log";
	ui->sciezkaDaneUczaceTextEdit->setText(workingDirPath);
	ui->sciezkaDaneTestoweTextEdit->setText(workingDirPath);
	process = NULL;
	test = 0;
	ui->lewo->setEnabled(false);
	ui->prawo->setEnabled(false);
	ui->wyswietlWPrzegladarce->setEnabled(false);
}

MainWindow::~MainWindow()
{
	if (process != NULL)
		delete process;
    delete ui;
}

void MainWindow::on_trainingDataButton_clicked()
{
    QString s = QFileDialog::getExistingDirectory(
              this, tr("Wybierz folder z danymi do trenowania."), workingDirPath);
	ui->sciezkaDaneUczaceTextEdit->setText(s);

}

void MainWindow::on_testDataButton_clicked()
{
	QString dirName = QFileDialog::getExistingDirectory(
		this, tr("Wybierz folder z danymi do testowania."), workingDirPath);
	ui->sciezkaDaneTestoweTextEdit->setText(dirName);
}

void MainWindow::on_actionZmie_ustawienia_triggered()
{
	dialog = new SettingsDialog(&args, workingDirPath, this);
	dialog->show();
}

void MainWindow::on_pushButton_3_clicked()
{
	test = 0;
	ui->imageLabel->clear();
	images.clear();
	ui->logTextEdit->append("Rozpoczyna się uczenie sieci neuronowej.\n");

	addAllArgsFromWindow();
	addArgsForLearning();

	disableButtons();
	runProgram();
}

void MainWindow::runProgram() {
	QString qs = workingDirPath+"/AI1.exe";
	
	ui->logTextEdit->append("Ścieżka do programu:");
	ui->logTextEdit->append(qs);
	ui->logTextEdit->append("\nArgumenty:");
	for (int i = 0; i < args.size(); i++)
		ui->logTextEdit->append(args[i]);
	ui->logTextEdit->append("");
	qApp->processEvents();
	
	process = new QProcess();
	connect(
		process, SIGNAL(finished(int, QProcess::ExitStatus)),
		this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));
	process->start(qs, args);
}

void MainWindow::readWriteLog() {
	QFile file(Opts::log_file);
	if (file.open(QIODevice::ReadOnly)) {

		QTextStream in(&file);

		while (!in.atEnd()) {
			QString line = in.readLine();
			ui->logTextEdit->append(line);
		}
	}
	file.close();
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
	readWriteLog();
	delete process;
	process = NULL;
	qDebug() << exitCode << "\t" << status;
	ui->logTextEdit->append("Zakończono\n");
	if (test) {
		showImages();
		test = 0;
	}
	args.clear();
	ui->testujButton->setEnabled(true);
	ui->pushButton_3->setEnabled(true);
}

void MainWindow::onBtnCancelClicked() {
	if (process != NULL) {
		process->kill();
	}
}

void MainWindow::disableButtons() {
	ui->testujButton->setEnabled(false);
	ui->pushButton_3->setEnabled(false);

	ui->lewo->setEnabled(false);
	ui->prawo->setEnabled(false);
	ui->wyswietlWPrzegladarce->setEnabled(false);
}

void MainWindow::showImages() {

	ui->lewo->setEnabled(true);
	ui->prawo->setEnabled(true);
	ui->wyswietlWPrzegladarce->setEnabled(true);
	QDir d(".");
	path = d.absolutePath() + "/markedCarsAnn/";
	ui->logTextEdit->append(path);
	QString a = QString("%1/markedCarsAnn/").arg(d.absolutePath());
	QDir d2(a);
	images = d2.entryList();
	curPic = 2;
	QString img = images.at(2);
	ui->imageLabel->setPixmap(QPixmap(path+img).scaledToWidth(ui->imageLabel->width()));
}

void MainWindow::on_testujButton_clicked() {
	test = 1;
	ui->imageLabel->clear();
	images.clear();
	ui->logTextEdit->append("Rozpoczyna się wykrywanie samochodów przy użyciu sieci neuronowej.\n");
	addAllArgsFromWindow();
	addArgsForTesting();

	disableButtons();
	runProgram();
}

void MainWindow::addAllArgsFromWindow() {
	if (ui->liczbaNeuronow1SpinBox->value()!=500)
		Opts::nodes_in_hidden1 = ui->liczbaNeuronow1SpinBox->value();
	if (ui->liczbaNeuronow2SpinBox->value() != 0)
		Opts::nodes_in_hidden2 = ui->liczbaNeuronow2SpinBox->value();
	if (ui->wysokoscSpinBox->value() != 20)
		Opts::ih = ui->wysokoscSpinBox->value();
	if (ui->szerokoscSpinBox->value() != 50)
		Opts::iw = ui->szerokoscSpinBox->value();
	if (ui->sciezkaDaneTestoweTextEdit->toPlainText() != "" && (Opts::imgs_dir == workingDirPath || Opts::imgs_dir == "."))
		Opts::imgs_dir = ui->sciezkaDaneTestoweTextEdit->toPlainText();
	if (ui->sciezkaDaneUczaceTextEdit->toPlainText() != "" && (Opts::imgs_dir == workingDirPath || Opts::imgs_dir == "."))
		Opts::imgs_learn = ui->sciezkaDaneUczaceTextEdit->toPlainText();
}

void MainWindow::addArgsForLearning() {
	
	if (Opts::imgs_learn != "fotyUczace")
		args.push_back(QString("-imgs_learn=%1").arg(Opts::imgs_learn));
	if (Opts::imgs_dir != ".")
		args.push_back(QString("-imgs_dir=%1").arg(Opts::imgs_dir));
	if (Opts::imgs_groups_regexp != "default.*")
		args.push_back(QString("-imgs_groups_regexp=%1").arg(Opts::imgs_groups_regexp));
	else
	if (QDir(Opts::imgs_dir).exists()) {
		Opts::imgs_groups_regexp = ".*default.*$";
		args.push_back(QString("-imgs_groups_regexp=%1").arg(Opts::imgs_groups_regexp));
	}
	if (Opts::log_file_append == true)
		args.push_back(QString("-log_file_append=true"));
	
	if (Opts::nodes_in_hidden1 != 500)
		args.push_back(QString("-nodes_in_hidden1=%1").arg(Opts::nodes_in_hidden1));
	if (Opts::nodes_in_hidden2 != 0)
		args.push_back(QString("-nodes_in_hidden2=%1").arg(Opts::nodes_in_hidden2));
	if (Opts::ih != 20)
		args.push_back(QString("-ih=%1").arg(Opts::ih));
	if (Opts::iw != 50)
		args.push_back(QString("-iw=%1").arg(Opts::iw));
	if (Opts::ann_learn_chunk_size != 2 * 1024 * 1024)
		args.push_back(QString("-ann_learn_chunk_size=%1").arg(Opts::ann_learn_chunk_size));

	args.push_back(QString("-ann_learn=true"));

	if (Opts::log_file != (workingDirPath + "/AI.log"))
		args.push_back(QString("-log_file=%1").arg(workingDirPath + Opts::log_file));
	else
		args.push_back(QString("-log_file=%1").arg(Opts::log_file));

}

void MainWindow::addArgsForTesting() {
	
	if (Opts::imgs_dir!=".")
		args.push_back(QString("-imgs_dir=%1").arg(Opts::imgs_dir));
	if (Opts::imgs_groups_regexp!="default.*")
		args.push_back(QString("-imgs_groups_regexp=%1").arg(Opts::imgs_groups_regexp));
	else
		if (QDir(Opts::imgs_dir).exists()) {
			Opts::imgs_groups_regexp = ".*default.*$";
			args.push_back(QString("-imgs_groups_regexp=%1").arg(Opts::imgs_groups_regexp));
		}
	
	if (Opts::log_file != (workingDirPath + "/AI.log"))
		args.push_back(QString("-log_file=%1").arg(workingDirPath + Opts::log_file));
	else
		args.push_back(QString("-log_file=%1").arg(Opts::log_file));

	if (Opts::log_file_append!=false)
		args.push_back(QString("-log_file_append=true"));
}

void MainWindow::on_wyswietlWPrzegladarce_clicked()
{
	QDesktopServices::openUrl(QUrl(path+images.at(curPic)));
}

void MainWindow::on_prawo_clicked()
{
	if (curPic < images.size()-1) {
	QString img = images.at(++curPic);
	ui->imageLabel->setPixmap(QPixmap(path + img).scaledToWidth(ui->imageLabel->width()));
	}
}

void MainWindow::on_lewo_clicked()
{
	if (curPic >= 3) {
	QString img = images.at(--curPic);
	ui->imageLabel->setPixmap(QPixmap(path + img).scaledToWidth(ui->imageLabel->width()));
	}
}

void MainWindow::on_actionSpecyfikacja_triggered()
{
	QDir f("../../doc/");
	QString s = f.absolutePath();
	QDesktopServices::openUrl(QUrl(s+"/spec.pdf"));
}
