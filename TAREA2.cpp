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

//Creamos la clase con todos los datos que se piden
class Respuesta{
    public: 
        string nombre, estado;
        int trans, rec, perd;
        
};

//Cuenta la cantidad de IP que existen en el txt
int contarIP(char *nombre_archivo);
//Ejecuta el comando PING, guarda los valores en las variables del objeto y luego los imprime
void ejecutarPING(Respuesta ip, string paquetes);
//Retorna la posición de un caracter en una cadena
int retornarPosicionCaracter(string cadena, string caracter);


int main (int argc, char *argv[]){

    //Valida los parámetros ejecutados al iniciar el comando
    if (argv[1] == NULL || argv[2] == NULL){

        if(argv[2] == NULL){
            printf("ERROR: No ingreso la cantidad de paquetes a enviar\n");
        }
        else {
            printf("ERROR: No ingreso el nombre del archivo con las IP\n");
        }
        return -1;
    }


    /*
        Contamos la cantidad de IP que existen en el archivo 
        (Para mayor informacion revisar la funcion)
    */
    int cant_ips =contarIP(argv[1]);
    if (cant_ips == -1 ) return -1;
    
    Respuesta ip[cant_ips];
    int i;
    ifstream archivo;
    archivo.open(argv[1],ios::in); 

    //Guardamos el valor de la IP 
    for(i=0; i<cant_ips; i++){

		getline(archivo, ip[i].nombre);
	}

    //Imprimimos los titulos de la tabla

    cout<< "IP                . Trans .  Rec  . Perd  . Estado\n";
    cout<< "---------------------------------------------------\n";
    
    //Creación del mutex
    pthread_mutex_init(&mutex, NULL);
    //Creación de todos los hilos 
    thread threads[cant_ips];
    for (i=0; i < cant_ips; i++) {
        threads[i] = thread(ejecutarPING , ip[i], argv[2]);
    }

    // Esperamos por el término de todos los hilos 
    for (i=0; i< cant_ips; i++) {
        threads[i].join();
    }
    
    pthread_mutex_destroy(&mutex);
    return 0;

}

//esta función cuenta la cantidad de ip en el archivo
int contarIP(char *nombre_archivo){

    //Abrimos el archivo con las IPs
    ifstream archivo;
    archivo.open(nombre_archivo,ios::in);    

    //Verificamos si existe
    if (archivo.fail()){
        printf("ERROR: Archivo no encontrado\n");
        return -1;
    }

    //Contamos cantidad de IPs
    int count=0;
    string temp;
    //esta función cuenta la cantidad de IP +1
    while(!archivo.eof()){	
		getline(archivo, temp);
        count++;
	}
    //Cerramos el achivo
    archivo.close();
    //Restamos uno al valor para que retorne la cantidad de IP precisas
    count --;
    return count;
} 


void ejecutarPING(Respuesta ip, string paquetes) {
    
    /*
        -Creamos un string que guarde el comando PING con 
        los valores obtenidos como parametros y luego ejecutamos
        -El comando generará un archivo txt con el nombre de la IP
    */
    string comando;
    string nombre_txt = ip.nombre + ".txt";
    comando = "ping " + ip.nombre + " -q -c"+ paquetes +" > " + nombre_txt;
    system(comando.c_str());


    //Abrimos el txt del IP individual
    ifstream txtIP;
    txtIP.open(nombre_txt,ios::in);  
    
    /*
        -A fin de llegar al apartado con el resumen del comando PING,
        buscamos la palabra '---' en el archivo, ejecutamos un par de
        getlines y obtenemos la linea con los valores buscados
    */
    string cadena, flag="---";
    do{
        txtIP >> cadena;
    } while (!txtIP.eof() && cadena!=flag);
    getline(txtIP, cadena);
    //guardamos en cadena, la linea con el resumen del comando 
    getline(txtIP, cadena);

    //Cerramos el archivo ya que no lo usaremos y posteriormente lo eliminamos
    txtIP.close();

    //Generamos el comando que lo eliminará y lo ejecutamos
    comando = "rm " + nombre_txt;
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
    ip.rec= atoi(cadena.substr(0, i).c_str());

    //Acortamos el string para que empiece despues de la segunda coma
    i=retornarPosicionCaracter(cadena, ",");
    cadena= cadena.substr(i+1, largoString-i);

    //Buscamos el numero antes de "% packet loss" del comando ping
    i = retornarPosicionCaracter(cadena, "%");

    //Calculamos a cuantos paquetes corresponde dicho porcentaje y lo guardamos
    ip.perd = atoi(cadena.substr(1, i-1).c_str());
    ip.perd = (ip.perd * ip.trans)/100;

    //Calculamos el valor de estado
    if (ip.rec>0){
        ip.estado= "UP";
    }else{
        ip.estado= "DOWN";
    }
    
    //Para mejorar la impresion, se le asigna un espacio definido
    string espacio=" ";
    if(ip.nombre.length()>=20){
        espacio=" ";
    } else{
        for(i=0;i<(20-ip.nombre.length()); i++){
            espacio=espacio+" ";
        }
    }
    //A traves de un Mutex imprimimos todos los valores resultantes
    pthread_mutex_lock(&mutex);
    cout << ip.nombre << espacio << ip.trans << "       " << ip.rec << "       " << ip.perd << "       "<< ip.estado << "\n";
    pthread_mutex_unlock(&mutex);
    

}

//Retorna la posición del caracter en un string
int retornarPosicionCaracter(string cadena, string caracter){
    int i, flag=0;
    for (i=0; flag==0 ; i++){
        if(cadena[i] == caracter[0]){
            //Si encuentra el caracter, cambia el valor del flag y sale del ciclo
            flag--;
            i=i-1;
        }
    }
    return i;
}
