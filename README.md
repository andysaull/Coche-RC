# Coche-RC
Coche a radiocontrol con un mando con pantalla para poder ajustar distintos parámetros.

_Trabajo final para el Grado Superior de Mantenimiento Electrónico en IES Virgen de la Paloma_

---------------------------------------------------------

La realización de este proyecto, se divide en dos partes, el mando emisor con pantalla y por otra el coche receptor ambos con un Arduino UNO que controla todos los módulos. La comunicación entre ellos se hace sobre radiofrecuencia a 433MHz.

El mando es capaz de mover el coche mediante dos joysticks, como también es capaz de encender las luces e intercambiar entre un modo sport y un modo seguro con 3 ajustes. El mando puede intercambiar los temas, es decir, los colores de la pantalla entre una carta de 5 colores que se almacenan en la EEPROM.

El mando se alimenta mediante una batería de Ni-Cd de 700mAh@9,6V.

Además, los joysticks se pueden calibrar por si con el tiempo cambian su valor.

El coche es de un coche radiocontrol comercial al que se le ha sacado la electrónica y se le ha puesto un Arduino como núcleo. Lleva dos motores DC para acelerar y girar, luces delanteras blancas, luces traseras rojas y luces de motor rojas.

El coche se alimenta mediante una batería de Ni-Mh de 1800mAh@9,6V.

Tanto el mando como el coche llevan una placa PCB hecha a medida para soldar todos los componentes en ella.

**Ver la carpeta de Fotos para ver más sobre el proyecto**
