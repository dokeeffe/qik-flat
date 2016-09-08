/*******************************************************************************
 * 
 *******************************************************************************/

#ifndef QIKFLAT_H
#define QIKFLAT_H

#include "defaultdevice.h"
#include "indilightboxinterface.h"
/* Firmata */
#include "firmata.h"

class QikFlat : public INDI::DefaultDevice, public INDI::LightBoxInterface
{
    public:

    QikFlat();
    virtual ~QikFlat();

    virtual bool initProperties();
    virtual void ISGetProperties (const char *dev);
    virtual bool updateProperties();

    virtual bool ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n);
    virtual bool ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n);
    virtual bool ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n);
    virtual bool ISSnoopDevice (XMLEle *root);

    protected:

    //  Generic indi device entries
    bool Connect();
    bool Disconnect();
    const char *getDefaultName();

    virtual bool saveConfigItems(FILE *fp);
    void TimerHit();

    // From Light Box
    virtual bool SetLightBoxBrightness(uint16_t value);
    virtual bool EnableLightBox(bool enable);

private:

    bool getStartupData();
    bool getStatus();
    bool getFirmwareVersion();
    bool getBrightness();

    // Device physical port
    //ITextVectorProperty PortTP;
    //IText PortT[1];

    // Firmware version
    ITextVectorProperty FirmwareTP;
    IText FirmwareT[1];

    int PortFD;
    int productID;
    uint8_t prevLightStatus;
    Firmata* sf;

};

#endif
