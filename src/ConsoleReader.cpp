/*
   https://stackoverflow.com/questions/7543313/how-to-handle-keypress-events-in-a-qt-console-application
*/
#include "headers/ConsoleReader.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <QProcess>

static struct termios oldSettings;
static struct termios newSettings;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
    tcgetattr(0, &oldSettings); /* grab old terminal i/o settings */
    newSettings = oldSettings; /* make new settings same as old settings */
    newSettings.c_lflag &= ~ICANON; /* disable buffered i/o */
    newSettings.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &newSettings); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
    tcsetattr(0, TCSANOW, &oldSettings);
    QProcess process;  // ensure echo on
    process.start("stty echo");
    process.waitForFinished();
}

/* Read 1 character without echo */
char getch(void) 
{
    return getchar();
}

ConsoleReader::ConsoleReader()
{
    initTermios(0);
}

ConsoleReader::~ConsoleReader()
{
    resetTermios();
}

void ConsoleReader::run()
{
  
  /* Read keyboard and translate into car controls 
   *  Car 1/2           [tranlasted char codes in brackets]
   *    	W/^   faster
   *    	S/v   slower
   *    	A/<   left
   *    	D/>   right
   *    	_/Ins fire    [ /I]
   *    	F1    status  [?]
   *    	F5    drive to start  [G]
   *    	F8    clear road  [C]  
   *    	F12   quit    [Q]
   *    	P     pause
   *    	0-5   speed 0%, 20%, ... 100%
   *    	T     turbo speed
   *    	M/m   U-turn
   *    	Home(Pos 1)  Scan Track  [h]
   *  
   *  detecting codes (xterm)
   *   	Ins	^[[2~  
   *  	F1 	^[OP
   *  	F12	^[[24~  
   * 
   * 
   */
    static bool testing = false;
    static bool verbose = true;
    if (!testing) verbose = false;
    
    
    forever {
      char key = getch(); 
      if (verbose) qDebug("%c", key);
      if (key == '\033') { // first value is ESC = \033 = ^[
          // By pressing one arrow key getch() will push three values into the buffer: '\033'  '['  'A', 'B', 'C' or 'D'
          key = getch();
          if (verbose) qDebug("%c", key);
          if (key == '[') {
              key = getch();  // the relevant value
              if (verbose) qDebug("%c", key);
              switch(key) { 
              case 'A':
                  if (testing) qDebug().noquote().nospace() << "<" << "arrow up" << ">"; 
                  key = '^';
                  break;
              case 'B':
                  if (testing) qDebug().noquote().nospace() << "<" << "arrow down" << ">"; 
                  key = 'v';
                  break;
              case 'C':
                  if (testing) qDebug().noquote().nospace() << "<" << "arrow right" << ">"; 
                  key = '>';
                  break;
              case 'D':
                  if (testing) qDebug().noquote().nospace() << "<" << "arrow left" << ">"; 
                  key = '<';
                  break;
              case 'E':
                  if (testing) qDebug().noquote().nospace() << "<" << "center arrow " << ">"; 
                  key = '\0';
                  break;
              case '1':
                  key = getch();    // ^[[1~ = Home
                  if (verbose) qDebug("%c", key);
                  if (key == '~') {
                      if (verbose) qDebug().noquote().nospace() << "<" << "Numpad Home/Pos1" << ">"; 
                      key = 'h';
                  }
                  else if (key == '5') {
                      key = getch();    // ^[[15~ = F5
                      if (verbose) qDebug("%c", key);
                      if (key == '~') {
                          if (testing) qDebug().noquote().nospace() << "<" << "F5" << ">"; 
                          key = 'G';
                      }
                      else {
                        key = '\0';
                      }
                  }
                  else if (key == '9') {
                      key = getch();    // ^[[19~ = F8 
                      if (verbose) qDebug("%c", key);
                      if (key == '~') {
                          if (testing) qDebug().noquote().nospace() << "<" << "F8" << ">"; 
                          key = 'C';
                      }
                      else {
                        key = '\0';
                      }
                  }
                  break;
              case '2':
                  key = getch();    // ^[[2~ = Ins
                  if (verbose) qDebug("%c", key);
                  if (key == '~') {
                      if (testing) qDebug().noquote().nospace() << "<" << "Numpad Ins" << ">"; 
                      key = 'I';
                  }
                  else if (key == '4') {
                      key = getch();    // ^[[24~ = F12
                      if (verbose) qDebug("%c", key);
                      if (key == '~') {
                          if (testing) qDebug().noquote().nospace() << "<" << "F12" << ">"; 
                          key = 'Q';
                      }
                      else {
                        key = '\0';
                      }
                  }
                  else {
                      key = '\0';
                  }
                  break;
              case '4':
                  key = getch();
                  qDebug("%c", key);
                  if (key == '~') if (verbose) qDebug().noquote().nospace() << "<" << "Numpad End" << ">"; 
                  key = '\0';
                  break;
              default:
                  key = '\0';
                  break;
              }
          }
          else if (key == 'O') {
              key = getch();       // ^[OP = F1
              if (verbose) qDebug("%c", key);
              switch(key) { 
              case 'P':
                  if (testing) qDebug().noquote().nospace() << "<" << "F1" << ">"; 
                  key = '?';
                  break;
              case 'H':           // ^[OH = Home
                  if (testing) qDebug().noquote().nospace() << "<" << "Cursor block Home/Pos1" << ">"; 
                  key = 'h';
                  break;
              }
          }
      }
      else {
          switch (key) {  // suppress g, i, q 
              case 'g':
              case 'G':
              case 'i':
              case 'I':
              case 'q':
              case 'Q':
              case 'c':
              case 'C':
              case 'h':
              case 'H':
                key = '\0';
                break;
          }
          if (testing) qDebug().noquote().nospace() << "<" << key << ">"; 
      }
        
      emit KeyPressed(key);
    }
}
