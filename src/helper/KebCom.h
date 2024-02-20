#include <stdlib.h>
#include "keyborad/hidkeyboard_extension.h"

class KebCom
{
private:
    /* data */
public:
    KebCom(/* args */);
    ~KebCom();
    static void mac_launch(std::string appname, HIDkeyboard keb)
    {
        uint8_t comb[] = {HID_KEY_GUI_LEFT, HID_KEY_SPACE};
        keb.sendMultipleKey(comb);
        delay(500);
        appname += "\n";
        keb.sendString(appname.c_str());
    }
    static void win_launch(std::string appname, HIDkeyboard keb)
    {
        keb.sendKey(HID_KEY_GUI_LEFT);
        delay(500);
        appname += "\n";
        keb.sendString(appname.c_str());
    }
};

KebCom::KebCom(/* args */)
{
}

KebCom::~KebCom()
{
}
