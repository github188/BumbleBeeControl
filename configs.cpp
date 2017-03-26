#include "configs.h"

Configs::Configs()
{
    //construct mapping between device name and type
    usbcanconfig.mapDevType.insert("VCI_PCI5121", VCI_PCI5121);
    usbcanconfig.mapDevType.insert("VCI_PCI9810", VCI_PCI9810);
    usbcanconfig.mapDevType.insert("VCI_USBCAN1", VCI_USBCAN1);
    usbcanconfig.mapDevType.insert("VCI_USBCAN2", VCI_USBCAN2);
    usbcanconfig.mapDevType.insert("VCI_USBCAN2A", VCI_USBCAN2A);
    usbcanconfig.mapDevType.insert("VCI_PCI9820", VCI_PCI9820);
    usbcanconfig.mapDevType.insert("VCI_CAN232", VCI_CAN232);
    usbcanconfig.mapDevType.insert("VCI_PCI5110", VCI_PCI5110);
    usbcanconfig.mapDevType.insert("VCI_CANLITE", VCI_CANLITE);
    usbcanconfig.mapDevType.insert("VCI_ISA9620", VCI_ISA9620);
    usbcanconfig.mapDevType.insert("VCI_ISA5420", VCI_ISA5420);
    usbcanconfig.mapDevType.insert("VCI_PC104CAN", VCI_PC104CAN);
    usbcanconfig.mapDevType.insert("VCI_CANETUDP", VCI_CANETUDP);
    usbcanconfig.mapDevType.insert("VCI_CANETE", VCI_CANETE);
    usbcanconfig.mapDevType.insert("VCI_DNP9810", VCI_DNP9810);
    usbcanconfig.mapDevType.insert("VCI_PCI9840", VCI_PCI9840);
    usbcanconfig.mapDevType.insert("VCI_PC104CAN2", VCI_PC104CAN2);
    usbcanconfig.mapDevType.insert("VCI_PCI9820I", VCI_PCI9820I);
    usbcanconfig.mapDevType.insert("VCI_CANETTCP", VCI_CANETTCP);
    usbcanconfig.mapDevType.insert("VCI_PEC9920", VCI_PEC9920);
    usbcanconfig.mapDevType.insert("VCI_PCI5010U", VCI_PCI5010U);
    usbcanconfig.mapDevType.insert("VCI_USBCAN_E_U", VCI_USBCAN_E_U);
    usbcanconfig.mapDevType.insert("VCI_USBCAN_2E_U", VCI_USBCAN_2E_U);
    usbcanconfig.mapDevType.insert("VCI_PCI5020U", VCI_PCI5020U);

    //mapping baudRate to timing1 and timing2
    usbcanconfig.baudRate_to_timers.insert("5",qMakePair(0xBF,0xFF));
    usbcanconfig.baudRate_to_timers.insert("10",qMakePair(0x31,0x1C));
    usbcanconfig.baudRate_to_timers.insert("20",qMakePair(0x18,0x1C));
    usbcanconfig.baudRate_to_timers.insert("40",qMakePair(0x87,0xFF));
    usbcanconfig.baudRate_to_timers.insert("50",qMakePair(0x09,0x1C));
    usbcanconfig.baudRate_to_timers.insert("80",qMakePair(0x83,0xFF));
    usbcanconfig.baudRate_to_timers.insert("100",qMakePair(0x04,0x1C));
    usbcanconfig.baudRate_to_timers.insert("125",qMakePair(0x03,0x1C));
    usbcanconfig.baudRate_to_timers.insert("200",qMakePair(0x81,0xFA));
    usbcanconfig.baudRate_to_timers.insert("250",qMakePair(0x01,0x1C));
    usbcanconfig.baudRate_to_timers.insert("400",qMakePair(0x80,0xFA));
    usbcanconfig.baudRate_to_timers.insert("500",qMakePair(0x00,0x1C));
    usbcanconfig.baudRate_to_timers.insert("666",qMakePair(0x80,0xB6));
    usbcanconfig.baudRate_to_timers.insert("800",qMakePair(0x00,0x16));
    usbcanconfig.baudRate_to_timers.insert("1000",qMakePair(0x00,0x14));
	//read config
	if (!status.s_haveRead)
	{
		read_configs();
	}
}

Configs::~Configs()
{

}

void Configs::write_configs()
{
    QSettings setting("cfg.ini",QSettings::IniFormat);
    //CAN Settings
    setting.beginGroup("CAN_Setting");

    setting.setValue("AccCode", usbcanconfig.acceptCode);
    setting.setValue("AccMask", usbcanconfig.mask);
    setting.setValue("Filter", usbcanconfig.filterMode);
    setting.setValue("BaudRate", usbcanconfig.baudRate);
    setting.setValue("mode", usbcanconfig.mode);
    setting.setValue("canNum", usbcanconfig.canNum);
    setting.setValue("indexNum", usbcanconfig.indexNum);
    setting.setValue("devName", usbcanconfig.devName);
    setting.setValue("devType", usbcanconfig.devType);
    setting.setValue("timing0", usbcanconfig.timing0);
    setting.setValue("timing1", usbcanconfig.timing1);

    setting.endGroup();

    //Camera Settings
    setting.beginGroup("Camera_Setting");
	
	setting.setValue("camera_width", camconfig.width);
	setting.setValue("camera_height", camconfig.height);
	setting.setValue("camera_FPS", camconfig.fps);
    setting.setValue("camera_Index", camconfig.idx);

    setting.endGroup();

    //Output Settings
    setting.beginGroup("Experiment_Setting");
    setting.setValue("BumbleId", expconfig.bumbleId);
	setting.setValue("TrainTrial", expconfig.trainTrial);
	setting.setValue("OutputBaseDir", expconfig.outputBaseDir);
    setting.endGroup();

    //Stimulus Parameter Settings
	setting.beginGroup("Parameter_Setting");
	setting.setValue("DutyCyle", expconfig.paraconfig.dutyCycle);
	setting.setValue("Frequency", expconfig.paraconfig.frequency);
	setting.setValue("Period Count", expconfig.paraconfig.periodCount);
	setting.setValue("Stimulus Count", expconfig.paraconfig.stimulusCount);
    setting.setValue("Stimulus Interval", expconfig.paraconfig.stimulusInterval);
    setting.setValue("Deriction", expconfig.paraconfig.direction);
    setting.endGroup();

}
void Configs::read_configs()
{
    if(!status.s_haveRead)
    {
        QSettings setting("cfg.ini",QSettings::IniFormat);

        //CAN Settings
        setting.beginGroup("CAN_Setting");

        usbcanconfig.acceptCode = setting.value("AccCode").toString();
        usbcanconfig.mask = setting.value("AccMask").toString();
        usbcanconfig.filterMode = setting.value("Filter").toUInt();
        usbcanconfig.baudRate = setting.value("BaudRate").toString();
        usbcanconfig.mode = setting.value("mode").toUInt();
        usbcanconfig.canNum = setting.value("canNum").toUInt();
        usbcanconfig.indexNum= setting.value("indexNum").toUInt();
        usbcanconfig.devName = setting.value("devName").toString();
        usbcanconfig.devType = setting.value("devType").toUInt();
        usbcanconfig.timing0 = setting.value("timing0").toUInt();
        usbcanconfig.timing1 = setting.value("timing1").toUInt();

        setting.endGroup();

        //Camera Settings
        setting.beginGroup("Camera_Setting");

        camconfig.width = setting.value("camera_width").toUInt();
        camconfig.height = setting.value("camera_height").toUInt();
        camconfig.fps = setting.value("camera_FPS").toUInt();
        camconfig.idx = setting.value("camera_Index").toUInt();

        setting.endGroup();

        //Output Settings
        setting.beginGroup("Experiment_Setting");

        expconfig.bumbleId = setting.value("BumbleId").toString();
        expconfig.trainTrial= setting.value("TrainTrial").toUInt();
        expconfig.outputBaseDir = setting.value("OutputBaseDir").toString();

        setting.endGroup();

        //Stimulus parameters' Settings
        setting.beginGroup("Parameter_Setting");

		expconfig.paraconfig.dutyCycle = setting.value("DutyCycle").toInt();
		expconfig.paraconfig.frequency = setting.value("Frequency").toInt();
		expconfig.paraconfig.periodCount = setting.value("Period Count").toInt();
		expconfig.paraconfig.stimulusCount = setting.value("Stimulus Count").toInt();
        expconfig.paraconfig.stimulusInterval = setting.value("Stimulus Interval").toInt();
        expconfig.paraconfig.direction= setting.value("Deriction").toInt();

        setting.endGroup();

        status.s_haveRead = true;
    }
}

Configs::Status Configs::status = {false, false, false, false, false, false};
//CAN configurations
Configs::UsbCanConfig Configs::usbcanconfig;
//Camera Configurations
Configs::CamConfig Configs::camconfig = {0, 0, 0, 0};
//Experiment Configurations
Configs::ExpConfig Configs::expconfig;
