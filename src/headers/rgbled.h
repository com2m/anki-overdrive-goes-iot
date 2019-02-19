#ifndef RGBLED_H
#define RGBLED_H

#include <QObject>
#include <QTimer>
#include <wiringPi.h>
#include <softPwm.h>
#include <QColor>

class RGBLed : public QObject {
    Q_OBJECT
public:
    explicit RGBLed(int LedPinRed, int LedPinGreen, int LedPinBlue);
    ~RGBLed();
    bool setColor(QColor color); 
    QColor setColorForPeriod(QColor color, int ms);
    enum Channel {
        RED = 1,
        GREEN = 2,
        BLUE = 3
   };
    
private:
    static bool wiringPiSetupDone;
    int LEDPinRed;
    int LEDPinGreen;
    int LEDPinBlue;
    void setPWM(QColor color);
    QColor currentColor;
    QTimer* periodTimer;
    
signals:

public slots:
    void periodUpdate();
};

#endif // RGBLED_H
