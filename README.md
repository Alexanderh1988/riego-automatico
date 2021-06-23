# riego automatico
 codigo para sistemas con riego automatico y control mediante WIFI
 
 C++
 
 Para chips: ESP8266, programado con compilador de arduino.
 Sensor: DHT11 instalar un transistor entre tierra y V++ (puede ser un S8055) para evitar corrosion galvanica, darle 0.1 seg de lectura es suficiente. 
 Bombita: se conecta motor en D2 mediante un transistor tip122 (no olvidar instalar diodo a V++ para devolver la energia sobrante del campo).
 Arranque suave --->      for (int motorValue = 0 ; motorValue <= 255; motorValue += 5).       
  Contiene watchdog para reiniciar el chip si es que se congela.

Interface de control propia mediante bootstrap, html, php, ajax (no disponible a publico :/ )
  
Opcion se puede tener una buena dashboard mediante Freeboard:

<img src="https://github.com/Alexanderh1988/riego-automatico/blob/main/Freeboard%20dashboard.jpg?raw=true">





