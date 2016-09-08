/*******************************************************************************
 * 
 * Indi driver for a simple flat field panel (Electro lumnnicent panel) controlled by an arduino based relay
 * 
 * 
 ******************************************************************************/

#include <memory>
#include <libnova.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "indicom.h"

#include "qik_flat.h"

// We declare an auto pointer to QikFlat.
std::unique_ptr<QikFlat> qikflat(new QikFlat());

void ISGetProperties(const char *dev)
{
    qikflat->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
    qikflat->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
    qikflat->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
    qikflat->ISNewNumber(dev, name, values, names, num);
}

void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    INDI_UNUSED(dev);
    INDI_UNUSED(name);
    INDI_UNUSED(sizes);
    INDI_UNUSED(blobsizes);
    INDI_UNUSED(blobs);
    INDI_UNUSED(formats);
    INDI_UNUSED(names);
    INDI_UNUSED(n);
}
void ISSnoopDevice (XMLEle *root)
{
    qikflat->ISSnoopDevice(root);
}

QikFlat::QikFlat() : LightBoxInterface(this, true)
{
    setVersion(1,0);
    prevLightStatus = 0xFF;
}

QikFlat::~QikFlat()
{

}

bool QikFlat::initProperties()
{
    INDI::DefaultDevice::initProperties();

    // Device port
    //IUFillText(&PortT[0],"PORT","Port","/dev/ttyUSB0");
    //IUFillTextVector(&PortTP,PortT,1,getDeviceName(),"DEVICE_PORT","Ports",OPTIONS_TAB,IP_RW,60,IPS_IDLE);   

    // Firmware version
    IUFillText(&FirmwareT[0],"Version","",NULL);
    IUFillTextVector(&FirmwareTP,FirmwareT,1,getDeviceName(),"Firmware","",MAIN_CONTROL_TAB,IP_RO,60,IPS_IDLE);

    initLightBoxProperties(getDeviceName(), MAIN_CONTROL_TAB);

    LightIntensityN[0].min  = 0;
    LightIntensityN[0].max  = 255;
    LightIntensityN[0].step = 10;

    setDriverInterface(AUX_INTERFACE | LIGHTBOX_INTERFACE);
    addDebugControl();
    return true;
}

void QikFlat::ISGetProperties (const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);

    //defineText(&PortTP);
    //loadConfig(true, "DEVICE_PORT");

    // Get Light box properties
    isGetLightBoxProperties(dev);
}

bool QikFlat::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        defineSwitch(&LightSP);
        defineNumber(&LightIntensityNP);
        //defineText(&StatusTP);
        defineText(&FirmwareTP);        

        updateLightBoxProperties();

        getStartupData();
    }
    else
    {
        deleteProperty(LightSP.name);
        deleteProperty(LightIntensityNP.name);
        deleteProperty(FirmwareTP.name);

        updateLightBoxProperties();
    }

    return true;
}


const char * QikFlat::getDefaultName()
{
    return (char *)"Qik Flat";
}

/**
 * Connect to the usb device, iterate over the first 5 /dev/ttyACMx devices (this assumes the device is a genuine arduino and appears as /dev/ttyACMx)
 * 
 **/
bool QikFlat::Connect()
{
    for( int a = 0; a < 5; a = a + 1 )
    {
    	string usbPort = "/dev/ttyACM" +  std::to_string(a);
    	DEBUGF(INDI::Logger::DBG_SESSION, "Attempting connection %s",usbPort.c_str());
        sf = new Firmata(usbPort.c_str());
        if (sf->portOpen && strstr(sf->firmata_name, "LightPanelSwitcher")) {
    	    DEBUG(INDI::Logger::DBG_SESSION, "ARDUINO BOARD CONNECTED.");
	    DEBUGF(INDI::Logger::DBG_SESSION, "FIRMATA VERSION:%s",sf->firmata_name);
	    return true;
        } else {
            DEBUG(INDI::Logger::DBG_SESSION,"Failed, trying next port.\n");
        }
    }
    DEBUG(INDI::Logger::DBG_SESSION, "ARDUINO BOARD FAIL TO CONNECT");
    delete sf;
    return false;
}




bool QikFlat::Disconnect()
{
    sf->closePort();
    delete sf;	
    DEBUG(INDI::Logger::DBG_SESSION, "ARDUINO BOARD DISCONNECTED.");
    IDSetSwitch (getSwitch("CONNECTION"),"DISCONNECTED\n");
    return true;
}

bool QikFlat::ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
    if (processLightBoxNumber(dev, name, values, names, n))
        return true;

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool QikFlat::ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        if (processLightBoxText(dev, name, texts, names, n))
            return true;

       // if (!strcmp(PortTP.name, name))
       // {
        //    IUUpdateText(&PortTP, texts, names, n);
        //    PortTP.s = IPS_OK;
        //    IDSetText(&PortTP, NULL);
        //    return true;
        //}        
    }

    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool QikFlat::ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        if (processLightBoxSwitch(dev, name, states, names, n))
            return true;
    }

    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool QikFlat::ISSnoopDevice (XMLEle *root)
{
    snoopLightBox(root);

    return INDI::DefaultDevice::ISSnoopDevice(root);
}

bool QikFlat::saveConfigItems(FILE *fp)
{
    //IUSaveConfigText(fp, &PortTP);
    return saveLightBoxConfigItems(fp);
}

bool QikFlat::getStartupData()
{
    bool rc1 = getFirmwareVersion();
    bool rc2 = getBrightness();

    return (rc1 && rc2);
}


bool QikFlat::EnableLightBox(bool enable)
{    
    if(enable) {
        DEBUG(INDI::Logger::DBG_SESSION, "Switching on light panel");
        sf->sendStringData((char *)"ON");
    } else {
        DEBUG(INDI::Logger::DBG_SESSION, "Switching off light panel");
        sf->sendStringData((char *)"OFF");
    }
}

bool QikFlat::getStatus()
{    
    DEBUG(INDI::Logger::DBG_SESSION, "getStatus");
    return true;
}

bool QikFlat::getFirmwareVersion()
{
    DEBUG(INDI::Logger::DBG_SESSION, "GetFW version");
    IUSaveText(&FirmwareT[0], sf->firmata_name);
    IDSetText(&FirmwareTP, NULL);
    return true;
}

void QikFlat::TimerHit()
{
    DEBUG(INDI::Logger::DBG_SESSION, "TimerHit");
    if (isConnected() == false) {
        return;
    }
    getStatus();
    SetTimer(1000);
}

bool QikFlat::getBrightness()
{    
    return true;
}

bool QikFlat::SetLightBoxBrightness(uint16_t value)
{
    DEBUG(INDI::Logger::DBG_SESSION, "SetLightBoxBrightness called: Not supported");
    //not supported, just return true... I think
    return true;
}
