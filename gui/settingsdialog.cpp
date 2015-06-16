#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "cmdopt.h"
#include <qfiledialog.h>

SettingsDialog::SettingsDialog(QStringList * args, QString path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
	this->setWindowTitle("Ustawienia");
	workingDirPath = path;
	dargs = args;
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_buttonBox_accepted()
{
	Opts::nodes_in_hidden1 = ui->liczbaNeuronow1SpinBox->value();
	Opts::nodes_in_hidden2 = ui->liczbaNeuronow2SpinBox->value();
	Opts::ih = ui->wysokoscSpinBox->value();
	Opts::iw = ui->szerokoscSpinBox->value();
	Opts::ann_learn_chunk_size = ui->learnChunkSpinBox->value();
	if (ui->sciezkaDaneTestoweTextEdit->toPlainText() != "" && ui->sciezkaDaneTestoweTextEdit->toPlainText() != workingDirPath)
		Opts::imgs_dir = ui->sciezkaDaneTestoweTextEdit->toPlainText();
	if (ui->sciezkaDaneUczaceTextEdit->toPlainText() != "" && ui->sciezkaDaneTestoweTextEdit->toPlainText() != workingDirPath)
		Opts::imgs_learn = ui->sciezkaDaneUczaceTextEdit->toPlainText();
	if (ui->regexTextEdit->toPlainText() != "")
		Opts::imgs_groups_regexp = ui->regexTextEdit->toPlainText();
	if (ui->logTextEdit->toPlainText() != "")
		Opts::log_file = ui->logTextEdit->toPlainText();
	if (ui->czyscPlikCheckBox->isChecked() == false)
		Opts::log_file_append = true;
}

void SettingsDialog::on_testDataButton_clicked()
{
	QString dirName = QFileDialog::getExistingDirectory(
	this, tr("Wybierz folder z danymi do testowania."), workingDirPath);
	ui->sciezkaDaneTestoweTextEdit->setText(dirName);

}

void SettingsDialog::on_trainingDataButton_clicked()
{
	QString s = QFileDialog::getExistingDirectory(
	this, tr("Wybierz folder z danymi do trenowania."), workingDirPath);
	ui->sciezkaDaneUczaceTextEdit->setText(s);
}
