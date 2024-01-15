#include "MFPMain.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MFPMain m;
    m.show();
    return a.exec();
}
