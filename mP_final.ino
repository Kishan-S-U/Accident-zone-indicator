// 1-hospital, 2-school, 3-accident prone, 4-blind turn
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <TinyGPS.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
TinyGPS gps;

#define Cs 10
#define led 8

File datafile;
const int h_add = 5;  //add to GMt to convert to Indian standard time
const int m_add = 30;
int index=0, sindex=0;
float tvalue, fvalue=1.0, speed;
int cH=0, cS=0, cA=0, cB=0, i=0, speed_limit;
double lt2, ln2;
double limit = 0.001500;
String hold, sdata, tdata;
String thold[4] = {"", "", "", ""};

void setup() {
  // put your setup code here, to run once:
  pinMode(Cs, OUTPUT);
  pinMode(led, OUTPUT);
  lcd.begin(16, 2);
  lcd.clear();
  Serial.begin(9600);
  if(sdCheck()){
    Serial.println("SD card initialised");
    datafile = SD.open("data1.txt", FILE_READ);  // reading the file with location data
    // checking if the files is present
    if(SD.exists("data1.txt")){
      lcd.setCursor(0, 0);
      lcd.print("Data file found...");
    }
    }else{
      lcd.setCursor(0, 0);
      lcd.print("SD Card Error");
    }
    datafile.close();
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available()) {
//    double t1 = micros();
    //String data = Serial.readString();   // get input from serial monitor
    char data = Serial.read();
    if(gps.encode(data)) {
      float lt1, ln1;
      long fix;
      int year;
      byte month, day, hour, minute, second, hundredths;
      unsigned long fix_age;
      gps.f_get_position(&lt1, &ln1, &fix);
      if (!(TinyGPS::GPS_INVALID_DATE && TinyGPS::GPS_INVALID_TIME)){
        gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);
      }
      hour += h_add;
      minute += m_add;
      while(minute >= 60){
        hour += 1;
        minute -= 60;
      }
      speed = gps.f_speed_kmph();
      String a = dataCheck(lt1, ln1);   // check the speed limit in this location
      Serial.println(a);  // print the zone.. for reference!!
      printData(a);  // print the respective result
      if(speed > speed_limit) {
        digitalWrite(led, HIGH); 
      }else {
        digitalWrite(led, LOW);
      }
      Alert();
      index = 0;
      sindex = 0;
//      double t2 = micros();
//      Serial.println(t2-t1);
    }
//    if(filestatus){
//      datafile = SD.open("data1.txt", FILE_READ);
//    }
  }
  hold = "";
}

bool sdCheck() {
  // this function will check if sd card is ready to work or not
  if(!SD.begin()){
    return false;
  }else{
    return true;
  }
}

bool diffCheck(double lat1, double lon1, double lat2, double lon2, double limit){
//  Serial.println("diff arrived");

  double difflat = abs(lat2 - lat1);
  double difflon = abs(lon2 - lon1);
  tvalue = (difflon + difflat)/2;
  if(difflat <= limit && difflon <= limit){
    return true;
  }else{
    return false;
  }
}

String dataCheck(float lt, float ln){
//  Serial.println("arrived");
  datafile = SD.open("data1.txt", FILE_READ);
  while(datafile.available()){
    sdata = datafile.readStringUntil('\r\n');
    if(index >= 1) {
      lt2 = sdata.substring(0, 9).toDouble();
      ln2 = sdata.substring(11, 20).toDouble();
      bool cc = diffCheck(lt, ln, lt2, ln2, limit);
//      bool cc = constrain(lt, lt2+limit+1, lt2+1) && constrain(ln, ln2+limit+1, ln2+1);
//      tvalue = (abs(lt2-lt) + abs(ln2 - ln))/2;
      if(cc){
        if(fvalue > tvalue && i < 4){   // this is logic for a case where multiple zones are within the range
          // everytime the distance is less than the previous value, final value will be updated
          fvalue = tvalue;
          thold[i] = sdata.substring(22, 24);  // this holds the information of, which all zones are within the range
          i = i + 1;
        }
        if(sdata.indexOf("HL")>=0){
          cH++;
          if(cS == 0 && cA == 0 && cB == 0 && cH == 1){
            hold = "HL";
          }
        }else if(sdata.indexOf("SL")>=0){
          cS++;
          if(cH == 0 && cA == 0 && cB == 0 && cS == 1){
            hold = "SL";
          }
        }else if(sdata.indexOf("AP")>=0){
          cA++;
          if(cH == 0 && cA == 1 && cB == 0 && cS == 0){
            hold = "AP";
          }
        }else if(sdata.indexOf("BT")>=0){
          cB++;
          if(cH == 0 && cA == 0 && cB == 1 && cS == 0){
            hold = "BT";
          }
        }
      }if(i > 1){
        // this will update the 'hold' value with the nearest zone, incase of multiple zones in the same range 
        hold = thold[i-1];
      }
    }
    index++;
    lt2 = 0;
    ln2 = 0;
  }
  if(cH == 0 && cS == 0 && cA == 0 && cB == 0){
    hold = "NL";
  }
  datafile.close();
  clearall(); // clearing intermediate values and resetting to default to perform new calculations
  return hold;
}

void clearall() {
  // this function just clears all the intermediate stored values of required variables
  cH = 0; cS = 0;
  cA = 0; cB = 0;
  i = 0; fvalue = 1.0;
  thold[0] = ""; thold[1] = "";
  thold[2] = ""; thold[3] = "";
}

void printData(String d) {
  lcd.clear();
  delay(100);
  if(d.indexOf("HL")>=0){
    lcd.setCursor(0, 0); lcd.print("HOSPITAL ZONE   DON'T HORN      ");
    lcd.setCursor(0, 1); lcd.print("MAX SPEED 25kmphMAX SPEED 25kmph");
    speed_limit = 25;
  }else if(d.indexOf("SL")>=0){
    lcd.setCursor(0, 0); lcd.print("SCHOOL ZONE     CHILDREN AROUND ");
    lcd.setCursor(0, 1); lcd.print("MAX SPEED 25kmphMAX SPEED 25kmph");
    speed_limit = 25;
  }else if(d.indexOf("AP")>=0){
    lcd.setCursor(0, 0); lcd.print("ACCIDENT ZONE   SLOW DOWN!!!!   ");
    lcd.setCursor(0, 1); lcd.print("MAX SPEED 35kmphMAX SPEED 35kmph");
    speed_limit = 35;
  }else if(d.indexOf("BT")>=0){
    lcd.setCursor(0, 0); lcd.print("BLIND TURN ZONE JUNCTIONS AROUND");
    lcd.setCursor(0, 1); lcd.print("MAX SPEED 30kmphMAX SPEED 30kmph");
    speed_limit = 30;
  }else if(d.indexOf("NL")>=0){
    lcd.setCursor(0, 0); lcd.print("CITY LIMITS     CITY LIMITS     ");
    lcd.setCursor(0, 1); lcd.print("MAX SPEED 50kmphMAX SPEED 50kmph");
    speed_limit = 50;
  }
}

void Alert() {
  delay(1000);
  for(int j=0;j<16;j++){
    lcd.scrollDisplayLeft();
  }
}
