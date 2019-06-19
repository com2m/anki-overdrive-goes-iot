/*
   https://stackoverflow.com/questions/7543313/how-to-handle-keypress-events-in-a-qt-console-application
*/
#ifndef CONSOLEREADER_H
#define CONSOLEREADER_H

#include <QThread>
#include "racecar.h"

class ConsoleReader : public QThread
{
    Q_OBJECT
signals:
    void KeyPressed(char ch);
    
public:
   ConsoleReader();
   ~ConsoleReader();
   void run();
};

#endif  /* CONSOLEREADER_H */
