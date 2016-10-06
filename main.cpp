#include <QCoreApplication>
#include "bluetoothcontroller.h"
#include "racecar.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Racecar *racecar = new Racecar();

    BluetoothController *controller = new BluetoothController(racecar);

    return a.exec();
}
