#include <stdlib.h>
#include "keyborad/hidkeyboard_extension.h"

class KebCom
{
private:
    static HIDkeyboard keb;
public:
    KebCom(/* args */);
    ~KebCom();
    static void init(){
        keb.begin();
    }
    static HIDkeyboard getKeb(){
        return keb;
    }
    static void mac_launch(std::string appname);
    static void win_launch(std::string appname);
};

HIDkeyboard KebCom::keb; // Define the static member variable

KebCom::KebCom(/* args */)
{
}




KebCom::~KebCom()
{
}

void KebCom::mac_launch(std::string appname)
{
    uint8_t comb[] = {HID_KEY_GUI_LEFT, HID_KEY_SPACE};
    getKeb().sendMultipleKey(comb);
    delay(500);
    appname += "\n";
    getKeb().sendString(appname.c_str());
}

void KebCom::win_launch(std::string appname)
{
    getKeb().sendKey(HID_KEY_GUI_LEFT);
    delay(500);
    appname += "\n";
    getKeb().sendString(appname.c_str());
}
