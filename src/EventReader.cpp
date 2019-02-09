/*
   https://stackoverflow.com/questions/7543313/how-to-handle-keypress-events-in-a-qt-console-application
*/
#include "headers/EventReader.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <QProcess>

EventReader::EventReader(const QString& eventDevice)
{
   this->eventDevice = eventDevice;
   if ((getuid ()) != 0)
      qDebug().noquote().nospace() << "You are not root! This may not work ... but generally does.";

   // Open Device
   if ((fd = open (qPrintable(eventDevice), O_RDONLY)) == -1) {
      qDebug().noquote().nospace() << eventDevice << " is not a vaild device";
   }
   else {
      // Print Device Name
      ioctl (fd, EVIOCGNAME (sizeof (name)), name);
      qDebug().noquote().nospace() << "Reading keys from: " << eventDevice << " (" << name << ")";
   }
}


EventReader::~EventReader()
{
   close(fd);
}

void EventReader::run()
{
  
  /* Read keyboard and translate into car controls 
   *  Car 1/2           [tranlasted char codes in brackets]
   *     W/^   faster
   *     S/v   slower
   *     A/<   left
   *     D/>   right
   *     _/Ins fire    [ /I]
   *     F1    status  [?]
   *     F5    drive to start  [G]
   *     F8    clear road  [C]  
   *     F12   quit    [Q]
   *     P     pause
   *     0-5   speed 0%, 20%, ... 100%
   *     T     turbo speed
   *     M/m   U-turn
   *     Home(Pos 1)  Scan Track  [h]
   */
   static bool testing = false;
   static bool verbose = true;
   if (!testing) verbose = false;
    
   forever {
      if (fd != -1 && (rd = read (fd, ev, size * 64)) < size) {
          // perror_exit ("read()");
          qDebug().noquote().nospace() << "EventReader::run() - cannot read()"; 
      }
      value = ev[0].value;
      if (value != ' ' && ev[1].value == 1 && ev[1].type == EV_KEY) { // Only read the key press event
         int keycode = ev[1].code;  // .code see see https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/input-event-codes.h
            // https://www.kernel.org/doc/Documentation/input/input.txt
            //   'code' is event code, for example REL_X or KEY_BACKSPACE, again a complete list is in include/uapi/linux/input-event-codes.h.
            //   'value' is the value the event carries. Either a relative change for EV_REL, absolute new value for EV_ABS (joysticks ...), or 0 for EV_KEY for release, 1 for keypress and 2 for autorepeat.
         char key = '\0';
         if (verbose) qDebug("%d", keycode);
         switch (keycode) {
            case KEY_W: 
               if (testing) qDebug().noquote().nospace() << "<" << "w" << ">"; 
               key = 'w';
               // printf("w"); 
               break;
            case KEY_S: 
               if (testing) qDebug().noquote().nospace() << "<" << "s" << ">"; 
               key = 's';
               // printf("s"); 
               break;
            case KEY_A: 
               if (testing) qDebug().noquote().nospace() << "<" << "a" << ">"; 
               key = 'a';
               // printf("a"); 
               break;
            case KEY_D: 
               if (testing) qDebug().noquote().nospace() << "<" << "d" << ">"; 
               key = 'd';
               // printf("d"); 
               break;
            case KEY_UP: 
               if (testing) qDebug().noquote().nospace() << "<" << "arrow up" << ">"; 
               key = '^';
               // printf("^"); 
               break;
            case KEY_DOWN: 
               if (testing) qDebug().noquote().nospace() << "<" << "arrow down" << ">"; 
               key = 'v';
               // printf("v"); 
               break;
            case KEY_LEFT: 
               if (testing) qDebug().noquote().nospace() << "<" << "arrow left" << ">"; 
               key = '<';
               // printf("<-"); 
               break;
            case KEY_RIGHT: 
               if (testing) qDebug().noquote().nospace() << "<" << "arrow right" << ">"; 
               key = '>';
               // printf("->"); 
               break;
            case KEY_SPACE: 
               if (testing) qDebug().noquote().nospace() << "<" << "SPACE" << ">"; 
               key = ' ';
               // printf("SPACE"); 
               break;         
            case KEY_KP0: 
               if (testing) qDebug().noquote().nospace() << "<" << "Numpad Ins (0)" << ">"; 
               key = 'I';
               // printf("Ins"); 
               break;         
            case KEY_HOME: 
               if (verbose) qDebug().noquote().nospace() << "<" << "Numpad Home/Pos1" << ">"; 
               key = 'h';
               // printf("Home"); 
               break;
            case KEY_KP7: 
               if (testing) qDebug().noquote().nospace() << "<" << "Cursor block Home/Pos1 (7)" << ">"; 
               key = 'h';
               break;
            case KEY_F1: 
               if (testing) qDebug().noquote().nospace() << "<" << "F1" << ">"; 
               key = '?';
               // printf("F1"); 
               break;
            case KEY_F5: 
               if (testing) qDebug().noquote().nospace() << "<" << "F5" << ">"; 
               key = 'G';
               // printf("F5"); 
               break;
            case KEY_F8: 
               if (testing) qDebug().noquote().nospace() << "<" << "F8" << ">"; 
               key = 'C';
               // printf("F8"); 
               break;
            case KEY_P: 
               if (testing) qDebug().noquote().nospace() << "<" << "p" << ">"; 
               key = 'p';
               // printf("p"); 
               break;
            case KEY_0: 
               if (testing) qDebug().noquote().nospace() << "<" << "0" << ">"; 
               key = '0';
               // printf("0"); 
               break;
            case KEY_1: 
               if (testing) qDebug().noquote().nospace() << "<" << "1" << ">"; 
               key = '1';
               // printf("1"); 
               break;
            case KEY_2: 
               if (testing) qDebug().noquote().nospace() << "<" << "2" << ">"; 
               key = '2';
               // printf("2"); 
               break;
            case KEY_3: 
               if (testing) qDebug().noquote().nospace() << "<" << "3" << ">"; 
               key = '3';
               // printf("3"); 
               break;
            case KEY_4: 
               if (testing) qDebug().noquote().nospace() << "<" << "4" << ">"; 
               key = '4';
               // printf("4"); 
               break;
            case KEY_5: 
               if (testing) qDebug().noquote().nospace() << "<" << "5" << ">"; 
               key = '5';
               // printf("5"); 
               break;
            case KEY_T: 
               if (testing) qDebug().noquote().nospace() << "<" << "t" << ">"; 
               key = 't';
               // printf("t"); 
               break;
            case KEY_F12: 
               if (testing) qDebug().noquote().nospace() << "<" << "F12" << ">"; 
               key = 'Q';
               // printf("F12"); 
               break;
            case KEY_G:   // suppress g, i, q, c, h
            case KEY_I: 
            case KEY_Q: 
            case KEY_C: 
            case KEY_H: 
                key = '\0'; break;
            default: 
                key = '\0'; break;
         }
         if (testing) qDebug().noquote().nospace() << "return <" << key << ">"; 
         emit KeyPressed(key);
      }
   }
}
