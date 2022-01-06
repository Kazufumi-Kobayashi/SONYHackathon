#include "BMPClass.h"

BMPClass::BMPClass() {
    this->theSD.begin();
}

BMPClass::~BMPClass() {
    
}

int BMPClass::open(String name) {
    this->filename = name;
    this->BMPfile = this->theSD.open(this->filename);
    if(!this->BMPfile){
        //File Open Error
        return -1;
    }
    if(!(this->BMPfile.read() == 0x42 && this->BMPfile.read() == 0x4d)){
        //File format(BMP) is wrong
        return -2;
    }
    int tmp_file;
    //ヘッダサイズを取得
    this->BMPfile.seek(10);
    this->BMPfile.read(&tmp_file, 4);
    this->headerSize = tmp_file;
    //画像の幅・高さを取得
    this->BMPfile.seek(18);
    this->BMPfile.read(&tmp_file, 4);
    this->width = tmp_file;
    this->BMPfile.read(&tmp_file, 4);
    this->height = tmp_file;
    //画像サイズが表示できる最大値を超えてないか判別
    if(this->width > Width_Max || this->height > Height_Max){
        return -3;
    }
    //ビット深度の判定
    this->BMPfile.seek(28);
    /*if(this->BMPfile.read() == 0x20){
      //ビット深度の判別(32bitのみ対応)
      return -4;
    }*/
    switch(this->BMPfile.read()){
    case 0x20:
        //RGBA8888(32bit)
        this->ByteRGB = 4;
        break;
    case 0x18:
        //RGB888(24bit)
        this->ByteRGB = 3;
        break;
    default:
        this->ByteRGB = 0;
    }
    if(this->ByteRGB == 0){
        return -4;
    }
    //表示する位置を計算
    this->px = (Width_Max - this->width) / 2;
    this->py = (Height_Max - this->height) / 2;
    return 0;
}

void BMPClass::readImg(uint16_t *bitmap) {
    this->BMPfile.seek(this->headerSize);
    unsigned int tmp_img = 0x00;
    int x = 0;
    int y = this->height;
    while(this->BMPfile.available()){
    this->BMPfile.read(&tmp_img, this->ByteRGB);
    bitmap[x + (this->width * (y - 1))] = this->RGB24to16(tmp_img);
    x++;
    if(x >= width){
      x = 0;
      y--;
      if(y <= 0){
        break;
      }
    }
  }
}

uint16_t BMPClass::RGB24to16(int RGB888) {
    return (((RGB888 >> 8) & 0xf800) | ((RGB888 >> 5) & 0x7e0) | ((RGB888 >> 3) & 0x1f));
}

int BMPClass::getWidth() {
    return this->width;
}

int BMPClass::getHeight() {
    return this->height;
}

int BMPClass::getPx() {
    return this->px;
}

int BMPClass::getPy() {
    return this->py;
}

String BMPClass::getfilename() {
    return this->filename;
}
