# Mambo kit

Conjunto de herramientas para preparar y ejecutar **pruebas**
de estres, estilo DNS Amplification DDOS. Por favor no utilizar
esta herramienta para atacar sitios web estatales o comerciales,
en especial a las gestoras culturales como CAPIF, SADAIC, etc.

## Instalar

**Dependencias**

Para crear los listados de servidores DNS vas a tener que instalar
[Zmap](http://zmap.io/) con sus dependencias.

**Compila Mambo**

    $ gcc -o mambo mambo.c

_Tira unos warnings, no pasa nada_

## Correr

**Crear listados de Servidores DNS**

El script `zm.sh` utiliza rangos de IP para escanear en busca de 
Servidores DNS abiertos. _Editalo para cambiar el archivo de donde 
sacar los rangos y donde guardar los resultados._

    # ./zm.sh

_Tarda unas horas_

Notas:

`blist.conf` tiene el rango que no se va a escanear.

`wlist_xx` son los rangos de IPs para cada pais, donde `xx` es el país (ar, br, cl, etc).

**Limpiar listados**

Chequea que servidores estan respondiendo.

    php alive.php listado

_Guarda los resultados en `listado.alive`_

**Correr**

    # ./mambo target isc.org listado

Errores:

`Segmentation fault` Probablemente algun Servidor DNS este caido, pasale `alive.php` al listado.
