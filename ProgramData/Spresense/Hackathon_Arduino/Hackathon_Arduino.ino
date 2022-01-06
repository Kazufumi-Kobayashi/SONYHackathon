#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SDHCI.h>
#include <Audio.h>
#include <USBSerial.h>

#include "BMPClass.h"

#define TFT_CS -1
#define TFT_RST 8
#define TFT_DC 9

#define SW1 5
#define SW2 6
#define SW3 7

USBSerial USBSerial;

/*-----------BMPSetup------------*/
BMPClass img_bmp;
uint16_t bitmap[(Width_Max * Height_Max)];
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);

void BMPView(char* file_name) {
  int errorCode = img_bmp.open(file_name);
  if(errorCode < 0) {
    Serial.print("Error Code : ");
    Serial.println(errorCode);
    while(1){}
  }

  tft.begin(40000000);
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);

  img_bmp.readImg(bitmap);

  int width = img_bmp.getWidth();
  int height = img_bmp.getHeight();
  int px = img_bmp.getPx();
  int py = img_bmp.getPy();
  
  tft.drawRGBBitmap(px, py, bitmap, width, height);
}

/*-----------AudioSetup------------*/
SDClass theSD;
AudioClass *theAudio;
File myFile;
err_t err;
bool ErrEnd = false;

static void audio_attention_cb(const ErrorAttentionParam *atprm) {
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING) {
      ErrEnd = true;
    }
}

void NormalAudioSetup() {
  /* Initialize SD */
  while (!theSD.begin()) {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }
  
  // start audio system
  theAudio = AudioClass::getInstance();
  theAudio->begin(audio_attention_cb);
  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
  err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_WAV, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_BITLENGTH_16, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK) {
      printf("Player0 initialize error\n");
      exit(1);
      }
}

void HiResAudioSetupHi() {
  /* Initialize SD */
  while (!theSD.begin()) {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }
  
  // start audio system
  theAudio = AudioClass::getInstance();
  theAudio->begin(audio_attention_cb);
  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_HIRES);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
  err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_WAV, "/mnt/sd0/BIN", AS_SAMPLINGRATE_192000, AS_BITLENGTH_24, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK) {
      printf("Player0 initialize error\n");
      exit(1);
      }
}

void HiResAudioSetupMid() {
  /* Initialize SD */
  while (!theSD.begin()) {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }
  
  // start audio system
  theAudio = AudioClass::getInstance();
  theAudio->begin(audio_attention_cb);
  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_HIRES);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
  err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_WAV, "/mnt/sd0/BIN", AS_SAMPLINGRATE_96000, AS_BITLENGTH_24, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK) {
      printf("Player0 initialize error\n");
      exit(1);
      }
}

void openMyFile(char* file_name) {
  /* Open file placed on SD card */
  myFile = theSD.open(file_name);

  /* Verify file open */
  if (!myFile) {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! %s\n", myFile.name());

  /* Set file position to beginning of data */
  err = theAudio->writeFrames(AudioClass::Player0, myFile);
  if (err != AUDIOLIB_ECODE_OK) {
    printf("File Read Error! =%d\n",err);
    myFile.close();
    exit(1);
    }
  
  /* Main volume set to -16.0 dB */
  theAudio->setVolume(100);
  theAudio->startPlayer(AudioClass::Player0);
}

void playAudio(char *file_name) {
  if (!myFile) {
    openMyFile(file_name);
  }
  while (1) {
    int err = theAudio->writeFrames(AudioClass::Player0, myFile);
    if (err == AUDIOLIB_ECODE_FILEEND) {
      Serial.println("Main player File End!");
      break;
    }
    if (ErrEnd) {
      Serial.println("Error End");
      stopAudio();
    }

    usleep(10000);
  }
}

void stopAudio() {
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  Serial.println("AudioStop");
}

void setup() {
  //LED Setup
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  //Swtich Setup
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);

  //Serial Setup
  Serial.begin(115200);
  USBSerial.begin(9600);

  //電源投入後25秒後起動中アナウンス
  sleep(5);

  BMPView("/image/startup.bmp");

  sleep(5);

  digitalWrite(LED0, HIGH);
  NormalAudioSetup();
  playAudio("/Sound/setup.wav");
  stopAudio();
  digitalWrite(LED0, LOW);

  sleep(20);

  //約10秒後に起動完了アナウンス
  digitalWrite(LED0, HIGH);
  NormalAudioSetup();
  playAudio("/Sound/aftersetup.wav");
  stopAudio();
  BMPView("/image/startup_complete.bmp");
  digitalWrite(LED0, LOW);

  //スタートボタンを押してキャリブレーションアナウンス
  while (1) {
    int value;
    value = digitalRead(SW2);
    if(value == LOW){
      break;
    }
  }
  digitalWrite(LED0, HIGH);
  NormalAudioSetup();
  playAudio("/Sound/calibration.wav");
  stopAudio();
  BMPView("/image/calib.bmp");
  digitalWrite(LED0, LOW);

  //キャリブレーション指示送信
  USBSerial.print("A");

  //RasPi側の動作完了確認待ち
  while(true){
    digitalWrite(LED1, HIGH);
    char Return_Rasp = USBSerial.read();
    if (Return_Rasp == 'B') {
      break;
    }
  }

  //キャリブレーション完了アナウンス
  digitalWrite(LED0, HIGH);
  NormalAudioSetup();
  playAudio("/Sound/beforestart.wav");
  stopAudio();
  BMPView("/image/calib_complete.bmp");
  digitalWrite(LED0, LOW);

  //スタートボタンを押して動作開始アナウンス
  while (1) {
    int value;
    value = digitalRead(SW2);
    if(value == LOW){
      break;
    }
  }
  digitalWrite(LED0, HIGH);
  NormalAudioSetup();
  playAudio("/Sound/afterstarting.wav");
  stopAudio();
  NormalAudioSetup();
  playAudio("/Sound/endcondition.wav");
  stopAudio();
  BMPView("/image/goodpose.bmp");
  digitalWrite(LED0, LOW);

  //動作開始指示+RasPi側の確認待ち
  USBSerial.print("C");
  while(true){
    char Return_Rasp = USBSerial.read();
    if (Return_Rasp == 'D') {
      break;
    }
  }

  //姿勢維持(姿勢判断待ち)
  while(true){
    int value;
    value = digitalRead(SW3);
    char Return_Rasp = USBSerial.read();

    //停止ボタンが押された場合停止
    if(value == LOW){
      break;
    }
    //'F'は姿勢が良い場合(画面表示)
    if (Return_Rasp == 'F') {
      BMPView("/image/goodpose.bmp");
    }
    //'E'は姿勢が悪い場合(アナウンス+画面表示)
    if (Return_Rasp == 'E') {
      digitalWrite(LED0, HIGH);
      NormalAudioSetup();
      playAudio("/Sound/notgoodpose.wav");
      stopAudio();
      BMPView("/image/notgoodpose.bmp");
      digitalWrite(LED0, LOW);
    }
  }

  //動作終了アナウンス
  digitalWrite(LED0, HIGH);
  NormalAudioSetup();
  playAudio("/Sound/taskfinish.wav");
  stopAudio();
  BMPView("/image/finish.bmp");
  digitalWrite(LED0, LOW);

  //manager側姿勢推定ループ停止指示送信+確認待ち
  USBSerial.print("G");
  while(true){
    char Return_Rasp = USBSerial.read();
    if (Return_Rasp == 'H') {
      break;
    }
  }

  //表情情報を元にした音楽再生
  USBSerial.print("I");
  while(true){
    char Return_Rasp = USBSerial.read();
    //happy or neutral
    if (Return_Rasp == '2' || '3') {
      digitalWrite(LED0, HIGH);
      HiResAudioSetupMid();
      playAudio("/Sound/happy_neutral.wav");
      stopAudio();
      digitalWrite(LED0, LOW);
      break;
    }
    //angry or surprise
    if (Return_Rasp == '0' || '5') {
      digitalWrite(LED0, HIGH);
      HiResAudioSetupHi();
      playAudio("/Sound/angry_surprise.wav");
      stopAudio();
      digitalWrite(LED0, LOW);
      break;
    }
    //sad or fear
    if (Return_Rasp == '1' || '4') {
      digitalWrite(LED0, HIGH);
      HiResAudioSetupMid();
      playAudio("/Sound/sad_fear.wav");
      stopAudio();
      digitalWrite(LED0, LOW);
      break;
    }
  }
}

void loop() {
  sleep(1);
}
