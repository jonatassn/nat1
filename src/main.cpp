// Programa : Teste escrita cartao SD
// Autor : Arduino e Cia
// Baseado no arquivo exemplo da biblioteca SD

// Carrega a biblioteca SD
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>
#include <Wire.h>        //Biblioteca para manipulação do protocolo I2C
#include <DS3231.h>      //Biblioteca para manipulação do DS3231


String lerRFIDString();
void lerArquivo(String nomeArquivo, File dataFile);
void gravaArquivo(String nomeArquivo, File dataFile, String datahora);
String getData();
/* Esquema Uno: 
 *  SDCard: MISO porta 12; SCK porta 13; MOSI porta 11; CS porta 4; 5VCC (ligar no 3vcc tamém) 
 *  RTC3132: SLC porta A5; SDA porta A4; 3.3vcc ; 32K não usa; SQW não usa
    Objetivo>: usar ID RFID para ler/gravar no cartao!
 */
 //RFID
constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
//constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

LiquidCrystal_I2C lcd(0x3F,16,2);  // Criando um LCD de 16x2 no endereço 0x20

// Init array that will store new NUID 
byte nuidPICC[4];
String IDHex;

 //SDCARD
Sd2Card SDcard;
SdVolume volume;

//RTC
DS3231 rtc;              //Criação do objeto do tipo DS3231
RTCDateTime dataehora;   //Criação do objeto do tipo RTCDateTime

int flag;
// Pino do Arduino conectado ao pino CS do modulo
const int chipSelect = 4;  
// Pino do Arduino conectado ao push-button
int pinobotao = 3;
static int contador;
char c;
File dataFile;
String datinha, idFormatado = "";
void gravaArquivo(String nomeArquivo, File dataFile);
void lerArquivo(String nomeArquivo, File dataFile);

void setup()
{
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  //inicializa RFID
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  
  // Define o pino do push-button como entrada
  
  Serial.println("Iniciando..");
  contador = 1;
  pinMode(pinobotao, INPUT);

  //Inicializa RTC
  rtc.begin(); //Inicialização do RTC DS3231
  //rtc.setDateTime(__DATE__, __TIME__);   //Configurando valores iniciais //do RTC DS3231
  //Inicia a comunicacao com o modulo SD
  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Falha ao acessar o cartao !");
    Serial.println("Verifique o cartao/conexoes e reinicie o Arduino...");
    return;
  }
  Serial.println("Cartao iniciado corretamente !");
  Serial.println("Aguardando...");
  
  Serial.println();
}

void loop() {

        lcd.home();
        lcd.print("Aguardando...");
    // Look for new cards // cartao novo
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed // fez leitura
  if ( ! rfid.PICC_ReadCardSerial())
    return;
  Serial.print("Lendo... ");
  String data;
  data = lerRFIDString() + '*' + getData();
  datinha = getData();
  Serial.print(" Leitura: ");
  lcd.clear();
  lcd.home();
  lcd.print("Ave: " + idFormatado );
  lcd.setCursor(0, 1);
  lcd.print(datinha);
  Serial.println(data);
  gravaArquivo("dataHora.txt", dataFile, data);
    // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  delay(5000);
  lcd.clear();

}

String getData() {
  String data, d,h,e, aux, aux2;
    dataehora = rtc.getDateTime();
  d = "/";
  e = ' ';
  h= ":";
  data = dataehora.day + d + dataehora.month + d;
  aux = dataehora.year;
  aux2 = "";
  for(int a = aux.length() - 2; a<aux.length(); a++) {
        aux2 = aux2 + aux[a];
  }
  data = data + aux2 + e + dataehora.hour + h + dataehora.minute + h + ((dataehora.second < 10) ? ('0' + dataehora.second) : dataehora.second);
 
  return data;
}

void gravaArquivo(String nomeArquivo, File dataFile, String datahora) {
    // Abre o arquivo arquivo.txt do cartao SD
    dataFile = SD.open("dataHora.txt", FILE_WRITE);
    // Grava os dados no arquivo
    if (dataFile) 
    {
      dataFile.println(datahora);
      dataFile.close();
      Serial.println("Gravou...");      
    }  
    else 
    {
      // Mensagem de erro caso ocorra algum problema
      // na abertura do arquivo
      Serial.println("Erro ao abrir arquivo.txt !");
    }
}

void lerArquivo(String nomeArquivo, File dataFile) {
    char c;
    char* str;
    nomeArquivo.toCharArray(str, nomeArquivo.length());
    dataFile = SD.open("dataHora.txt", FILE_READ);
    Serial.print("lendo arquivo ");
    Serial.println(str);
    while(dataFile.available()) {
        c = dataFile.read();
        Serial.print(c);
    }
}

String lerRFIDString() {
        String IDstring = "";


  // Veifica o numero do cartao, grava em 
  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("Novo Cartao, "));
    IDHex = "";
    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
       nuidPICC[i] = rfid.uid.uidByte[i];
       IDstring = nuidPICC[i];
       IDHex = IDHex + IDstring;
       
    }
        idFormatado = "";
        for(int a = IDHex.length() - 3 ; a<IDHex.length(); a++) {
                idFormatado = idFormatado + IDHex[a];
                Serial.println(idFormatado);
        }
        
    Serial.print(F("ID tag:"));
    Serial.print(IDHex);
    Serial.println();
    return IDHex;
  }
  else{
    Serial.println(F("Cartao logado!"));
    Serial.println(IDHex);
    return IDHex;
  }

}


/*
 *     delay(1000);
    
    Serial.println(getData());
    String nomeArquivo = "dataHora.txt", data;
    if(flag <= 1) {
        data = getData();
        delay(100);
        Serial.println(data);
        gravaArquivo("dataHora.txt", dataFile, data);
        flag++;
    }
    else if(flag == 2) {
         lerArquivo("dataHora.txt", dataFile);
         flag++;
    }
    else if(flag == 3){
         Serial.println("Acabou o loop..");
         flag++;
    }
 */
