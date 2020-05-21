#ifndef OLED_CLOCK_PRESERNTER_H
#define OLED_CLOCK_PRESERNTER_H

#include <AceTime.h>
//#include "CGlobals.h"

#include "config.h"
#include "StoredInfo.h"
#include "ClockInfo.h"
#include "RenderingInfo.h"

using namespace ace_time;
using ace_time::common::printPad2;
using ace_time::common::CstrPrint;

class Presenter {
      enum displayID {thisdate, thistime, thisday, tztype, tzname, tzdst, tzver, acever, thissec, thisAM, thisring};
      enum {yearfirst, monthfirst, dayfirst};
  public:
/** Constructor.
    Presenter(SSD1306Ascii& oled):
        mOled(oled) {}

  Presenter(gslc_tsGui& gui):
      m_gui(gui) {}
*/
static const uint8_t displayIDdate = thisdate;
static const uint8_t displayIDtime = thistime;
static const uint8_t displayIDday = thisday;
static const uint8_t displayIDsec = thissec;
static const uint8_t displayIDam = thisAM;
static const uint8_t displayIDtype = tztype;
static const uint8_t displayIDname = tzname;
static const uint8_t displayIDtz = tzver;
static const uint8_t displayIDace = acever;
static const uint8_t displayIDdst = tzdst;
static const uint8_t displayIDring = thisring;

    void display() {
      if (needsUpdate()) {
        displayData();
      }

      mPrevRenderingInfo = mRenderingInfo;
    }

    void setRenderingInfo(uint8_t mode, bool suppressBlink, bool blinkShowState,
        const ClockInfo& clockInfo) {
      mRenderingInfo.mode = mode;
      mRenderingInfo.suppressBlink = suppressBlink;
      mRenderingInfo.blinkShowState = blinkShowState;
      mRenderingInfo.hourMode = clockInfo.hourMode;
      mRenderingInfo.timeZone = clockInfo.timeZone;
      mRenderingInfo.dateTime = clockInfo.dateTime;
      mRenderingInfo.hourMode = timeMode;     //GUI interface
    }

    void setTimeDateDisplay(int _timeMode, int _dateMode){
      timeMode= _timeMode;
      dateMode=_dateMode;
    }


      using SomeEvent = void (*)(int,char*); //type aliasing
    //C++ version of: typedef void (*InputEvent)(const char*)

    void RegisterCallback(SomeEvent InEvent)
    {
      DispEvent = InEvent;
    }

  private:
    // Disable copy-constructor and assignment operator
   // Presenter(const Presenter&) = delete;
    //Presenter& operator=(const Presenter&) = delete;

    /**
     * True if the display should actually show the data. If the clock is in
     * "blinking" mode, then this will return false in accordance with the
     * mBlinkShowState.
     */
    bool shouldShowFor(uint8_t mode) const {
      return mode != mRenderingInfo.mode
          || mRenderingInfo.suppressBlink
          || mRenderingInfo.blinkShowState;
    }

    /** The display needs to be cleared before rendering. */
    bool needsClear() const {
      return mRenderingInfo.mode != mPrevRenderingInfo.mode;
    }

    /** The display needs to be updated because something changed. */
    bool needsUpdate() const {
      return mRenderingInfo.mode != mPrevRenderingInfo.mode
          || mRenderingInfo.suppressBlink != mPrevRenderingInfo.suppressBlink
          || (!mRenderingInfo.suppressBlink
              && (mRenderingInfo.blinkShowState
                  != mPrevRenderingInfo.blinkShowState))
          || mRenderingInfo.hourMode != mPrevRenderingInfo.hourMode
          || mRenderingInfo.timeZone != mPrevRenderingInfo.timeZone
          || mRenderingInfo.dateTime != mPrevRenderingInfo.dateTime;
    }

    void displayData() {

      //mOled.home();

      switch (mRenderingInfo.mode) {
        case MODE_DATE_TIME:
        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
          displayDateTime();
          displayGage();
          break;

        case MODE_TIME_ZONE:
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
      #else
        case MODE_CHANGE_TIME_ZONE_NAME:
      #endif
          displayTimeZone();
          break;

        case MODE_ABOUT:
          displayAbout();
          break;
      }
    }
    // for now just demostrate session advancing over and over.
    //Just going to generate a minute heartbeat to Gui
    //eventually there is probably a better place for this
     void displayGage(){
     const ZonedDateTime& dateTime = mRenderingInfo.dateTime;
     switch(sessState){
      case 0:
        target = dateTime.second();
        if(target<60) sessState=1;
        dateMode++;
        if (dateMode>=3) dateMode = 0;
        break;
     case 1:
      current = dateTime.second();
      if(((current-target<0)&&(current+60-target>=5))||(current-target>=5)){
       sessState=0;
       datestr = String()+"Minute";
       displayGUI(datestr, thisring);
        }
      break;
      default:
      break;
      }
    }
    void displayDateTime()  {
    #if ENABLE_SERIAL == 1
      SERIAL_PORT_MONITOR.println(F("displayDateTime()"));
    #endif
      //mOled.setFont(fixed_bold10x15);
      const ZonedDateTime& dateTime = mRenderingInfo.dateTime;
      if (dateTime.isError()) {
       // mOled.println(F("<Error>"));
      datestr=String("Unknown");
	    displayGUI(datestr, thisdate);
      //DispEvent(thisdate);
        return;
      }

      // date
       String yearString;
       String monthString;
       String dayString;
      if (shouldShowFor(MODE_CHANGE_YEAR)) {
        //mOled.print(dateTime.year());
        yearString=String()+(dateTime.year());
      } else {
        //mOled.print("    ");
        yearString = String()+("    ");
      }
      //mOled.print('-');

      if (shouldShowFor(MODE_CHANGE_MONTH)) {
        //printPad2(mOled, dateTime.month());
        monthString = String()+(dateTime.month());
      } else {
        //mOled.print("  ");
        monthString ="   ";
      }
      //mOled.print('-');
      //datestr = datestr+"-";
      if (shouldShowFor(MODE_CHANGE_DAY)) {
        //printPad2(mOled, dateTime.day());
        dayString = String()+(dateTime.day());
      } else{
        //mOled.print("  ");
        dayString = "   ";
      }
      //mOled.clearToEOL();
      //mOled.println();
      switch(dateMode){
        case yearfirst:
        datestr = yearString+"-"+monthString+"-"+dayString;
        break;
        case monthfirst:
        datestr = monthString+"/"+dayString+"/"+yearString;
        break;
        case dayfirst:
        datestr = dayString+"/"+monthString+"/"+yearString;
        break;
      }
      displayGUI(datestr, thisdate);
      //DispEvent(thisdate);
      
      // time
      datestr="";
      if (shouldShowFor(MODE_CHANGE_HOUR)) {
        uint8_t hour = dateTime.hour();
        if (mRenderingInfo.hourMode == StoredInfo::kTwelve) {
          if (hour == 0) {
            hour = 12;
          } else if (hour > 12) {
            hour -= 12;
          }
          //printPad2(mOled, hour, ' ');
          if (hour<10){
            datestr=String()+" "+(hour);
          } else {
          datestr=String()+(hour);
          }
        } else {
          //printPad2(mOled, hour);
          if (hour<10){
            datestr=String()+" "+(hour);
          } else {
            datestr=String()+(hour);
          }
        }
      } else {
        //mOled.print("  ");
        datestr=String()+"  ";
      }
      //mOled.print(':');
      datestr=datestr+":";
      if (shouldShowFor(MODE_CHANGE_MINUTE)) {
        //printPad2(mOled, dateTime.minute());
        if (dateTime.minute()<10){
          datestr=datestr+"0"+dateTime.minute();
        }else {
        datestr=datestr+dateTime.minute();
        }
      } else {
        //mOled.print("  ");
        datestr=datestr+"  ";
      }
      //mOled.print(':');
      datestr=datestr+":";
      displayGUI(datestr, thistime);
      
      if (shouldShowFor(MODE_CHANGE_SECOND)) {
        //printPad2(mOled, dateTime.second());
        if (dateTime.second()<10){
          datestr=String()+"0"+(dateTime.second());
        } else {
        datestr=String()+(dateTime.second());
        }
      } else {
        //mOled.print("  ");
        datestr=String()+"  ";
      }
      displayGUI(datestr, thissec);
      //mOled.print(' ');

      if (mRenderingInfo.hourMode == StoredInfo::kTwelve) {
        //mOled.print((dateTime.hour() < 12) ? "AM" : "PM");
        datestr=String()+((dateTime.hour() < 12) ? "AM" : "PM");
      } else {
        datestr="  ";
      }
      displayGUI(datestr, thisAM);
      //mOled.clearToEOL();
      //mOled.println();
      
      // week day
      //mOled.print(DateStrings().dayOfWeekLongString(dateTime.dayOfWeek()));
      //mOled.clearToEOL();
      datestr = String(DateStrings().dayOfWeekLongString(dateTime.dayOfWeek()));
      displayGUI(datestr, thisday);
      //DispEvent(thisday);
    }

    void displayTimeZone()  {
    #if ENABLE_SERIAL
      SERIAL_PORT_MONITOR.println(F("displayTimeZone()"));
    #endif
      //mOled.setFont(fixed_bold10x15);

      // Don't use F() strings for short strings <= 4 characters. Seems to
      // increase flash memory, while saving only a few bytes of RAM.

      // Display the timezone using the TimeZoneData, not the dateTime, since
      // dateTime will contain a TimeZone, which points to the (singular)
      // Controller::mZoneProcessor, which will contain the old timeZone.
      auto& tz = mRenderingInfo.timeZone;
      //mOled.print("TZ: ");
      datestr=String()+"TZ:";
      const __FlashStringHelper* typeString;
      switch (tz.getType()) {
        case TimeZone::kTypeManual:
          typeString = F("manual");
          break;
        case TimeZone::kTypeBasic:
        case TimeZone::kTypeBasicManaged:
          typeString = F("basic");
          break;
        case TimeZone::kTypeExtended:
        case TimeZone::kTypeExtendedManaged:
          typeString = F("extd");
          break;
        default:
          typeString = F("unknown");
      }
        //mOled.print(typeString);
        //mOled.clearToEOL();
        datestr = datestr+typeString;
        displayGUI(datestr, tztype);
      
      datestr = "";
      switch (tz.getType()) {
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case TimeZone::kTypeManual:
          //mOled.println();
          //mOled.print("UTC");
          datestr=String()+"UTC";
          if (shouldShowFor(MODE_CHANGE_TIME_ZONE_OFFSET)) {
            TimeOffset offset = tz.getStdOffset();
            //offset.printTo(mOled);
            datestr = datestr+offset;
            Serial.print("TZ offset");
            offset.printTo(Serial);
            Serial.println(datastr);
          }
          //mOled.clearToEOL();
			    displayGUI(datestr, tzname);
          //mOled.println();
          //mOled.print("DST: ");
			    datestr="DST:";
          if (shouldShowFor(MODE_CHANGE_TIME_ZONE_DST)) {
            //mOled.print((tz.getDstOffset().isZero()) ? "off " : "on");
          datestr = datestr+((tz.getDstOffset().isZero()) ? "off " : "on");
          Serial.print("DST offset");
          displayGUI(datestr, tzdst);
		  }
          //mOled.clearToEOL();
          break;

      #else
        case TimeZone::kTypeBasic:
        case TimeZone::kTypeExtended:
        case TimeZone::kTypeBasicManaged:
        case TimeZone::kTypeExtendedManaged:
          // Print name of timezone
          //mOled.println();
          if (shouldShowFor(MODE_CHANGE_TIME_ZONE_NAME)) {
            //tz.printShortTo(mOled);
            tz.printShortTo(Serial);
            Serial.println("--------------------------");
            tz.printShortTo(cstrPrint);
            datestr = String()+cstrPrint.getCstr();
            cstrPrint.flush(); // needed only if this will be used again
            //Serial.print("cstrPrint datestr: ");
            //Serial.println(datestr);
           displayGUI(datestr, tzname);
            }
          //mOled.clearToEOL();

          // Clear the DST: {on|off} line from a previous screen
          datestr=" ";
          displayGUI(datestr, tzdst); 
          //DispEvent(tzdst);
          //mOled.println();
          //mOled.clearToEOL();
          break;
      #endif

        default:
          //mOled.println();
          //mOled.print(F("<unknown>"));
          datestr=String("<unknown>");
          displayGUI(datestr, tztype);
          displayGUI(datestr, tzname);
          datestr=" ";
          displayGUI(datestr, tzdst);
          //mOled.clearToEOL();
          //mOled.println();
          //mOled.clearToEOL();
          break;
        }
    }
	
    void displayAbout()  {
    #if ENABLE_SERIAL == 1
      SERIAL_PORT_MONITOR.println(F("displayAbout()"));
    #endif

      // Use F() macros for these longer strings. Seems to save both
      // flash memory and RAM.
      /*
      mOled.print(F("TZ: "));
      mOled.println(zonedb::kTzDatabaseVersion);
      mOled.print(F("AT: "));
      mOled.print(ACE_TIME_VERSION_STRING);
      */
      datestr = String("TZ: ")+(zonedb::kTzDatabaseVersion);
      displayGUI(datestr, tzver);
      datestr = String("AT: ")+(ACE_TIME_VERSION_STRING);
      displayGUI(datestr, acever);
     }
     
     void displayGUI (String& dstring, displayID targ){
      int dlen;
      /*
      Serial.print("displayGUI: ");
      Serial.print(targ);
      Serial.print(" text: ");
      Serial.println(dstring);*/
      dlen = dstring.length();
      dstring.toCharArray(buffer, dlen + 1);
      DispEvent(targ, buffer);
	  }
   
    int sessState = 0;
    int dateMode =0; 
    int timeMode = 0;  //24HR
    uint16_t target;
    uint16_t current;
    CstrPrint<32> cstrPrint; // 32-byte buffer
    char buffer[30];
    String datestr="";
    SomeEvent DispEvent;
    RenderingInfo mRenderingInfo;
    RenderingInfo mPrevRenderingInfo;
};

#endif
