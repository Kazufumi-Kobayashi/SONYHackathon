#include <SDHCI.h>
#include <File.h>

#define Width_Max 320
#define Height_Max 240

class BMPClass {
    public:
        BMPClass();
        ~BMPClass();

        //画像ファイルを開く(return:エラーコード(-1~-4),正常(0))
        int open(String name);
        //画像データの読み込み
        void readImg(uint16_t *bitmap);

        //各パラメータを取得
        int getWidth();
        int getHeight();
        int getPx();
        int getPy();
        String getfilename();
    private:
    String filename;
    SDClass theSD;
    File BMPfile;
    //画像の幅・高さを保存
    int width;
    int height;
    //LCDの中央に表示する位置を保存
    int px;
    int py;
    //ヘッダサイズを保存
    int headerSize;
    //RGBA8888 or RGB888
    int ByteRGB;
    //RGBA8888 => RGB565変換
    uint16_t RGB24to16(int RGB888);
};
