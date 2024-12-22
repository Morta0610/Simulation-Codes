#include <LiquidCrystal.h>

// LCD ekran pin bağlantıları
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// HX711 sensör pin bağlantıları
#define DT A0
#define SCK A1
#define sw 8

// Logic state pinleri
#define logicPinChips 13 // Cips için logic pin
#define logicPinApple 12 // Elma için logic pin
#define ledPin 1         // LED pinini 1. pin olarak ayarlıyoruz

// Global değişkenler
long sample = 0;
float val = 0;
long count = 0;
float totalPrice = 0;  // Toplam fiyat
String currentProduct = "";  // Şu anki ürün adı
float productWeight = 0;     // Ürünün kilosu
float productPrice = 0;      // Ürünün fiyatı
int weight = 0;              // Load Cell'den okunan ağırlık
bool productAdded = false;   // Ürün eklenip eklenmediğini kontrol etmek için değişken

// Ürün bilgileri
float chipsWeight = 100.0;   // Cipsin kilosu 100 gram
float chipsPrice = 25.0;     // Cipsin fiyatı 25 TL
float appleWeight = 333.0;   // Elmanın kilosu 333 gram
float applePrice = 30.0;     // Elmanın fiyatı 30 TL

// HX711 sensöründen veri okumak için fonksiyon
unsigned long readCount(void) {
  unsigned long Count;
  unsigned char i;
  pinMode(DT, OUTPUT);
  digitalWrite(DT, HIGH);
  digitalWrite(SCK, LOW);
  Count = 0;
  pinMode(DT, INPUT);
  while (digitalRead(DT));
  for (i = 0; i < 24; i++) {
    digitalWrite(SCK, HIGH);
    Count = Count << 1;
    digitalWrite(SCK, LOW);
    if (digitalRead(DT))
      Count++;
  }
  digitalWrite(SCK, HIGH);
  Count ^= 0x800000;
  digitalWrite(SCK, LOW);
  return (Count);
}

void setup() {
  // LCD ve pin ayarları
  pinMode(SCK, OUTPUT);
  pinMode(sw, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);    // LED pinini çıkış olarak ayarla
  pinMode(logicPinChips, INPUT);   // Cips için logic state pinini giriş olarak ayarla
  pinMode(logicPinApple, INPUT);   // Elma için logic state pinini giriş olarak ayarla
  lcd.begin(16, 4);  // 16x4 ekran
  lcd.print("Weight Measurement");
  delay(1000);
  lcd.clear();
  calibrate();  // Kalibrasyonu başlat
}

void loop() {
  // Load Cell'den ağırlığı oku
  count = readCount();
  weight = (((count - sample) / val) - 2 * ((count - sample) / val));  // Yeni hesaplama
  
  // Ekranda Load Cell'den gelen ağırlığı göster
  lcd.setCursor(0, 0);
  lcd.print("Weight: ");
  lcd.print(weight);
  lcd.print(" g");

  // Cips logic pinini kontrol et
  if (digitalRead(logicPinChips) == HIGH && !productAdded) {
    currentProduct = "Cips";         // Ürün adı Cips
    productWeight = chipsWeight;     // Ürün kilo bilgisi
    productPrice = chipsPrice;       // Ürün fiyatı
    productAdded = true;             // Ürün eklenmiş olduğu için tekrar eklenemez
    lcd.clear();  // Ekranı temizle
  }
  // Elma logic pinini kontrol et
  else if (digitalRead(logicPinApple) == HIGH && !productAdded) {
    currentProduct = "Elma";         // Ürün adı Elma
    productWeight = appleWeight;     // Ürün kilo bilgisi
    productPrice = applePrice;       // Ürün fiyatı
    productAdded = true;             // Ürün eklenmiş olduğu için tekrar eklenemez
    lcd.clear();  // Ekranı temizle
  }

  // Ürün adı ve sabit ağırlığı ikinci satırda göster
  lcd.setCursor(0, 1);
  lcd.print(currentProduct);  // Ürün adı
  lcd.print(" ");
  lcd.print(productWeight);   // Sabit ürün ağırlığı
  lcd.print(" g");

  // Ürün fiyatını 3. satırda göster
  lcd.setCursor(0, 2);
  lcd.print("Price: ");
  lcd.print(productPrice);
  lcd.print(" TL");

  // Ağırlık ile ürünün kilogramını karşılaştır
  if (weight != productWeight) {
    digitalWrite(ledPin, HIGH);  // LED yanacak
  } else {
    digitalWrite(ledPin, LOW);   // LED sönük
  }

  // Ürün eklenmediyse toplam fiyatı arttır
  if (productAdded) {
    totalPrice += productPrice;  // Toplam fiyata ürünün fiyatını ekle
    productAdded = false;         // Ürün bir kez eklendikten sonra tekrar eklenmesini engelle
  }

  // Toplam fiyatı 4. satırda göster
  lcd.setCursor(0, 3);
  lcd.print("Total: ");
  lcd.print(totalPrice);
  lcd.print(" TL");

  // Butona basıldığında toplam fiyatı sıfırla ve ürün eklemeyi sıfırla
  if (digitalRead(sw) == LOW) {
    totalPrice = 0;  // Toplam fiyatı sıfırla
    currentProduct = "";  // Ürün adını sıfırla
    productAdded = false; // Ürün eklenmedi olarak işaretle
    calibrate();  // Kalibrasyonu başlat
    lcd.clear();   // Ekranı temizle
  }

  delay(500);  // Her 0.5 saniyede bir okuma yap
}

// Kalibrasyon fonksiyonu
void calibrate() {
  lcd.clear();
  lcd.print("Calibrating...");
  lcd.setCursor(0, 1);
  lcd.print("Please Wait...");
  for (int i = 0; i < 100; i++) {
    count = readCount();
    sample += count;  // Duyusal örneklem alma
  }
  sample /= 100;

  lcd.clear();
  lcd.print("Put 100g weight");
  count = 0;
  while (count < 1000) {
    count = readCount();
    count = sample - count;
  }

  lcd.clear();
  lcd.print("Calibrated!");
  delay(2000);

  for (int i = 0; i < 100; i++) {
    count = readCount();
    val += sample - count;
  }

  val = val / 100.0;
  val = val / 100.0; // Kullandığınız kalibrasyon ağırlığını buraya yazın (100g)
  lcd.clear();
  lcd.print("Calibration Done!");
}
