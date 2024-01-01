//Arduino Micro または Leonard を使用してください

//簡単な説明
//コマンドに対応しました。デリミタ:CR、Baud:115200、DataBits:8、StopBit:1

//更新履歴
//V1 初版
//V6 外部調整対応、I/O部修正
//V6.0.0.4 小田急自動切換

//入力ピンアサイン
#define PIN_Disp_0    11 //故障　未使用
#define PIN_Disp_1    9//直通
#define PIN_Disp_2    10//EB
#define PIN_Disp_3    A2//抑速
#define PIN_Disp_4    A3//電制
#define PIN_Disp_5    A4//ATS白色
#define PIN_Disp_6    A5//ATS警報

//↓デバッグのコメント(//)を解除するとシリアルモニタでデバッグできます
//#define DEBUG

bool EB_OER = false; //EB作動
bool EB_JR = false; //EB表示灯
bool EB_JR_latch = false; //EB作動ラッチ

bool ATS_ERR = false; //ATS表示灯
bool ATS_ERR_latch = false; //ATS作動ラッチ
bool ATS_Norm = false; //ATS白色表示灯
bool Densei = false;  //電制表示灯
bool Oer_Kaisei = false;  //回生(小田急)
bool Broken = false; //故障表示灯
bool Chokutsu = false; //直通表示灯
bool Yokusoku = false; //抑速表示灯
bool JyoyoMax = false; //常用最大(小田急)バルブ用
bool Unit1 = false; //ユニット表示灯1
bool DenryuSign = false; //電流符号


uint16_t pressure = 0;

//String strbve = "0000/0/ 00000/000000/0000/0";
String strbve = "";

bool JR = false;

void setup() {

  pinMode(5, INPUT_PULLUP); //連動切替

  pinMode(PIN_Disp_0, OUTPUT); //故障
  pinMode(PIN_Disp_1, OUTPUT); //直通
  pinMode(PIN_Disp_2, OUTPUT); //EB
  pinMode(PIN_Disp_3, OUTPUT); //抑速
  pinMode(PIN_Disp_4, OUTPUT); //電制
  pinMode(PIN_Disp_5, OUTPUT); //ATS白色
  pinMode(PIN_Disp_6, OUTPUT); //ATS警報

  digitalWrite(PIN_Disp_0, 0); //故障
  digitalWrite(PIN_Disp_1, 0); //直通
  digitalWrite(PIN_Disp_2, 0); //EB
  digitalWrite(PIN_Disp_3, 0); //抑速
  digitalWrite(PIN_Disp_4, 0); //電制
  digitalWrite(PIN_Disp_5, 0); //ATS白色
  digitalWrite(PIN_Disp_6, 0); //ATS警報

  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.setTimeout(10);
  Serial1.setTimeout(10);

}

void loop() {

  bool lock = !digitalRead(5); //連動切替
  strbve = "";
  if (Serial.available()) {
    strbve = Serial.readStringUntil('\r');
  }
  if (Serial1.available()) {
    strbve = Serial1.readStringUntil('\r');
  }
  if (strbve.length() > 14 && lock) {

    //ATS白色表示灯抽出
    ATS_Norm = strbve.substring(14, 15).toInt();

    //ATS警報抽出
    ATS_ERR = strbve.substring(15, 16).toInt();

    //直通抽出
    Chokutsu = strbve.substring(17, 18).toInt();

    //抑速抽出
    Yokusoku = strbve.substring(19, 20).toInt();

    //ユニット表示1抽出
    Unit1 = strbve.substring(40, 41).toInt();

    //電流符号抽出
    DenryuSign = (strbve.substring(7, 8) != "-");

    //EB抽出/小田急モード抽出
    int Ats10 = strbve.substring(16, 17).toInt();//EB(JR)
    JR = (Ats10 != 9);

    //JRモード
    if (JR) {

      EB_OER = false;

      //JRモードの時Ats10番はEB
      EB_JR =  (Ats10 == 1);

      //電制(JR)抽出
      Densei = strbve.substring(18, 19).toInt();

      //小田急モード
    } else {
      EB_OER = strbve.substring(31, 32).toInt();
      if (!EB_OER) {
        Oer_Kaisei = strbve.substring(16, 17).toInt();// 5 回生(小田急)
      }
    }

    if (!JR && !EB_OER) {
      JyoyoMax = strbve.substring(26, 27).toInt();// 常用最大ブレーキ作用 (小田急)
    }
  } else if (!lock) {
    Broken = true;//故障表示灯点灯
    Chokutsu = true;//直通表示灯
    EB_JR = true; //直通
    Yokusoku = true; //抑速
    Densei = true; //電制
    ATS_Norm = true;
    ATS_ERR = true;//ATS警報
  }

  //圧力計データ入力がある場合はpressireに圧力計値を入れる
  if ( strbve.length() > 22 ) {
    //圧力値読取り
    pressure = strbve.substring(21, 25).toInt();
    //Serial.println(pressure);
    //圧力上限を超えた場合抑える
    if (pressure > 5000 ) {
      pressure = 5000;
    }
  }

  //表示灯制御部
  digitalWrite(PIN_Disp_0, Broken);   //故障
  digitalWrite(PIN_Disp_1, Chokutsu); //直通
  digitalWrite(PIN_Disp_2, EB_JR || EB_OER); //EB
  digitalWrite(PIN_Disp_3, Yokusoku); //抑速
  digitalWrite(PIN_Disp_4, Densei || Oer_Kaisei || (!Yokusoku && !Densei && Unit1 && !DenryuSign)  || (!ATS_Norm && !ATS_ERR && !DenryuSign)); //電制
  digitalWrite(PIN_Disp_5, ATS_Norm); //ATS白色
  digitalWrite(PIN_Disp_6, ATS_ERR); //ATS警報
  //表示灯制御部ここまで

  delay(10);
}