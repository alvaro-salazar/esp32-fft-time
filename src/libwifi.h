/*
 * The MIT License
 *
 * Copyright 2024 Alvaro Salazar <alvaro@denkitronik.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LIBWIFI_H
#define LIBWIFI_H

extern const char* ssid;            //< Cambia por el nombre de tu red WiFi
extern const char* password;        //< Cambia por la contraseña de tu red WiFi
void listWiFiNetworks();            //< Función para listar las redes WiFi disponibles
void startWiFi(const char* hostname);    //< Función para iniciar el servicio de WiFi
void checkWiFi();                   //< Función para verificar la conexión a la red WiFi
String getMacAddress();                //< Función para obtener la dirección MAC del dispositivo
void setMDNS(const char* hostname);    //< Función para iniciar el servicio de mDNS
#endif /* LIBWIFI_H */