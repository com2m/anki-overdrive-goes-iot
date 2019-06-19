/*
   // http://www.thelinuxdaily.com/2010/05/grab-raw-keyboard-input-from-event-device-node-devinputevent/
*/
#ifndef EVENTREADER_H
#define EVENTREADER_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <QThread>
#include <QString>
#include "racecar.h"



class EventReader : public QThread
{
    Q_OBJECT
signals:
    void KeyPressed(char ch);
    
private:
  struct input_event ev[64];
  int fd, rd, value, size = sizeof (struct input_event);
  char name[256] = "Unknown";
  QString eventDevice = "/dev/input/event0";  

public:
   EventReader(const QString& = "/dev/input/event0");
   ~EventReader();
   void run();
};

#endif  /* EVENTREADER_H */
