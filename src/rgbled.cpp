#include "headers/rgbled.h"
#include <QDebug>


/*
	pi@raspberrypi:~/Projects/qtgpio $ gpio readall
	 +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
	 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
	 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
	 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
	 |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5v      |     |     |
	 |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
	 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 0 | IN   | TxD     | 15  | 14  |
	 |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
	 |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
	 |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
	 |  22 |   3 | GPIO. 3 |  OUT | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
	 |     |     |    3.3v |      |   | 17 || 18 | 0 | OUT  | GPIO. 5 | 5   | 24  |
	 |  10 |  12 |    MOSI |  OUT | 0 | 19 || 20 |   |      | 0v      |     |     |
	 |   9 |  13 |    MISO |  OUT | 1 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
	 |  11 |  14 |    SCLK |   IN | 0 | 23 || 24 | 0 | OUT  | CE0     | 10  | 8   |
	 |     |     |      0v |      |   | 25 || 26 | 1 | IN   | CE1     | 11  | 7   |
	 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
	 |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
	 |   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | OUT  | GPIO.26 | 26  | 12  |
	 |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
	 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | OUT  | GPIO.27 | 27  | 16  |
	 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
	 |     |     |      0v |      |   | 39 || 40 | 0 | OUT  | GPIO.29 | 29  | 21  |
	 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
	 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
	 +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
*/

bool RGBLed::wiringPiSetupDone = false;

RGBLed::RGBLed(int LedPinRed, int LedPinGreen, int LedPinBlue) {
    if (!wiringPiSetupDone) {
        wiringPiSetup();
        wiringPiSetupDone = true;
        // qDebug().noquote().nospace() << "wiringPiSetup() done";

    }
    currentColor = Qt::black;
    LEDPinRed = LedPinRed;
    LEDPinGreen = LedPinGreen;
    LEDPinBlue = LedPinBlue;

    softPwmCreate(LEDPinRed,   currentColor.red(),   255); 
    softPwmCreate(LEDPinGreen, currentColor.green(), 255); 
    softPwmCreate(LEDPinBlue,  currentColor.blue(),  255);

    // qDebug().noquote().nospace() << "created RGB LED on pins R=" << LEDPinRed << ", G=" << LEDPinGreen << ", B=" << LEDPinBlue;
}

bool RGBLed::setColor(QColor color) {
    
    /* There are 19 predefined QColor objects: white, black, red, darkRed, green, darkGreen, blue, darkBlue, 
     * cyan, darkCyan, magenta, darkMagenta, yellow, darkYellow, gray, darkGray, lightGray, color0 and color1, 
     * accessible as members of the Qt namespace (ie. Qt::red).
    */
    // qDebug().noquote().nospace() << "try to setColor() to R=" << color.red() << ", G=" << color.green() << ", B=" << color.blue();
    currentColor = color;
    softPwmWrite(LEDPinRed, color.red());
    softPwmWrite(LEDPinGreen, color.green());
    softPwmWrite(LEDPinBlue, color.blue());
    // qDebug().noquote().nospace() << "setColor() to R=" << color.red() << ", G=" << color.green() << ", B=" << color.blue();
    return true;
}

QColor RGBLed::setColorForPeriod(QColor color, int ms) {

    softPwmWrite(LEDPinRed, color.red());
    softPwmWrite(LEDPinGreen, color.green());
    softPwmWrite(LEDPinBlue, color.blue());

    periodTimer = new QTimer(this);
    connect(periodTimer, SIGNAL(timeout()), this, SLOT(periodUpdate()));
    periodTimer->setSingleShot(true);
    periodTimer->start(ms);
    return currentColor;
}

void RGBLed::periodUpdate() {
    setColor(currentColor);
}


RGBLed::~RGBLed() {
    softPwmWrite(LEDPinRed, 0) ;
    softPwmWrite(LEDPinGreen, 0) ;
    softPwmWrite(LEDPinBlue, 0) ;
}
