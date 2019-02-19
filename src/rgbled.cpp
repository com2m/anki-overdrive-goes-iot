#include "headers/rgbled.h"
#include <algorithm>
#include <vector>
#include <math.h>
#include <QDebug>
#include <QThread>

// #define RGBLEDWhiteTtesting
#ifdef RGBLEDWhiteTtesting
#include <termios.h>
static struct termios oldSettings;
static struct termios newSettings;
#endif

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
int luminance(int color, RGBLed::Channel c);

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
    QColor white = Qt::white;
    softPwmCreate(LEDPinRed,   currentColor.red(),   white.red()); 
    softPwmCreate(LEDPinGreen, currentColor.green(), white.green()); 
    softPwmCreate(LEDPinBlue,  currentColor.blue(),  white.blue());

#ifdef RGBLEDWhiteTtesting
    QThread::sleep(5);
    QColor color = QColor("#ffa500");  // orange
    qDebug().noquote().nospace() << "RGB LED for orange " << color << ", adjusted: R=" << luminance(color.red(), RGBLed::RED) << " G=" << luminance(color.green(), RGBLed::GREEN) << " B=" << luminance(color.blue(), RGBLed::BLUE);
    setColor(color); QThread::sleep(5);
    color = Qt::cyan;
    qDebug().noquote().nospace() << "RGB LED for cyan " << color << ", adjusted: R=" << luminance(color.red(), RGBLed::RED) << " G=" << luminance(color.green(), RGBLed::GREEN) << " B=" << luminance(color.blue(), RGBLed::BLUE);
    setColor(color); QThread::sleep(5);
    color = Qt::yellow;
    qDebug().noquote().nospace() << "RGB LED for yellow " << color << ", adjusted: R=" << luminance(color.red(), RGBLed::RED) << " G=" << luminance(color.green(), RGBLed::GREEN) << " B=" << luminance(color.blue(), RGBLed::BLUE);
    setColor(color); QThread::sleep(5);

    // qDebug().noquote().nospace() << "created RGB LED on pins R=" << LEDPinRed << ", G=" << LEDPinGreen << ", B=" << LEDPinBlue;
    qDebug().noquote().nospace() << "Try to find white value for RGB LED, use R/r  G/g  B/b  to  +/-  change color values, q to quit";
    int R = 205;
    int G = 72;
    int B = 113;
    color.setRgb(R, G, B);
    setColor(color); QThread::sleep(5);
    tcgetattr(0, &oldSettings); 
    newSettings = oldSettings; 
    newSettings.c_lflag &= ~ICANON; 
    newSettings.c_lflag &= 0 ? ECHO : ~ECHO; 
    tcsetattr(0, TCSANOW, &newSettings); 
   
    bool stopit = false;
    forever {  // use original RGB values for PWM
      char key = getchar();
      switch(key) { 
         case 'R': R = R < 255 ? R+1 : 255; break;
         case 'r': R = R >   0 ? R-1 :   0; break;
         case 'G': G = G < 255 ? G+1 : 255; break;
         case 'g': G = G >   0 ? G-1 :   0; break;
         case 'B': B = B < 255 ? B+1 : 255; break;
         case 'b': B = B >   0 ? B-1 :   0; break;
         case 'q': stopit = true; break;
      }
      if (stopit) break;
      qDebug().noquote().nospace() << "RGB LED color: R=" << R << " G=" << G << " B=" << B;
      softPwmWrite(LEDPinRed, R);
      softPwmWrite(LEDPinGreen, G);
      softPwmWrite(LEDPinBlue, B);
      QThread::sleep(0.7);
    }
    qDebug().noquote().nospace() << "Check colors for RGB LED using scaled color values just as in running program ";
    stopit = false; 
    forever {  // use scaled RGB values
      char key = getchar();
      switch(key) { 
         case 'R': R = R < 255 ? R+1 : 255; break;
         case 'r': R = R >   0 ? R-1 :   0; break;
         case 'G': G = G < 255 ? G+1 : 255; break;
         case 'g': G = G >   0 ? G-1 :   0; break;
         case 'B': B = B < 255 ? B+1 : 255; break;
         case 'b': B = B >   0 ? B-1 :   0; break;
         case 'q': stopit = true; break;
      }
      if (stopit) break;
      qDebug().noquote().nospace() << "RGB LED color: R=" << R << " G=" << G << " B=" << B;
      color.setRgb(R, G, B);  
      setColor(color);  // show adjusted colors
      QThread::sleep(0.7);
    }
#endif // RGBLEDWhiteTtesting
}

bool RGBLed::setColor(QColor color) {
    /* There are 19 predefined QColor objects: white, black, red, darkRed, green, darkGreen, blue, darkBlue, 
     * cyan, darkCyan, magenta, darkMagenta, yellow, darkYellow, gray, darkGray, lightGray, color0 and color1, 
     * accessible as members of the Qt namespace (ie. Qt::red), see https://doc.qt.io/qt-5/qt.html#GlobalColor-enum 
    */
    setPWM(color);
    currentColor = color;
    return true;
}

QColor RGBLed::setColorForPeriod(QColor color, int ms) {
    setPWM(color);
    periodTimer = new QTimer(this);
    connect(periodTimer, SIGNAL(timeout()), this, SLOT(periodUpdate()));
    periodTimer->setSingleShot(true);
    periodTimer->start(ms);
    return currentColor;
}

void RGBLed::setPWM(QColor color) {
    // qDebug().noquote().nospace() << "try to setColor() to R=" << color.red() << ", G=" << color.green() << ", B=" << color.blue();
    int red = luminance(color.red(), RGBLed::RED);
    int green = luminance(color.green(), RGBLed::GREEN);
    int blue = luminance(color.blue(), RGBLed::BLUE);
    std::vector<int> v{red, green, blue}; 
    std::vector<int>::iterator max = std::max_element(v.begin(), v.end());
    double scaleup = 255.0 / *max;   // use max brightness
    softPwmWrite(LEDPinRed, red * scaleup + 0.5);
    softPwmWrite(LEDPinGreen, green * scaleup + 0.5);
    softPwmWrite(LEDPinBlue, blue * scaleup + 0.5);
    // qDebug().noquote().nospace() << "setColor() to R=" << color.red() << ", G=" << color.green() << ", B=" << color.blue();
}

// Luminance see Jive Dadson on https://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color
// Inverse of sRGB "gamma" function. (approx 2.2)
double inv_gam_sRGB(int ic) {
    double c = ic/255.0;
    if ( c <= 0.04045 )
        return c/12.92;
    else 
        return pow(((c+0.055)/(1.055)),2.4);
}

// sRGB "gamma" function (approx 2.2)
int gam_sRGB(double v) {
    if(v<=0.0031308)
        v *= 12.92;
    else 
        v = 1.055*pow(v,1.0/2.4)-0.055;
    return int(v*255+0.5); // This is correct in C++. Other languages may not
                           // require +0.5
}

// sRGB luminance(Y) values - https://en.wikipedia.org/wiki/Relative_luminance
// Y = 0.2126R + 0.7152G + 0.0722B
// The formula reflects the luminosity function: green light contributes the most to the intensity perceived by humans, and blue light the least. 
const double rY = 0.212655;
const double gY = 0.715158;
const double bY = 0.072187;

// RGB LED luminance(Y) values; RGB was "white" at (205, 72, 113), thus the ratios are
const double rLED = 0.525641025641026;   // 205 / (205+72+113)
const double gLED = 0.184615384615385;   //  72 ...
const double bLED = 0.28974358974359;

int luminance(int color, RGBLed::Channel c) {
   double Y;
   switch (c) {
      case RGBLed::RED:   Y = rLED; break;
      case RGBLed::GREEN: Y = gLED; break;
      case RGBLed::BLUE:  Y = bLED; break;
      default: Y = 1.0; break;
   }
   return int(Y*color+0.5); 
}

/*
int luminance(int color, RGBLed::Channel c) {
   double Y;
   double norm = 1.0/rY + 1.0/gY + 1.0/bY;
   switch (c) {
      case RGBLed::RED:   Y = rY; break;
      case RGBLed::GREEN: Y = gY; break;
      case RGBLed::BLUE:  Y = bY; break;
      default: Y = 1.0; break;
   }
   // return gam_sRGB(color/255.0 * 1.0/Y/norm);  
   return int(inv_gam_sRGB(color)*255 +0.5); 
}

int luminance(int color, RGBLed::Channel c) {
   double Y;
   switch (c) {
      case RGBLed::RED:   Y = rY; break;
      case RGBLed::GREEN: Y = gY; break;
      case RGBLed::BLUE:  Y = bY; break;
      default: Y = 1.0; break;
   }
   return gam_sRGB(Y*inv_gam_sRGB(color));
}

// GRAY VALUE ("brightness")
int gray(int r, int g, int b) {
    return gam_sRGB(
            rY*inv_gam_sRGB(r) +
            gY*inv_gam_sRGB(g) +
            bY*inv_gam_sRGB(b)
    );
}
*/

void RGBLed::periodUpdate() {
    setColor(currentColor);
}


RGBLed::~RGBLed() {
    softPwmWrite(LEDPinRed, 0) ;
    softPwmWrite(LEDPinGreen, 0) ;
    softPwmWrite(LEDPinBlue, 0) ;
}
