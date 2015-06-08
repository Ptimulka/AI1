#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
	SettingsDialog(QStringList * args, QString workingDirPath, QWidget *parent = 0);
    ~SettingsDialog();

private slots:
	void on_buttonBox_accepted();

    void on_testDataButton_clicked();

    void on_trainingDataButton_clicked();

private:
    Ui::SettingsDialog *ui;
	QStringList * dargs;
	QString workingDirPath;
};

#endif // SETTINGSDIALOG_H
