/*
    . /TAREA2 archivo.txt [NUMERO]
*/

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>

pthread_mutex_t mutex;

using namespace std;

class Respuesta{
    public: 
        string nombre, rec, estado;
        int trans, perd;
        
};


int contarIP(char *nombre_archivo);
void ejecutarPING(Respuesta ip, string paquetes);
int retornarPosicionCaracter(string cadena, string caracter);

// argc cantidad de parámetros y argv los vectores de parámetros
int main (int argc, char *argv[]){

    //Valida los parámetros ejecutados al iniciar el comando

    if (argv[1] == NULL || argv[2] == NULL){

        if(argv[1] == NULL) printf("ERROR NO INGRESO EL NOMBRE DEL ARCHIVO\n");
        else printf("ERROR NO INGRESO UN NUMERO\n");
        return -1;
    }

    ifstream archivo;
    archivo.open(argv[1],ios::in);   
    int cant_ips =contarIP(argv[1]);
    if (cant_ips == -1 ) return -1;
    
    Respuesta ip[cant_ips];
    int i;
    
    for(i=0; i<cant_ips; i++){

		getline(archivo, ip[i].nombre);
	}
    
    //Creación del mutex
    pthread_mutex_init(&mutex, NULL);
    //Creación de todos los hilos 
    thread threads[cant_ips];
    for (i=0; i < cant_ips; i++) {
        threads[i] = thread(ejecutarPING , ip[i], argv[2]);
    }

    /* Para esperar por el término de todos los hilos */
    for (i=0; i< cant_ips; i++) {
        threads[i].join();
    }
    pthread_mutex_destroy(&mutex);
    return 0;

}


int contarIP(char *nombre_archivo){

    //Abrimos el archivo con las ips
    ifstream archivo;
    archivo.open(nombre_archivo,ios::in);    

    //Verificamos si existe
    if (archivo.fail()){
        printf("ERROR ARCHIVO NO ENCONTRADO\n");
        return -1;
    }

    //Contamos cantidad de ips
    int count=0;
    string temp;
    //esta función cuenta la cantidad de ip +1
    while(!archivo.eof()){	
		getline(archivo, temp);
        count++;
        //printf("ip %d\n", count);
	}
    archivo.close();
    count --;
    return count;
} 


void ejecutarPING(Respuesta ip, string paquetes) {
    string comando;
    string nombre_txt = ip.nombre + ".txt";

    comando = "ping " + ip.nombre + " -q -c"+ paquetes +" > " + nombre_txt;
    //cout << "IP= " + ip + " \ncomando: " + comando + "\n";
    system(comando.c_str());
    //cout << "FINISH " + ip + " \n";

    //Abrimos el txt del IP individual
    ifstream txtIP;
    txtIP.open(nombre_txt,ios::in);  
    
    //buscamos la palabra --- en el archivo
    string cadena, flag="---";
    do{
        txtIP >> cadena;
    } while (!txtIP.eof() && cadena!=flag);
    //leemos esa linea temporalmente
    getline(txtIP, cadena);
    //guardamos en cadena, la linea con el resumen del comando ping
    getline(txtIP, cadena);

    //cout << "IP = "+ ip +" STRING " + cadena + " \n";
    txtIP.close();

    //eliminamos el archivo txt que creo esta hebra
    comando = "rm " + nombre_txt;
    //cout << "IP= " + ip.nombre + " comando: " + comando + "\n";
    system(comando.c_str());

    //calculamos el largo del string
    int largoString = cadena.length();

    //Buscamos el numero antes de "packets transmitted" del comando ping
    int i = retornarPosicionCaracter(cadena, "p");
    //Guardamos el valor 
    ip.trans=  atoi(cadena.substr(0, i-1).c_str());

    //Acortamos el string para que empiece despues de la primera coma
    i=retornarPosicionCaracter(cadena, ",");
    cadena= cadena.substr(i+1, largoString-i);
    //Actualizamos el largo del string
    largoString = cadena.length();
    //Buscamos el numero antes de "received" del comando ping
    i = retornarPosicionCaracter(cadena, "r");
    //Guardamos el valor 
    ip.rec= cadena.substr(0, i);

    //Acortamos el string para que empiece despues de la segunda coma
    i=retornarPosicionCaracter(cadena, ",");
    cadena= cadena.substr(i+1, largoString-i);

    //Buscamos el numero antes de "% packet loss" del comando ping
    i = retornarPosicionCaracter(cadena, "%");

    
    ip.perd = atoi(cadena.substr(1, i-1).c_str());
    ip.perd = (ip.perd * ip.trans)/100;

    pthread_mutex_lock(&mutex);
    cout << "IP= " << ip.nombre << " Trans: "<< ip.trans<< " rec: "<< ip.rec << " perd: "<< ip.perd << "\n";
    pthread_mutex_unlock(&mutex);
    

}

int retornarPosicionCaracter(string cadena, string caracter){
    int i, flag=0;
    for (i=0; flag==0 ; i++){
        if(cadena[i] == caracter[0]){
            flag--;
            i=i-1;
        }
    }
    return i;
}
