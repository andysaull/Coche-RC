/*Mando RF
Este proyecto esta realizado por Andres Saumell.
Se trata de un mando de radiofrecuencia con una pantalla LCD TFT de 2.4" para controlar un coche, tanto sus motores como sus luces, poner un modo seguro, cambiar el tema de la pantalla o calibrar el mando*/
#include <EEPROM.h>             // Libreria para guardar y leer datos de la EEPROM del Arduino
#include <Adafruit_GFX.h>       // Libreria de graficos (para dibujar cuadrados, circulos, etc...)
#include <Adafruit_TFTLCD.h>    // Libreria especifica para controlar el hardware del LCD
#include <TouchScreen.h>        // Libreria para controlar la pantalla tactil
#include <VirtualWire.h>        // Libreria para controlar la radiofrecuencia

//Define las patillas de lectura del tactil
#define YP A3  // LCD_CS
#define XM A2  // LCD_RS
#define YM 9   // LCD_D1
#define XP 8   // LCD_D0

//Define el rango donde puede leer el tactil
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940
//Define la presion minima y la maxima para saber que estamos tocando el tactil
#define MINPRESION 10
#define MAXPRESION 1000

//Para una mejor precision del tactil medimos la resistencia entre X+ y X- y la medida obtenida la ponemos en el ultimo numero
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 100);

//Define las patillas del LCD
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET 12 //Esta patilla originalmente en el LCD esta en el pin A4. Dado que necesitamos ese pin para leer el otro potenciometro, se ha hecho un puente con wrapping llevandolo hasta el pin 12

// Asigna nombres a los colores en 16-bits para tener facilidad a la hora de usar los colores
#define NEGRO     0x0000    //0
#define AZUL      0x001F    //1
#define ROJO      0xF800    //2
#define VERDE     0x1EE3    //3
#define CYAN      0x07FF    //4
#define MAGENTA   0xF81F    //5
#define AMARILLO  0xFFE0    //6
#define BLANCO    0xFFFF    //7
#define NARANJA   0xFD20    //8
#define MORADO    0xB93F    //9
#define AZULPLAN  0x2D1E    //10
#define MARRON    0x60A2    //11
#define ROJOSCURO 0xE800    //12

//Nombres de cada pantalla con los que ademas podremos activar la pantalla adecuada
boolean ESTADO = false;
boolean AJUSTESCOCHE = false;
boolean AJUSTESMANDO = false;
boolean CALIBRACION = false;
boolean INFORMACION = false;
boolean MENU = true; //MENU lo dejamos en true porque es el primero que queremos que se active al encender el mando
//boolean DATOSGPS = false;

boolean Tema = false;

//Ajustes y activaciones del mando y coche
boolean LucesDelanteras = false;
boolean LucesTraseras = false;
boolean LucesMotor = false;

boolean ModoSeguro = false;
char ModoSeguroAjuste = 20;
boolean ModoSport = true;

//Se activa cuando empezamos a calibrar
byte flagcalibracion = 0;

int X;    //Variable horizontal del tactil
int Y;    //Variable vertical del tactil
int Z;    //Variable de presion del tactil

int colorfondo; //Color del fondo de la pantalla
int colortexto; //Color del texto de la pantalla
int colortextoinfo; //Color del texto de la pantalla de informacion
int colorcuadro;    //Color de los cuadros

int Velocidadpant; //Variable que nos sirve para jugar con ella en la pantalla y en el modo seguro
long VelocidadenvioUP;
int Velocidadenvio2UP;
int VelocidadenvioDO;
int Velocidadenvio2DO;
int Giropant;
int GiroenvioRI;
int GiroenvioLE;

long totalenvio = 0;
int envioluces = 0;
long envioseguro = 0;

char Velocidadenviofade;
char Giroenviofade;

    //Variables de porcentaje que se muestran en pantalla
byte PorcentajeVelocidadenvioUP;
byte PorcentajeVelocidadenvioAntUP; //Por cada variable de porcentaje, tenemos que poner otra variable mas para que detecte si el numero mostrado en pantalla es igual que antes para no hacer parpadear la pantalla
byte PorcentajeVelocidadUP;  //Variable en porcentaje del potenciometro (Conversion a porcentaje)
byte PorcentajeVelocidadAntUP;  //Variable en porcentaje anterior del potenciometro
byte PorcentajeVelocidadenvioDO;
byte PorcentajeVelocidadenvioAntDO;
byte PorcentajeVelocidadDO;
byte PorcentajeVelocidadAntDO;
byte PorcentajeGiroRI;
byte PorcentajeGiroAntRI;
byte PorcentajeGiroLE;
byte PorcentajeGiroAntLE;
byte PorcentajeGiroenvioRI;
byte PorcentajeGiroenvioAntRI;
byte PorcentajeGiroenvioLE;
byte PorcentajeGiroenvioAntLE;

int Velocidadmaxcalib;     //Maxima lectura del potenciometro de aceleracion
int Velocidadmincalib;     //Minima lectura del potenciometro de aceleracion
int Velocidadrepcalib;     //Lectura reposo del potenciometro de aceleracion
int Giromaxcalib;          //Maxima lectura del potenciometro de giro
int Girorepcalib;          //Minima lectura del potenciometro de giro
int Giromincalib;          //Lectura reposo del potenciometro de giro

char DireccionMemoria;

//unsigned long tiempoactual;
//unsigned long tiempoprevio = 0;

/*Array de caracteres para la RadioFrecuencia*/
char mensajefinal[10];

/*Instancia del LCD*/
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

void setup(void) {
  //Serial.begin(9600);
  vw_set_tx_pin(10);
  vw_set_rx_pin(11);
  vw_setup(3000);
  tft.reset();

  tft.begin(0x9341); //Inicializar el LCD con el driver adecuado

  colorfondo = EEPROM.read(DireccionMemoria);
  colortexto = EEPROM.read(DireccionMemoria+1);
  colortextoinfo = EEPROM.read(DireccionMemoria+2);
  colorcuadro = EEPROM.read(DireccionMemoria+3);
  LecturaColores();

  Velocidadmaxcalib = EEPROM.read(DireccionMemoria+4);
  Velocidadmincalib = EEPROM.read(DireccionMemoria+5);
  Velocidadrepcalib = EEPROM.read(DireccionMemoria+6);
  Giromaxcalib = EEPROM.read(DireccionMemoria+7);
  Giromincalib = EEPROM.read(DireccionMemoria+8);
  Girorepcalib = EEPROM.read(DireccionMemoria+9);

  tft.setRotation(3); //Rotacion horizontal
  pinMode(13, OUTPUT);
  PantallaInicio();

  tft.enterSleepMode();
  tft.setCursor(100, 100);
  tft.setTextSize(2);
  tft.setTextColor(ROJO, NEGRO);
  tft.print("Dormido");
  tft.exitSleepMode();
  BarraSuperior();
}

//********************************//
//********************************//
//EMPIEZA EL DIBUJO DE LA PANTALLA//
//******Y EL ENVIO DE DATOS*******//
//********************************//
//********************************//

void loop()
{
/*Enviamos una señal por RF con una codificacion inventada.
Anteriormente se enviaba una señal por cada potenciometro y otra para las luces.
Con este sistema se ahorra enviar muchas señales lo que se traduce en una mejora de velocidad de respuesta (lag entre mando y coche)

El primer digito es para identificar sobre que modulo estamos enviando (Potenciometros de giro, velocidad, LEDs, etc).
Los 3 siguientes son para el potenciometro de velocidad, los 3 ultimos para el de giro.
Las luces se activan con los 3 ultimos digitos
v=potenciometros de velocidad
g=potenciometros de giro
x=sin importancia
-----------------------------
1xxxxx0 = apagar luces delanteras
1xxxxx1 = encender luces delanteras
1xxxx0x = apagar luces traseras
1xxxx1x = encender luces traseras
1xxx0xx = apagar luces motor
1xxx1xx = encender luces motor
(Las luces solo se envian cuando ninguno de los dos potenciometros esta activado, es decir, cuando estan en reposo)
2vvvggg = Aceleracion adelante y giro derecha
3vvvggg = Aceleracion atras y giro derecha
4vvvggg = Aceleracion adelante y giro izquierda
5vvvggg = Aceleracion atras y giro izquierda
6xxxggg = Aceleracion 0 y giro derecha
7xxxggg = Aceleracion 0 y giro izquierda
8vvvxxx = Aceleracion adelante y giro 0
9vvvxxx = Aceleracion atras y giro 0
*/

/* Envio de un dato para activar la recepcion del mando y la emision del coche para el GPS. Actualmente desactivado
  tiempoactual = millis();
  if(tiempoactual - tiempoprevio >= 100)
  {
      tiempoprevio = tiempoactual;

      send("9999");

      vw_rx_start();
      uint8_t buf[VW_MAX_MESSAGE_LEN];
      uint8_t buflen = VW_MAX_MESSAGE_LEN;

      if (vw_get_message(buf, &buflen)) //Guardamos el mensaje en el buffer
        {
            for (int j = 0; j < buflen; j++)
            {
                mensaje[j] = char(buf[j]); //Llenamos la matriz mensaje con el buffer
            }

            datos = atoi(mensaje);
            Serial.print("Recepcion: ");
            Serial.println(datos);
        }
        vw_wait_rx_max(100);
      vw_rx_stop();
  }
*/
  Potenciometros(); //Activa la funcion de lectura y mapeo de potenciometros

  if (ModoSport == true) //Si el Modo Sport esta activado, las variables que cambian progresivamente con el modo seguro, se hacen igual que las variables recogidas por los potenciometros
  {
      VelocidadenvioUP = Velocidadenvio2UP;
      VelocidadenvioDO = Velocidadenvio2DO;
  }
  else if (ModoSeguro == true) //Si el Modo Seguro esta activado, inicia la funcion de Modo Seguro
  {
      ModoSegurof();
  }

  totalenvio = 0; //Ponemos la variable totalenvio en 0 en cada loop para que no se incremente con el bucle

  if(Velocidadpant > Velocidadrepcalib && Velocidadpant <= Velocidadmaxcalib) //Si el joystick de velocidad esta por encima del valor de reposo y por debajo del valor maximo (es decir, si movemos el joystick hacia arriba...)
  {
      totalenvio = VelocidadenvioUP*1000L; //Multiplicamos la variable por mil para codificarlo
      totalenvio = totalenvio+1000000L; //Sumamos un millon para identificar que estamos acelerando
  }
  else if(Velocidadpant < Velocidadrepcalib && Velocidadpant >= Velocidadmincalib) //Igual que el "if" anterior pero con el joystick hacia abajo
  {
      totalenvio = VelocidadenvioDO*1000L;
      totalenvio = totalenvio+2000000L; //Sumamos 2 millones para diferenciarlo del de subida
  }
  else if(Velocidadpant == Velocidadrepcalib) // Si el joystick esta en reposo...
  {
      totalenvio = totalenvio + 5000000L; //Sumamos 5 millones para codificarlo e identificar que el motor de aceleracion no se mueva
  }

  if(Giropant > Girorepcalib && Giropant <= Giromaxcalib) //Si movemos el joystick de giro a la derecha...
  {
      totalenvio = totalenvio + 1000000L; //Sumamos un millon para identificar
      totalenvio = totalenvio + GiroenvioRI; //Y lo sumamos a la variable total
  }
  else if(Giropant < Girorepcalib && Giropant >= Giromincalib) //Si giramos a la izquierda
  {
      if(Velocidadpant == Velocidadrepcalib) //Y si el joystick de aceleracion esta en reposo sumamos 2 millones
      {
          totalenvio = totalenvio + 2000000L;
      }
      else
      {
          totalenvio = totalenvio + 3000000L; //Sino le sumamos 3 millones
      }
      totalenvio = totalenvio + GiroenvioLE; //Despues le sumamos el valor a la variable total
  }
  else if(Giropant == Girorepcalib) //Si el joystick de giro esta en reposo
  {
      totalenvio = totalenvio + 7000000L; //Sumamos 7 millones
  }

  if((Velocidadpant == Velocidadrepcalib)&&(Giropant == Girorepcalib)) //Si los dos joystick estan en reposo...
  {
      totalenvio = envioseguro + envioluces + 1000000L; //Sumamos las variables del Modo Seguro y de las luces. Solo cuando los dos joystick estan en reposo se envian estas variables
      tft.fillCircle(10, 5, 4, colortextoinfo); //Ademas, dibujamos un circulo en la pantalla para saber cuando estan los joystick en reposo y cuando se envian estas variables.
  }
  else
  {
      tft.fillCircle(10, 5, 4, colortexto); //Si los joystick estan activados, dibuja el circulo de otro color
  }

  ltoa(totalenvio, mensajefinal, 10); //Convertimos la variable total de envio en un array de caracteres para poder enviarlo correctamente
  send(mensajefinal); //Invocamos la funcion de envio por RF y enviamos el mensaje

  lecturaPanel();   //Funcion para leer el panel tactil

  if(MENU == true)  //Si la variable MENU esta activada (es la primera pantalla que tenemos, asi que esta activada desde el principio)
  {
      Menu();   //Escribimos en la pantalla el menu donde elegimos las distintas cosas que queremos elegir
  }

  else if(ESTADO == true)     //Si la variable ESTADO esta activada...
  {
      Estado();   //Escribimos en la pantalla todas las informaciones (velocidad, bateria, etc...)
  }

  else if(AJUSTESCOCHE == true)     //Si la variable AJUSTESCOCHE esta activada...
  {
      AjustesCoche();   //Escribimos en la pantalla cuadros donde ajustaremos distintos parametros del coche
  }

  else if(AJUSTESMANDO == true)     //Si la variable AJUSTESMANDO esta activada...
  {
      AjustesMando();   //Escribimos en pantalla los cuadros de ajustes de mando
  }

  /*else if(DATOSGPS == true)
  {
      Datosgps();
  }*/

  else if(CALIBRACION == true)      //Si la variable de CALIBRACION esta activada...
  {
      Calibracion();     //Activamos la funcion de calibracion
  }

  else if(INFORMACION == true)      //Si la variable de INFORMACION esta activada...
  {
      Informacion();     //Activamos la pantalla de informacion
  }

}


//************************//
//************************//
//DESCRIPCION DE FUNCIONES//
//************************//
//************************//

void send (char *message) //Funcion para enviar el mensaje por RF
{
  vw_send((uint8_t *)message, 10); //Envia el mensaje
  vw_wait_tx(); //Espera hasta que se haya acabado de transmitir todo

  //Serial.print("Mensaje: ");
  //Serial.println(message); //Muestra el mensaje por Serial
}

void lecturaPanel() //Funcion que lee la presion que estamos haciendo en el tactil y la posicion y la almacena en una variable
{
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  Y=map(p.x, TS_MAXX, TS_MINX, 0, 240);
  X=map(p.y, TS_MAXY, TS_MINY, 320, 0);
  Z=p.z;
}

void Potenciometros() //Funcion que lee los potenciometros, los almacena en distintas variables y los mapea para diferentes funciones (unas para el modo seguro, otras para leer exclusivamente en pantalla...)
{
  Velocidadpant = analogRead(A5);    //Lee el potenciometro de velocidad y lo almacena en la variable
  Velocidadpant = map(Velocidadpant, 0, 1023, 0, 255);    //Mapeamos para tener un valor entre 0-255 para el PWM

  //Esta variable de 0-255 indica el potenciometro de velocidad desde el reposo hasta el maximo.
  Velocidadenvio2UP = map(Velocidadpant, Velocidadrepcalib, Velocidadmaxcalib, 0, 255);
  if (Velocidadenvio2UP > 255)
    Velocidadenvio2UP = 255;
  if (Velocidadenvio2UP <= 30)
    Velocidadenvio2UP = 0;

  //Igual que la variable de Velocidadenvio2UP pero para el minimo del potenciometro
  Velocidadenvio2DO = map(Velocidadpant, Velocidadrepcalib, Velocidadmincalib, 0, 255);
  if (Velocidadenvio2DO > 255)
    Velocidadenvio2DO = 255;
  if (Velocidadenvio2DO <= 30)
    Velocidadenvio2DO = 0;

  //Ajuste de la variable con la que podemos jugar para mostrar datos en la pantalla de la velocidad
  if (Velocidadpant > Velocidadmaxcalib)
    Velocidadpant = Velocidadmaxcalib;
  if (Velocidadpant < Velocidadmincalib)
    Velocidadpant = Velocidadmincalib;
  if ((Velocidadpant > (Velocidadrepcalib - 3)) && (Velocidadpant < (Velocidadrepcalib + 3)))
  {
    Velocidadpant = Velocidadrepcalib;
  }

  Giropant = analogRead(A4);
  Giropant = map(Giropant, 0, 1023, 0, 255);



  //Esta variable de 0-255 indica el potenciometro de velocidad desde el reposo hasta el maximo.
  GiroenvioRI = map(Giropant, Girorepcalib, Giromaxcalib, 0, 255);
  if (GiroenvioRI >= 255)
    GiroenvioRI = 255;
  else if (GiroenvioRI <= 0)
    GiroenvioRI = 0;

  //Igual que la variable de GiroenvioRI pero para el minimo del potenciometro
  GiroenvioLE = map(Giropant, Girorepcalib, Giromincalib, 0, 255);
  if (GiroenvioLE >= 255)
    GiroenvioLE = 255;
  if (GiroenvioLE <= 0)
    GiroenvioLE = 0;

  if (Giropant > Giromaxcalib)
    Giropant = Giromaxcalib;
  if (Giropant < Giromincalib)
    Giropant = Giromincalib;
  if ((Giropant > (Girorepcalib - 10)) && (Giropant < (Girorepcalib + 10)))
  {
    Giropant = Girorepcalib;
  }

}

void PantallaInicio() //Funcion para la pantalla de inicio con el triangulo que se va rellenando poco a poco
{
    int           i, cx = tft.width()  / 2 - 1,
                   cy = tft.height() / 2 - 1;

    tft.fillScreen(colorfondo);
    for(i=min(cx,cy); i>2; i-=5)
        {
            tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
            //tft.color565(cx, cy, i));
            colorcuadro);
            tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
            tft.color565(cy, i, i));
            //MARRON);
        }

    delay(500);
    tft.setCursor(40, 100);
    tft.setTextColor(colortexto);
    tft.setTextSize(3);
    tft.print("Mando RF V1.1");
    tft.setCursor(40, 130);
    delay(250);
    tft.print("Estable");

    delay(3000);


    tft.fillScreen(colorfondo);
}

void Menu() //Funcion con los cuadros del menu
{
    if(MENU == true)        //Estado
    {
        tft.drawRoundRect(20, 50, 80, 50, 5, colorcuadro); //Dibujamos un cuadro redondeado
        tft.setTextColor(colortexto); //Ajustamos el color del texto
        tft.setTextSize(2); //Ajustamos el tamaño del texto
        tft.setCursor(25, 67); //Ajustamos el lugar donde vamos a colocar el texto
        tft.print("Estado"); //Escribimos el texto
        if((X>20 && X<100) && (Y>50 && Y<100) && (Z>MINPRESION && Z<MAXPRESION)) //Si tocamos en el cuadro que esta dibujado...
        {
            MENU = false; //La variable de MENU la desactivamos porque pasamos de pantalla
            ESTADO = true; //Y activamos la variable de ESTADO
            BorrarPantalla(); //Limpiamos la pantalla con una funcion
        }
    }

    if(MENU == true)        //Ajustes Coche
    {
        if (LucesDelanteras == true)
        {
            tft.fillCircle(130, 95, 3, colortextoinfo);
        }
        else if (LucesDelanteras == false)
        {
            tft.fillCircle(130, 95, 3, colorfondo);
        }
        if (LucesTraseras == true)
        {
            tft.fillCircle(160, 95, 3, colortextoinfo);
        }
        else if (LucesTraseras == false)
        {
            tft.fillCircle(160, 95, 3, colorfondo);
        }
        if (LucesMotor == true)
        {
            tft.fillCircle(190, 95, 3, colortextoinfo);
        }
        else if (LucesMotor == false)
        {
            tft.fillCircle(190, 95, 3, colorfondo);
        }

        tft.drawRoundRect(120, 50, 80, 50, 5, colorcuadro);
        tft.setTextSize(1);
        tft.setCursor(140, 60);
        tft.print("Ajustes");
        tft.setCursor(145, 80);
        tft.print("Coche");
        if((X>120 && X<200) && (Y>50 && Y<100) && (Z>MINPRESION && Z<MAXPRESION))
        {
            MENU = false;
            AJUSTESCOCHE = true;
            BorrarPantalla();
        }
    }

    if(MENU == true)        //Ajustes Mando
    {
        if(ModoSport == true)
        {
            tft.drawRoundRect(220, 50, 80, 50, 5, colortextoinfo);
        }
        else if(ModoSeguro == true)
        {
            tft.drawRoundRect(220, 50, 80, 50, 5, colorcuadro);
        }
        tft.setCursor(240, 60);
        tft.print("Ajustes");
        tft.setCursor(245, 80);
        tft.print("Mando");
        if((X>220 && X<300) && (Y>50 && Y<100) && (Z>MINPRESION && Z<MAXPRESION))
        {
            MENU = false;
            AJUSTESMANDO = true;
            BorrarPantalla();
        }
    }

    /*if(MENU == true)        //Datos GPS
    {
        tft.drawRoundRect(20, 120, 80, 50, 5, colorcuadro);
        tft.setCursor(45, 130);
        tft.print("Datos");
        tft.setCursor(50, 150);
        tft.print("GPS");
        if((X>20 && X<100) && (Y>120 && Y<170) && (Z>MINPRESION && Z<MAXPRESION))
        {
            MENU = false;
            DATOSGPS = true;
            BorrarPantalla();
        }
    }*/

    if(MENU == true)
    {
        tft.drawRoundRect(70, 120, 80, 50, 5, colorcuadro);
        tft.setCursor(77, 140);
        tft.print("Calibracion");
        if((X>70 && X<150) && (Y>120 && Y<170) && (Z>MINPRESION && Z<MAXPRESION))
        {
            MENU = false;
            CALIBRACION = true;
            BorrarPantalla();
        }
    }

    if(MENU == true)        //Informacion
    {
        tft.drawRoundRect(170, 120, 80, 50, 5, colorcuadro);
        tft.setCursor(175, 140);
        tft.print("Informacion");
        if((X>170 && X<290) && (Y>120 && Y<170) && (Z>MINPRESION && Z<MAXPRESION))
        {
            MENU = false;
            INFORMACION = true;
            BorrarPantalla();
        }
    }
}

void Estado()
{
    tft.setCursor(30, 50);
    tft.setTextColor(colorcuadro);
    tft.setTextSize(2);
    tft.print("Velocidad joystick:"); //Escribimos el texto fijo en la pantalla
    if((Velocidadpant>=Velocidadrepcalib) && (Velocidadpant<=Velocidadmaxcalib)) //Si el joystick de subida esta activado
    {
        PorcentajeVelocidadUP = map(Velocidadpant, Velocidadrepcalib, Velocidadmaxcalib, 0, 100); //Mapeamos de 0-100 la variable de velocidad de subida

        if(PorcentajeVelocidadAntDO-PorcentajeVelocidadUP>1||PorcentajeVelocidadAntDO-PorcentajeVelocidadUP<-1) //
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 50);
            tft.print(PorcentajeVelocidadAntDO);
            tft.print("%");
            PorcentajeVelocidadAntDO = PorcentajeVelocidadUP;
        }

        if(PorcentajeVelocidadUP-PorcentajeVelocidadAntUP>1||PorcentajeVelocidadUP-PorcentajeVelocidadAntUP<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 50);
            tft.print(PorcentajeVelocidadAntUP);
            tft.print("%");

            tft.setTextColor(colortextoinfo);
            tft.setCursor(270, 50);
            tft.print(PorcentajeVelocidadUP);
            tft.print("%");
            PorcentajeVelocidadAntUP = PorcentajeVelocidadUP;
        }
    }


    else if((Velocidadpant<=Velocidadrepcalib) && (Velocidadpant>=Velocidadmincalib))
    {
        PorcentajeVelocidadDO = map(Velocidadpant, Velocidadrepcalib, Velocidadmincalib, 0, 100);

        if(PorcentajeVelocidadAntUP-PorcentajeVelocidadDO>1||PorcentajeVelocidadAntUP-PorcentajeVelocidadDO<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 50);
            tft.print(PorcentajeVelocidadAntUP);
            tft.print("%");
            PorcentajeVelocidadAntUP = PorcentajeVelocidadDO;
        }

        if(PorcentajeVelocidadDO-PorcentajeVelocidadAntDO>1||PorcentajeVelocidadDO-PorcentajeVelocidadAntDO<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 50);
            tft.print(PorcentajeVelocidadAntDO);
            tft.print("%");

            tft.setTextColor(colortexto);
            tft.setCursor(270, 50);
            tft.print(PorcentajeVelocidadDO);
            tft.print("%");
            PorcentajeVelocidadAntDO = PorcentajeVelocidadDO;
        }
    }

    tft.setCursor(30, 70);
    tft.setTextColor(colorcuadro);
    tft.setTextSize(2);
    tft.print("Velocidad ajustada:");
    if((Velocidadpant>=Velocidadrepcalib) && (Velocidadpant<=Velocidadmaxcalib))
    {
        PorcentajeVelocidadenvioUP = map(VelocidadenvioUP, 0, 255, 0, 100);

        if(PorcentajeVelocidadenvioAntDO-PorcentajeVelocidadenvioUP>1||PorcentajeVelocidadenvioAntDO-PorcentajeVelocidadenvioUP<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 70);
            tft.print(PorcentajeVelocidadenvioAntDO);
            tft.print("%");
            PorcentajeVelocidadenvioAntDO = PorcentajeVelocidadenvioUP;
        }

        if(PorcentajeVelocidadenvioUP-PorcentajeVelocidadenvioAntUP>1||PorcentajeVelocidadenvioUP-PorcentajeVelocidadenvioAntUP<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 70);
            tft.print(PorcentajeVelocidadenvioAntUP);
            tft.print("%");

            tft.setTextColor(colortextoinfo);
            tft.setCursor(270, 70);
            tft.print(PorcentajeVelocidadenvioUP);
            tft.print("%");
            PorcentajeVelocidadenvioAntUP = PorcentajeVelocidadenvioUP;
        }
    }


    else if((Velocidadpant<=Velocidadrepcalib) && (Velocidadpant>=Velocidadmincalib))
    {
        PorcentajeVelocidadenvioDO = map(VelocidadenvioDO, 0, 255, 0, 100);

        if(PorcentajeVelocidadenvioAntUP-PorcentajeVelocidadenvioDO>1||PorcentajeVelocidadenvioAntUP-PorcentajeVelocidadenvioDO<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 70);
            tft.print(PorcentajeVelocidadenvioAntUP);
            tft.print("%");
            PorcentajeVelocidadenvioAntUP = PorcentajeVelocidadenvioDO;
        }

        if(PorcentajeVelocidadenvioDO-PorcentajeVelocidadenvioAntDO>1||PorcentajeVelocidadenvioDO-PorcentajeVelocidadenvioAntDO<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 70);
            tft.print(PorcentajeVelocidadenvioAntDO);
            tft.print("%");

            tft.setTextColor(colortexto);
            tft.setCursor(270, 70);
            tft.print(PorcentajeVelocidadenvioDO);
            tft.print("%");
            PorcentajeVelocidadenvioAntDO = PorcentajeVelocidadenvioDO;
        }
    }

    tft.setCursor(30, 110);
    tft.setTextColor(colorcuadro);
    tft.setTextSize(2);
    tft.print("Giro joystick:");
    if((Giropant>=Girorepcalib) && (Giropant<=Giromaxcalib))
    {
        PorcentajeGiroRI = map(Giropant, Girorepcalib, Giromaxcalib, 0, 100);

        if(PorcentajeGiroAntRI-PorcentajeGiroRI>1||PorcentajeGiroAntRI-PorcentajeGiroRI<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 110);
            tft.print(PorcentajeGiroAntLE);
            tft.print("%");
            PorcentajeGiroAntLE = PorcentajeGiroRI;
        }

        if(PorcentajeGiroRI-PorcentajeGiroAntRI>1||PorcentajeGiroRI-PorcentajeGiroAntRI<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 110);
            tft.print(PorcentajeGiroAntRI);
            tft.print("%");

            tft.setTextColor(colortextoinfo);
            tft.setCursor(270, 110);
            tft.print(PorcentajeGiroRI);
            tft.print("%");
            PorcentajeGiroAntRI = PorcentajeGiroRI;
        }
    }


    else if((Giropant<=Girorepcalib) && (Giropant>=Giromincalib))
    {
        PorcentajeGiroLE = map(Giropant, Girorepcalib, Giromincalib, 0, 100);

        if(PorcentajeGiroAntRI-PorcentajeGiroLE>1||PorcentajeGiroAntRI-PorcentajeGiroLE<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 110);
            tft.print(PorcentajeGiroAntRI);
            tft.print("%");
            PorcentajeGiroAntRI = PorcentajeGiroLE;
        }

        if(PorcentajeGiroLE-PorcentajeGiroAntLE>1||PorcentajeGiroLE-PorcentajeGiroAntLE<-1)
        {
            tft.setTextColor(colorfondo);
            tft.setCursor(270, 110);
            tft.print(PorcentajeGiroAntLE);
            tft.print("%");

            tft.setTextColor(colortexto);
            tft.setCursor(270, 110);
            tft.print(PorcentajeGiroLE);
            tft.print("%");
            PorcentajeGiroAntLE = PorcentajeGiroLE;
        }
    }

/*
    tft.setCursor(30, 110);
    tft.setTextColor(colorcuadro);
    tft.setTextSize(2);
    tft.print("Giro ajustado:");
    PorcentajeGiroenvio = map(Giroenvio, 0, 255, 0, 100);
    if(PorcentajeGiroenvio-PorcentajeGiroenvioAnt>1||PorcentajeGiroenvio-PorcentajeGiroenvioAnt<-1)
    {
        tft.setTextColor(colorfondo);
        tft.setCursor(270, 110);
        tft.print(PorcentajeGiroenvioAnt);
        tft.print("%");

        tft.setTextColor(colortextoinfo);
        tft.setCursor(270, 110);
        tft.print(PorcentajeGiroenvio);
        tft.print("%");
        PorcentajeGiroenvioAnt = PorcentajeGiroenvio;
    }

    tft.setCursor(30, 130);
    tft.setTextColor(colorcuadro);
    tft.setTextSize(2);
    tft.print("Recibido: ");
    if(datos-datosAnt>1||datos-datosAnt<-1)
    {
        tft.setTextColor(colorfondo);
        tft.setCursor(240, 130);
        tft.print(datosAnt);
        tft.print("Km/h");

        tft.setTextColor(colortextoinfo);
        tft.setCursor(240, 130);
        tft.print(datos);
        tft.print("Km/h");
        datosAnt = datos;
    }
    */
    lecturaPanel();
    BotonMenu();
}

void AjustesCoche()
{
    if(AJUSTESCOCHE == true)
    {
        if(LucesDelanteras == true)
        {
            tft.fillRoundRect(20, 80, 80, 50, 5, colorcuadro);
            tft.setTextColor(colortexto);
            tft.setTextSize(1);
            tft.setCursor(45, 90);
            tft.print("Luces");
            tft.setCursor(30, 110);
            tft.print("delanteras");
            if((X>20 && X<100) && (Y>80 && Y<130) && (Z>MINPRESION && Z<MAXPRESION))
            {
                LucesDelanteras = false;
                envioluces = envioluces - 1;
                BorrarPantalla();
            }
        }

        else
        {
            tft.drawRoundRect(20, 80, 80, 50, 5, colorcuadro);
            tft.setCursor(45, 90);
            tft.print("Luces");
            tft.setCursor(30, 110);
            tft.print("delanteras");
            if((X>20 && X<100) && (Y>80 && Y<130) && (Z>MINPRESION && Z<MAXPRESION))
            {
                LucesDelanteras = true;
                envioluces = envioluces + 1;
                BorrarPantalla();
            }
        }
        if(LucesTraseras == true)
        {
            tft.fillRoundRect(120, 80, 80, 50, 5, colorcuadro);
            tft.setCursor(145, 90);
            tft.print("Luces");
            tft.setCursor(130, 110);
            tft.print("traseras");
            if((X>120 && X<200) && (Y>80 && Y<130) && (Z>MINPRESION && Z<MAXPRESION))
            {
                LucesTraseras = false;
                envioluces = envioluces - 10;
                BorrarPantalla();
            }
        }
        else
        {
            tft.drawRoundRect(120, 80, 80, 50, 5, colorcuadro);
            tft.setCursor(145, 90);
            tft.print("Luces");
            tft.setCursor(135, 110);
            tft.print("traseras");
            if((X>120 && X<200) && (Y>80 && Y<130) && (Z>MINPRESION && Z<MAXPRESION))
            {
                LucesTraseras = true;
                envioluces = envioluces + 10;
                BorrarPantalla();
            }
        }
        if(LucesMotor == true)
        {
            tft.fillRoundRect(220, 80, 80, 50, 5, colorcuadro);
            tft.setCursor(245, 90);
            tft.print("Luces");
            tft.setCursor(245, 110);
            tft.print("motor");
            if((X>220 && X<300) && (Y>80 && Y<130) && (Z>MINPRESION && Z<MAXPRESION))
            {
                LucesMotor = false;
                envioluces = envioluces - 100;
                BorrarPantalla();
            }
        }
        else
        {
            tft.drawRoundRect(220, 80, 80, 50, 5, colorcuadro);
            tft.setCursor(245, 90);
            tft.print("Luces");
            tft.setCursor(245, 110);
            tft.print("motor");
            if((X>220 && X<300) && (Y>80 && Y<130) && (Z>MINPRESION && Z<MAXPRESION))
            {
                LucesMotor = true;
                envioluces = envioluces + 100;
                BorrarPantalla();
            }
        }
    }
    BotonMenu();
}

void AjustesMando()
{
    if(AJUSTESMANDO == true)
    {
        if(Tema == true)
        {
            tft.fillRoundRect(20, 50, 80, 50, 5, colorcuadro);
            tft.setTextColor(colortexto);
            tft.setTextSize(1);
            tft.setCursor(50, 60);
            tft.print("Tema");
            tft.setCursor(40, 80);
            tft.print("colores");

            ColoresDeAjuste();

            if((X>20 && X<100) && (Y>50 && Y<100) && (Z>MINPRESION && Z<MAXPRESION))
            {
                Tema = false;
                BorrarPantalla();
            }
        }

        else
        {
            tft.drawRoundRect(20, 50, 80, 50, 5, colorcuadro);
            tft.setTextColor(colortexto);
            tft.setCursor(50, 60);
            tft.print("Tema");
            tft.setCursor(40, 80);
            tft.print("colores");
            if((X>20 && X<100) && (Y>50 && Y<100) && (Z>MINPRESION && Z<MAXPRESION))
            {
                Tema = true;
                BorrarPantalla();
            }
        }
    }
    if(AJUSTESMANDO == true)
    {
            if(ModoSport == true)
        {
            tft.fillRoundRect(120, 50, 80, 50, 5, colorcuadro);
            tft.setTextColor(colortextoinfo);
            tft.setCursor(150, 60);
            tft.print("Modo");
            tft.setCursor(146, 80);
            tft.print("Sport");
        }
        else
        {
            tft.drawRoundRect(120, 50, 80, 50, 5, colorcuadro);
            tft.setTextColor(colortextoinfo);
            tft.setCursor(150, 60);
            tft.print("Modo");
            tft.setCursor(146, 80);
            tft.print("Sport");
            if((X>120 && X<200) && (Y>50 && Y<100) && (Z>MINPRESION && Z<MAXPRESION))
            {
                ModoSport = true;
                ModoSeguro = false;
                envioseguro = 0;
                Tema = false;
                BorrarPantalla();
            }
        }
    }
    if(AJUSTESMANDO == true)
    {
        if(ModoSeguro == true)
        {
            tft.fillRoundRect(220, 50, 80, 50, 5, colorcuadro);
            tft.setTextColor(colortexto);
            tft.setCursor(250, 60);
            tft.print("Modo");
            tft.setCursor(242, 80);
            tft.print("Seguro");
            if((X>220 && X<300) && (Y>50 && Y<100) && (Z>MINPRESION && Z<MAXPRESION))
                {
                    Tema = false;
                    BorrarPantalla();
                }

            if ((Tema == false) && (ModoSeguroAjuste == 30))
            {
                tft.fillRoundRect(20, 120, 80, 50, 5, colorcuadro);
                tft.setTextColor(colortexto);
                tft.setTextSize(2);
                tft.setCursor(32, 136);
                tft.print("Suave");
                envioseguro = 1000;
            }
            else if (Tema == false)
            {
                tft.drawRoundRect(20, 120, 80, 50, 5, colorcuadro);
                tft.setTextColor(colortexto);
                tft.setTextSize(2);
                tft.setCursor(32, 136);
                tft.print("Suave");
                if((X>20 && X<100) && (Y>120 && Y<170) && (Z>MINPRESION && Z<MAXPRESION))
                {
                    ModoSeguroAjuste = 30;
                    BorrarPantalla();
                }
            }
            if ((Tema == false) && (ModoSeguroAjuste == 20))
            {
                tft.fillRoundRect(120, 120, 80, 50, 5, colorcuadro);
                tft.setTextColor(colortexto);
                tft.setTextSize(2);
                tft.setCursor(132, 136);
                tft.print("Medio");
                envioseguro = 2000;
            }
            else if (Tema == false)
            {
                tft.drawRoundRect(120, 120, 80, 50, 5, colorcuadro);
                tft.setTextColor(colortexto);
                tft.setTextSize(2);
                tft.setCursor(132, 136);
                tft.print("Medio");
                if((X>120 && X<200) && (Y>120 && Y<170) && (Z>MINPRESION && Z<MAXPRESION))
                    {
                        ModoSeguroAjuste = 20;
                        BorrarPantalla();
                    }
            }
            if ((Tema == false) && (ModoSeguroAjuste == 10))
            {
                tft.fillRoundRect(220, 120, 80, 50, 5, colorcuadro);
                tft.setTextColor(colortexto);
                tft.setTextSize(2);
                tft.setCursor(225, 136);
                tft.print("Fuerte");
                envioseguro = 3000;
            }
            else if (Tema == false)
            {
                tft.drawRoundRect(220, 120, 80, 50, 5, colorcuadro);
                tft.setTextColor(colortexto);
                tft.setTextSize(2);
                tft.setCursor(225, 136);
                tft.print("Fuerte");
                if((X>220 && X<300) && (Y>120 && Y<170) && (Z>MINPRESION && Z<MAXPRESION))
                    {
                        ModoSeguroAjuste = 10;
                        BorrarPantalla();
                    }
            }
        }
        else
        {
            tft.drawRoundRect(220, 50, 80, 50, 5, colorcuadro);
            tft.setTextColor(colortexto);
            tft.setTextSize(1);
            tft.setCursor(250, 60);
            tft.print("Modo");
            tft.setCursor(242, 80);
            tft.print("Seguro");
            if((X>220 && X<300) && (Y>50 && Y<100) && (Z>MINPRESION && Z<MAXPRESION))
            {
                ModoSeguro = true;
                ModoSport = false;
                Tema = false;
                BorrarPantalla();
            }
        }
    }
    BotonMenu();
}

/*void Datosgps()
{

    BotonMenu();
}*/

void Calibracion()
{
    tft.setCursor(15, 50);
    tft.setTextColor(colortexto);
    tft.setTextSize(2);
    tft.print("Se parara la");
    tft.setCursor(15, 70);
    tft.print("comunicacion con el coche");
    tft.setCursor(150, 90);
    delay(750);
    tft.print(".");
    delay(750);
    tft.print(".");
    delay(750);
    tft.print(".");
    delay(2000);
    BorrarPantalla();

    flagcalibracion = 1;
    while (flagcalibracion == 1)
    {
        tft.setCursor(50, 50);
        tft.print("<---- Aceleracion");
        tft.setCursor(15, 100);
        tft.print("Acelera al maximo y");
        tft.setCursor(15, 120);
        tft.print("despues pulsa la pantalla");
        lecturaPanel();
        if(Z>MINPRESION && Z<MAXPRESION)
        {
            Velocidadmaxcalib = analogRead(A5);
            Velocidadmaxcalib = map(Velocidadmaxcalib, 0, 1023, 0, 255);
            EEPROM.write(DireccionMemoria+4, Velocidadmaxcalib);
            flagcalibracion = 2;
            tft.setCursor(150, 200);
            tft.print(Velocidadmaxcalib);
            delay(1500);
            BorrarPantalla();
        }
    }
    while (flagcalibracion == 2)
    {
        tft.setCursor(50, 50);
        tft.print("<---- Aceleracion");
        tft.setCursor(15, 100);
        tft.print("Acelera al minimo y");
        tft.setCursor(15, 120);
        tft.print("despues pulsa la pantalla");
        lecturaPanel();
        if(Z>MINPRESION && Z<MAXPRESION)
        {
            Velocidadmincalib = analogRead(A5);
            Velocidadmincalib = map(Velocidadmincalib, 0, 1023, 0, 255);
            EEPROM.write(DireccionMemoria+5, Velocidadmincalib);
            flagcalibracion = 3;
            tft.setCursor(150, 200);
            tft.print(Velocidadmincalib);
            delay(1500);
            BorrarPantalla();
        }
    }
    while (flagcalibracion == 3)
    {
        tft.setCursor(50, 50);
        tft.print("<---- Aceleracion");
        tft.setCursor(15, 100);
        tft.print("No aceleres y");
        tft.setCursor(15, 120);
        tft.print("despues pulsa la pantalla");
        lecturaPanel();
        if(Z>MINPRESION && Z<MAXPRESION)
        {
            Velocidadrepcalib = analogRead(A5);
            Velocidadrepcalib = map(Velocidadrepcalib, 0, 1023, 0, 255);
            EEPROM.write(DireccionMemoria+6, Velocidadrepcalib);
            flagcalibracion = 4;
            tft.setCursor(150, 200);
            tft.print(Velocidadrepcalib);
            delay(1500);
            BorrarPantalla();
        }
    }
    while (flagcalibracion == 4)
    {
        tft.setCursor(125, 50);
        tft.print("Giro ---->");
        tft.setCursor(15, 100);
        tft.print("Gira a la derecha y");
        tft.setCursor(15, 120);
        tft.print("despues pulsa la pantalla");
        lecturaPanel();
        if(Z>MINPRESION && Z<MAXPRESION)
        {
            Giromaxcalib = analogRead(A4);
            Giromaxcalib = map(Giromaxcalib, 0, 1023, 0, 255);
            EEPROM.write(DireccionMemoria+7, Giromaxcalib);
            flagcalibracion = 5;
            tft.setCursor(150, 200);
            tft.print(Giromaxcalib);
            delay(1500);
            BorrarPantalla();
        }
    }
    while (flagcalibracion == 5)
    {
        tft.setCursor(125, 50);
        tft.print("Giro ---->");
        tft.setCursor(15, 100);
        tft.print("Gira a la izquierda y");
        tft.setCursor(15, 120);
        tft.print("despues pulsa la pantalla");
        lecturaPanel();
        if(Z>MINPRESION && Z<MAXPRESION)
        {
            Giromincalib = analogRead(A4);
            Giromincalib = map(Giromincalib, 0, 1023, 0, 255);
            EEPROM.write(DireccionMemoria+8, Giromincalib);
            flagcalibracion = 6;
            tft.setCursor(150, 200);
            tft.print(Giromincalib);
            delay(1500);
            BorrarPantalla();
        }
    }
    while (flagcalibracion == 6)
    {
        tft.setCursor(125, 50);
        tft.print("Giro ---->");
        tft.setCursor(15, 100);
        tft.print("No gires y");
        tft.setCursor(15, 120);
        tft.print("despues pulsa la pantalla");
        lecturaPanel();
        if(Z>MINPRESION && Z<MAXPRESION)
        {
            Girorepcalib = analogRead(A4);
            Girorepcalib = map(Girorepcalib, 0, 1023, 0, 255);
            EEPROM.write(DireccionMemoria+9, Girorepcalib);
            flagcalibracion = 0;
            tft.setCursor(150, 200);
            tft.print(Girorepcalib);
            tft.setCursor(140, 220);
            tft.print("Exito");
            delay(1500);
            BorrarPantalla();
        }
    }

    BorrarPantalla();
    CALIBRACION = false;
    MENU = true;
}

void Informacion()
{
    tft.setCursor(30, 40);
    tft.setTextColor(colorcuadro);
    tft.setTextSize(1);
    tft.print("Calibracion velocidad: ");
    tft.setTextColor(colortexto);
    tft.setCursor(180, 40);
    tft.print(Velocidadmaxcalib);
    tft.setCursor(180, 50);
    tft.print(Velocidadrepcalib);
    tft.setCursor(180, 60);
    tft.print(Velocidadmincalib);

    tft.setCursor(30, 80);
    tft.setTextColor(colorcuadro);
    tft.print("Calibracion giro: ");
    tft.setTextColor(colortexto);
    tft.setCursor(180, 80);
    tft.print(Giromaxcalib);
    tft.setCursor(180, 90);
    tft.print(Girorepcalib);
    tft.setCursor(180, 100);
    tft.print(Giromincalib);

    tft.setCursor(30, 120);
    tft.setTextColor(colorcuadro);
    tft.print("Version: ");
    tft.setTextColor(colortexto);
    tft.print("1.1");

    tft.setCursor(30, 130);
    tft.setTextColor(colorcuadro);
    tft.print("Hecho por Andy Saull  -  ");
    tft.setTextColor(colortexto);
    tft.print("Para IES La Paloma");

    tft.drawRoundRect(120, 150, 80, 49, 5, colorcuadro);
    tft.setTextColor(colortexto);
    tft.setCursor(135, 160);
    tft.print("Reiniciar");
    tft.setCursor(145, 180);
    tft.print("mando");
    if((X>120 && X<200) && (Y>150 && Y<199) && (Z>MINPRESION && Z<MAXPRESION))
        {
            Reinicio();
        }
    BotonMenu();
}

void ModoSegurof()
{
    if (Velocidadenvio2UP == VelocidadenvioUP)
        {
            Velocidadenviofade = 0;
        }
    else if (Velocidadenvio2UP > VelocidadenvioUP)
        {
            if (Velocidadenvio2UP-VelocidadenvioUP >= 100)
                {
                    Velocidadenviofade =  30 + ModoSeguroAjuste;
                }
            else if (Velocidadenvio2UP-VelocidadenvioUP >= 30)
                {
                    Velocidadenviofade = 10 + ModoSeguroAjuste;
                }
            else if (Velocidadenvio2UP-VelocidadenvioUP >= 10)
                {
                    Velocidadenviofade = ModoSeguroAjuste;
                }
            else if (Velocidadenvio2UP-VelocidadenvioUP >= 3)
                {
                    VelocidadenvioUP = Velocidadenvio2UP;
                    Velocidadenviofade = 0;
                }
        }
    else if (Velocidadenvio2UP < VelocidadenvioUP)
    {
        VelocidadenvioUP = Velocidadenvio2UP;
        Velocidadenviofade = 0;
    }
    VelocidadenvioUP = VelocidadenvioUP + Velocidadenviofade;

    if (VelocidadenvioUP >= 255)
        VelocidadenvioUP = 255;
    else if (VelocidadenvioUP <= 0)
        VelocidadenvioUP = 0;


    if (Velocidadenvio2DO == VelocidadenvioDO)
        {
            Velocidadenviofade = 0;
        }
    else if (Velocidadenvio2DO > VelocidadenvioDO)
        {
            if (Velocidadenvio2DO-VelocidadenvioDO >= 100)
                {
                    Velocidadenviofade =  30 + ModoSeguroAjuste;
                }
            else if (Velocidadenvio2DO-VelocidadenvioDO >= 30)
                {
                    Velocidadenviofade = 10 + ModoSeguroAjuste;
                }
            else if (Velocidadenvio2DO-VelocidadenvioDO >= 10)
                {
                    Velocidadenviofade = ModoSeguroAjuste;
                }
            else if (Velocidadenvio2DO-VelocidadenvioDO >= 3)
                {
                    VelocidadenvioDO = Velocidadenvio2DO;
                    Velocidadenviofade = 0;
                }
        }
    else if (Velocidadenvio2DO < VelocidadenvioDO)
    {
        VelocidadenvioDO = Velocidadenvio2DO;
        Velocidadenviofade = 0;
    }
    VelocidadenvioDO = VelocidadenvioDO + Velocidadenviofade;

    if (VelocidadenvioDO >= 255)
        VelocidadenvioDO = 255;
    else if (VelocidadenvioDO <= 0)
        VelocidadenvioDO = 0;

/*
    if (Giroenvio >= 255)
        Giroenvio = 255;
    else if (Giroenvio <= 0)
        Giroenvio = 0;

    if (Giroenvio == PotGiro)
    {
        Giroenviofade = 0;
    }

    if (PotVelocidad >= 0 && PotVelocidad <= 64)
    {
        Giroenvio = PotGiro;
    }
    else if (PotVelocidad >= 65 && PotVelocidad <= 128)
    {
        Giroenviofade = 30 + ModoSeguroAjuste;
    }
    else if (PotVelocidad >= 129 && PotVelocidad <= 192)
    {
        Giroenviofade = 10 + ModoSeguroAjuste;
    }
    else if (PotVelocidad >= 192 && PotVelocidad <= 255)
    {
        Giroenviofade = ModoSeguroAjuste;
    }
    Giroenvio = Giroenvio + Giroenviofade;
    */
}

void BotonMenu()
{
    tft.drawRect(0, 200, 320, 40, MORADO);
    tft.setTextColor(colortexto);
    tft.setTextSize(1);
    tft.setCursor(130, 205);
    tft.print("Volver al");
    tft.setCursor(145, 225);
    tft.print("Menu");
    if((X>0 && X<320) && (Y>200 && Y<240) && (Z>MINPRESION && Z<MAXPRESION))
    {
        ESTADO = false;
        AJUSTESCOCHE = false;
        AJUSTESMANDO = false;
        INFORMACION = false;
        MENU = true;
        BorrarPantalla();
    }
}

void ColoresDeAjuste()
{
    tft.drawLine(0, 159, 320, 159, NEGRO);

    tft.fillRect(0, 160, 64, 40, NEGRO);
    if((X>0 && X<64) && (Y>160 && Y<200) && (Z>MINPRESION && Z<MAXPRESION))
    {
        EEPROM.write(DireccionMemoria, 0);
        EEPROM.write(DireccionMemoria+1, 2);
        EEPROM.write(DireccionMemoria+2, 3);
        EEPROM.write(DireccionMemoria+3, 7);
        Reinicio();
    }
    tft.fillRect(64, 160, 64, 40, ROJO);
    if((X>64 && X<128) && (Y>160 && Y<200) && (Z>MINPRESION && Z<MAXPRESION))
    {
        EEPROM.write(DireccionMemoria, 12);
        EEPROM.write(DireccionMemoria+1, 7);
        EEPROM.write(DireccionMemoria+2, 3);
        EEPROM.write(DireccionMemoria+3, 0);
        Reinicio();
    }
    tft.fillRect(128, 160, 64, 40, AMARILLO);
    if((X>128 && X<192) && (Y>160 && Y<200) && (Z>MINPRESION && Z<MAXPRESION))
    {
        EEPROM.write(DireccionMemoria, 6);
        EEPROM.write(DireccionMemoria+1, 10);
        EEPROM.write(DireccionMemoria+2, 9);
        EEPROM.write(DireccionMemoria+3, 0);
        Reinicio();
    }
    tft.fillRect(192, 160, 64, 40, AZULPLAN);
    if((X>192 && X<256) && (Y>160 && Y<200) && (Z>MINPRESION && Z<MAXPRESION))
    {
        EEPROM.write(DireccionMemoria, 10);
        EEPROM.write(DireccionMemoria+1, 7);
        EEPROM.write(DireccionMemoria+2, 8);
        EEPROM.write(DireccionMemoria+3, 0);
        Reinicio();
    }
    tft.fillRect(256, 160, 64, 40, BLANCO);
    if((X>256 && X<320) && (Y>160 && Y<200) && (Z>MINPRESION && Z<MAXPRESION))
    {
        EEPROM.write(DireccionMemoria, 7);
        EEPROM.write(DireccionMemoria+1, 2);
        EEPROM.write(DireccionMemoria+2, 10);
        EEPROM.write(DireccionMemoria+3, 0);
        Reinicio();
    }
}

void LecturaColores()
{
  switch (colorfondo)
  {
    case 0:
        colorfondo = NEGRO;
        break;
    case 6:
        colorfondo = AMARILLO;
        break;
    case 7:
        colorfondo = BLANCO;
        break;
    case 10:
        colorfondo = AZULPLAN;
        break;
    case 12:
        colorfondo = ROJOSCURO;
        break;
  }

  switch(colortexto)
  {
    case 2:
        colortexto = ROJO;
        break;
    case 7:
        colortexto = BLANCO;
        break;
    case 10:
        colortexto = AZULPLAN;
        break;
  }

  switch (colortextoinfo)
  {
    case 3:
        colortextoinfo = VERDE;
        break;
    case 8:
        colortextoinfo = NARANJA;
        break;
    case 9:
        colortextoinfo = MORADO;
        break;
    case 10:
        colortextoinfo = AZULPLAN;
        break;
  }

  switch (colorcuadro)
  {
    case 0:
        colorcuadro = NEGRO;
        break;
    case 7:
        colorcuadro = BLANCO;
        break;
  }
}

void BarraSuperior()
{
    tft.drawRect(0, 0, 320, 12, colortexto); //Dibuja el primer recuadro rojo de arriba de la pantalla
    tft.setCursor(134, 3);
    tft.setTextColor(colorcuadro);
    tft.setTextSize(1);
    tft.print("Andy Saull"); //Texto central

    tft.setCursor(220, 3);
    tft.setTextColor(colortextoinfo);
    tft.print("-Lamborghini RF-"); //Texto de la derecha
}

void BorrarPantalla()
{
    tft.fillRect(0, 15, 320, 225, colorfondo);
}

void Reinicio()
{
    asm volatile ("  jmp 0");
}
