#include <Ethernet.h>
#include <SPI.h>
#include <Bounce2.h> // Эту библиотеку необходимо скачать тут: https://github.com/thomasfredericks/Bounce-Arduino-Wiring

byte mac[] = {0x90,0xA2,0xDA,0x0E,0xF1,0x92}; // MAC-адрес нашего устройства (написан на наклейке платы Ethernet shield)
IPAddress ip(192,168,1,11); // IP адрес, если вдруг не получится получить его через DHCP
//IPAddress server(192,168,1,10); // ip-адрес удалённого сервера (использовался, пока не было имени)
char server[] = "smarthome.mydomain.ru"; // Имя удалённого сервера
char request[40]; // Переменная для формирования ссылок
int CounterPin[6] = {22,23,24,25,26,27}; // Объявляем массив пинов, на которых висят счетчики
char *CounterName[6] = {"0300181","0293594","0300125","0295451","0301008","0293848"}; // Объявляем массив имен счетчиков, которые мы будем передавать на сервер
Bounce CounterBouncer[6] = {}; // Формируем для счетчиков Bounce объекты
EthernetClient rclient; // Объект для соединения с сервером

void setup() {
  //Serial.begin(9600);
  for (int i=0; i<6; i++) {
    pinMode(CounterPin[i], INPUT); // Инициализируем пин
    digitalWrite(CounterPin[i], HIGH); // Включаем подтягивающий резистор
    CounterBouncer[i].attach(CounterPin[i]); // Настраиваем Bouncer
    CounterBouncer[i].interval(10); // и прописываем ему интервал дребезга
  }
  // Инициализируем сеть
  if (Ethernet.begin(mac) == 0) {
    Ethernet.begin(mac, ip); // Если не получилось подключиться по DHCP, пробуем еще раз с явно указанным IP адресом
  }
  delay(1000); // даем время для инициализации Ethernet shield
}

void loop() {
  delay(1000); // Задержка в 1 сек, пусть будет. Мы уверены, что два раза в секунду счетчик не может сработать ни при каких  обстоятельствах, потому что одно срабатывание - 10 литров.
  // Проверяем состояние всех счетчиков
  for (int i=0; i<6; i++) {
    boolean changed = CounterBouncer[i].update();
    if ( changed ) {
      int value = CounterBouncer[i].read();
      // Если значение датчика стало ЗАМКНУТО
      if ( value == LOW) {
        //Serial.println(CounterPin[i]);
        sprintf(request, "GET /input.pl?object=%s HTTP/1.0", CounterName[i]); // Формируем ссылку запроса, куда вставляем имя счетчика
        sendHTTPRequest(); // Отправляем HTTP запрос
      }
    }
  }
}

// Функция отправки HTTP-запроса на сервер
void sendHTTPRequest() {
  if (rclient.connect(server,80)) {
    rclient.println(request);
    rclient.print("Host: ");
    rclient.println(server);
    rclient.println("Authorization: Basic UmI9dlPnaJI2S0f="); // Base64 строка, полученная со значения "user:password"
    rclient.println("User-Agent: Arduino Sketch/1.0");
    rclient.println();   
    rclient.stop();
  }
}
