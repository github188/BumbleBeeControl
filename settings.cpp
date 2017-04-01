#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Settings)
{
	_configsPtr = new Configs();
	ui->setupUi(this);
	read_config_to_dialog();
}

Settings::~Settings()
{
	delete ui;
}

void Settings::read_config_to_dialog()
{
	if(Configs::status.s_haveRead)
	{
		ui->acceptCode_input->setText(Configs::usbcanconfig.acceptCode);
		ui->baudRate_combobox->setCurrentText(Configs::usbcanconfig.baudRate);
		ui->canNum_combobox->setCurrentIndex(Configs::usbcanconfig.canNum);
		ui->deviceType_combobox->setCurrentText(Configs::usbcanconfig.devName);
		ui->filterMode_combobox->setCurrentIndex(Configs::usbcanconfig.filterMode);
		ui->indexNum_combobox->setCurrentIndex(Configs::usbcanconfig.indexNum);
		ui->mask_input->setText(Configs::usbcanconfig.mask);
		ui->bumblebeeNumEdit->setText(Configs::expconfig.bumbleId);
		ui->bumblebeeTrainTrialEdit->setText(QString::number(Configs::expconfig.trainTrial));
		ui->dataSavePathEdit->setText(Configs::expconfig.outputBaseDir);
	}
	else
	{
		_configsPtr->read_configs();
	}
}

void Settings::on_saveAndCancel_buttom_accepted()
{
	//Usbcan Setting
	Configs::usbcanconfig.acceptCode = ui->acceptCode_input->text();
	Configs::usbcanconfig.mask = ui->mask_input->text();
	Configs::usbcanconfig.filterMode = quint32(ui->filterMode_combobox->currentIndex());
	Configs::usbcanconfig.mode = quint32(ui->mode_combobox->currentIndex());
	Configs::usbcanconfig.baudRate = ui->baudRate_combobox->currentText();
	Configs::usbcanconfig.devType = Configs::usbcanconfig.mapDevType.value(ui->deviceType_combobox->currentText());
	Configs::usbcanconfig.devName = ui->deviceType_combobox->currentText();
	Configs::usbcanconfig.indexNum = quint32(ui->indexNum_combobox->currentIndex());
	Configs::usbcanconfig.canNum = quint32(ui->canNum_combobox->currentIndex());
	//convert baudRate to timers
	Configs::usbcanconfig.timing0 = Configs::usbcanconfig.baudRate_to_timers.value(ui->baudRate_combobox->currentText()).first;
	Configs::usbcanconfig.timing1 = Configs::usbcanconfig.baudRate_to_timers.value(ui->baudRate_combobox->currentText()).second;

	//Experiment Setting
	//TODO加入中文支持
	Configs::expconfig.bumbleId = ui->bumblebeeNumEdit->text();
	Configs::expconfig.trainTrial = ui->bumblebeeTrainTrialEdit->text().toUInt();
	Configs::expconfig.outputBaseDir = ui->dataSavePathEdit->text();

	_configsPtr->write_configs();
}

void Settings::on_getSavePathDirToolButton_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	ui->dataSavePathEdit->setText(dir);
}
