/*Coche RC
Este proyecto esta realizado por Andres Saumell.
Se trata de un coche de radiocontrol con un motor para las ruedas traseras y un motor para las delanteras*/
#include <VirtualWire.h>

//El entero que hemos recibido desde el mando se guardara aqui
int datos;

//El mensaje recibido se guardara aqui
char mensaje[5];

void setup()
{
  //Iniciamos la comunicacion por Serial
  Serial.begin(9600);

  //Iniciamos los pines
  pinMode(6, OUTPUT); //Motor trasero
  pinMode(4, OUTPUT); //Leds de luces delanteras del coche

  vw_set_ptt_inverted(true);
  vw_setup(2000);
  vw_rx_start();

}

void loop()
{
    //Establecemos el tamano maximo del buffer y iniciamos el buffer
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    if (vw_get_message(buf, &buflen)) //Guardamos el mensaje en el buffer
    {

      for (int i = 0; i < buflen; i++)
      {
          mensaje[i] = char(buf[i]); //Llenamos la matriz mensaje con el buffer
      }

        //Ponemos el final de linea al final, o si no la funcion atoi no funcionara
        //mensaje[buflen] = <code class="cpp string">'\0'</code>;

        //Convertimos el mensaje de array de caracteres a enteros:
        datos = atoi(mensaje);

        // Descomentar para ver el mensaje recibido por Serial
        Serial.println(datos);

        //Analizamos el mensaje, guardando la velocidad y el motor
        int vel1 = datos%1000;
        datos = datos/1000;
        int motor_activado = datos;

        //Segun el valor de motor_activado, movemos un motor u otro
        if(motor_activado == 1)
         analogWrite(6, vel1);
        else if(motor_activado == 2)
         analogWrite(9, vel1);

        if(motor_activado == 3 && vel1 == 000)
         digitalWrite(4, LOW);
        else if(motor_activado == 3 && vel1 == 001)
         digitalWrite(4, HIGH);


    }
}
