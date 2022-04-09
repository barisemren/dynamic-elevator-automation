
#include <Stepper.h> // Step Motor için gerekli kütüphane

#include <Keypad.h> // Tuş takımı için gerekli kütüphane

#include <HX711_ADC.h> // Ağırlık sensörleri için gerekli kütüphane

//**********************************************************************************LOAD CELL PIN TANIMLAMALARI********************************************************************************
const int HX711_dout_1 = A1; 
const int HX711_sck_1 = A2; 
const int HX711_dout_2 = A5; 
const int HX711_sck_2 = A7; 

//HX711 pin tanımlamaları: (dout pin, sck pin)
HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1); //HX711 1
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2); //HX711 2
unsigned long t = 0;

//**********************************************************************************TUŞ TAKIMI PIN TANIMLAMALARI*******************************************************************************
const byte satir = 1;
const byte sutun = 4;

char tus_takimi_kat_bir[satir][sutun]={
  {'1','2','3','4'}
};
char tus_takimi_kat_iki[satir][sutun]={
  {'A','B','C','D'}
};
char tus_takimi_kat_uc[satir][sutun]={
  {'X','Y','Z','W'}
};

byte satir_pinleri_1[satir]={30};
byte sutun_pinleri_1[sutun]={26,28,22,24};

byte satir_pinleri_2[satir]={40};
byte sutun_pinleri_2[sutun]={36,38,32,34};

byte satir_pinleri_3[satir]={50};
byte sutun_pinleri_3[sutun]={46,48,42,44};

Keypad kiosk1 = Keypad(makeKeymap(tus_takimi_kat_bir), satir_pinleri_1, sutun_pinleri_1, satir, sutun);
Keypad kiosk2 = Keypad(makeKeymap(tus_takimi_kat_iki), satir_pinleri_2, sutun_pinleri_2, satir, sutun);
Keypad kiosk3 = Keypad(makeKeymap(tus_takimi_kat_uc), satir_pinleri_3, sutun_pinleri_3, satir, sutun);

//**********************************************************************************STEP MOTOR PIN TANIMLAMALARI*******************************************************************************

const int stepsPerRevolution = 2048; // Step motorun bir tam turu
int currentfloor = 1; // mevcut kat

Stepper myStepper1 = Stepper(stepsPerRevolution, 39, 43, 41, 45); // Step motor tuş tanımlamaları
Stepper myStepper2 = Stepper(stepsPerRevolution, 47, 51, 49, 53);


//********************************************************************************** PIN TANIMLAMALARI SONU ***********************************************************************************

void setup() {
  Serial.begin(57600); 
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  float calibrationValue_1; // 1. Sensör için kalibrasyon faktörü
  float calibrationValue_2; // 2. Sensör için kalibrasyon faktörü

  calibrationValue_1 = -219.36; // Önceden hesaplanmış değer
  calibrationValue_2 = -219.36; // Önceden hesaplanmış değer

  LoadCell_1.begin();
  LoadCell_2.begin();
  unsigned long stabilizingtime = 2000; 
  boolean _tare = true; 
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) { 
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilizingtime, _tare);
  }
  if (LoadCell_1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.1 wiring and pin designations");
  }
  if (LoadCell_2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.2 wiring and pin designations");
  }
  LoadCell_1.setCalFactor(calibrationValue_1); 
  LoadCell_2.setCalFactor(calibrationValue_2);
  Serial.println("Startup is complete");



  myStepper1.setSpeed(10); // Step motorun dönüş hızı
  myStepper2.setSpeed(10);  
  
  Serial.begin(9600);
  Serial.print("Current Floor : ");
  Serial.println(currentfloor); // Öncelikle başlangıç katı belirtilir.
}

void loop() {

  static boolean newDataReady = 0;
  const int serialPrintInterval = 500; //Daha yavaş bir ekrana yazdırma isteniyorsa değer artırılabilir

  // Yeni bir veri olup olmadığını kontrol edelim:
  if (LoadCell_1.update()) newDataReady = true;
  LoadCell_2.update();

  // Veriyi alıp ekrana yazdıralım:
  if ((newDataReady)) {
    if (millis() > t + serialPrintInterval) {
      float car1_load = LoadCell_1.getData();
      float car2_load = LoadCell_2.getData();
      Serial.print("Load_cell 1 output val: ");
      Serial.print(car1_load);
      Serial.print("    Load_cell 2 output val: ");
      Serial.println(car2_load);
      newDataReady = 0;
      t = millis();
    }
  }

  // Komut satırından t tuşuna basılırsa dara alma işlemi başlatılır:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell_1.tareNoDelay();
      LoadCell_2.tareNoDelay();
    }
  }

  //Dara işleminin bitip bitmediği kontrol edilir:
  if (LoadCell_1.getTareStatus() == true) {
    Serial.println("Tare load cell 1 complete");
  }
  if (LoadCell_2.getTareStatus() == true) {
    Serial.println("Tare load cell 2 complete");
  }

  // Load Cell'lerden gelen değerler okunur:
  float car1_load = LoadCell_1.getData();
  float car2_load = LoadCell_2.getData();
  
  // Kiosklardan gelen verileri okuma:
  char tus1 = kiosk1.getKey();
  char tus2 = kiosk2.getKey();
  char tus3 = kiosk3.getKey();

//**********************************************************************************1. KAT ÇAĞRILARI BAŞLANGICI *******************************************************************************

  // 1. Kiosk 1. kat çağrısı başlangıcı: 
  if(tus1 == '1'){
    Serial.println(" Call From Floor 1");
    if(currentfloor == 1){
      currentfloor = 1;
      if(car1_load > car2_load){
        myStepper2.step(0*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        myStepper1.step(0*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 1;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(1*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 1;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(2*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(2.3*stepsPerRevolution);
        }
      }
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }
  // 1. Kiosk ve 1. kat çağrısı sonu



  // 1. Kiosk ve 2. kat çağrısı başlangıcı:
  if(tus1 == '2'){
    Serial.println(" Call From Floor 1 to Floor 2");
    if(currentfloor == 1){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        Serial.println("Waiting On The 1st Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        Serial.println("Waiting On The 1st Floor");
        myStepper1.step(stepsPerRevolution);
        delay(3000);
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(2*stepsPerRevolution);
        Serial.println("Waiting On The 1st Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(2*stepsPerRevolution);
        Serial.println("Waiting On The 1st Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        }
      }
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }    
  // 1. Kiosk 2. kat çağrısı sonu

   
  // 1. Kiosk 3. kat çağrısı başlangıcı:
  
  if(tus1 == '3'){
    Serial.println(" Call From Floor 1 to Floor 3");
    if(currentfloor == 1){
      currentfloor = 3;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-2*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-2.3*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 3;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        Serial.println("Waiting On The 1st Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper2.step(-2*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        Serial.println("Waiting On The 1st Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper1.step(-2.3*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 3;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(2*stepsPerRevolution);
        Serial.println("Waiting On The 1st Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper2.step(-2*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(2.3*stepsPerRevolution);
        Serial.println("Waiting On The 1st Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper1.step(-2.3*stepsPerRevolution);
        }
      }
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }
  // 1. Kiosk 3. kat çağrısı sonu

//**********************************************************************************1. KAT ÇAĞRILARI SONU *************************************************************************************



//**********************************************************************************2. KAT ÇAĞRILARI BAŞLANGICI *******************************************************************************

  // 2. Kiosk 1. kat çağrısı başlangıcı: 
  if(tus2 == 'A'){
    Serial.println(" Call From Floor 2 to Floor 1");
    if(currentfloor == 1){
      currentfloor = 1;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }
  // 2. Kiosk 1. kat çağrısı sonu

  // 2. Kiosk 2. kat çağrısı başlangıcı:
  if(tus2 == 'B'){
    Serial.println(" Call From Floor 2");
    if(currentfloor == 1){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 2;
      if(car1_load > car2_load){
        myStepper2.step(0*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        myStepper1.step(0*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }
  // 2. Kiosk 2. kat çağrısı sonu

  //2. Kiosk 3. kat çağrısı başlangıcı:
  if(tus2 == 'C'){
    Serial.println(" Call From Floor 2 to Floor 3");
    if(currentfloor == 1){
      currentfloor = 3;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 3;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 3;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        Serial.println("Waiting On The 2nd Floor");
        delay(3000);
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        }
      }
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }
  // 2. Kiosk 3. kat çağrısı sonu

  
//**********************************************************************************2. KAT ÇAĞRILARI SONU *************************************************************************************



//**********************************************************************************3. KAT ÇAĞRILARI BAŞLANGICI *******************************************************************************

  // 3. Kiosk 1. kat çağrısı başlangıcı:
  if(tus3 == 'X'){
    Serial.println(" Call From Floor 3 to Floor 1");
    if(currentfloor == 1){
      currentfloor = 1;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-2*stepsPerRevolution);
        Serial.println("Waiting On The 3rd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper2.step(2*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-2.3*stepsPerRevolution);
        Serial.println("Waiting On The 3rd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper1.step(2.3*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 1;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        Serial.println("Waiting On The 3rd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper2.step(2*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        Serial.println("Waiting On The 3rd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper1.step(2.3*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 1;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(2*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(2.3*stepsPerRevolution);
        }
      }
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }
  // 3. Kiosk 1. kat çağrısı sonu

  // 3. Kiosk 2. kat çağrısı başlangıcı:
  if(tus3 == 'Y'){
    Serial.println(" Call From Floor 3 to Floor 2");
    if(currentfloor == 1){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-2*stepsPerRevolution);
        Serial.println("Waiting On The 3rd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-2.3*stepsPerRevolution);
        Serial.println("Waiting On The 3rd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        Serial.println("Waiting On The 3rd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        Serial.println("Waiting On The 3rd Floor");
        delay(3000);
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 2;
      if(car1_load > car2_load){
        Serial.println("Going Down");
        myStepper2.step(stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Down");
        myStepper1.step(1.5*stepsPerRevolution);
        }
      }
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }
  // 3. Kiosk 2. kat çağrısı sonu

  // 3. Kiosk 3. kat çağrısı başlangıcı:
  if(tus3 == 'Z'){
    Serial.println(" Call From Floor 3");
    if(currentfloor == 1){
      currentfloor = 3;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-2*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-2.3*stepsPerRevolution);
        }
      }
    else if (currentfloor == 2){
      currentfloor = 3;
      if(car1_load > car2_load){
        Serial.println("Going Up");
        myStepper2.step(-1*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        Serial.println("Going Up");
        myStepper1.step(-1.5*stepsPerRevolution);
        }
      }
    else if (currentfloor == 3){
      currentfloor = 3;
      if(car1_load > car2_load){
        myStepper2.step(0*stepsPerRevolution);
        }
      else if(car1_load <= car2_load){
        myStepper1.step(0*stepsPerRevolution);
        }
      }
    // 3. Kiosk 3. kat çağrısı sonu

    
//**********************************************************************************3. KAT ÇAĞRILARI SONU *************************************************************************************

    // Tüm işlemler tamamlandığında mevcut kat bildirilir:
    Serial.print("Current Floor : ");
    Serial.println(currentfloor);
  }

}
