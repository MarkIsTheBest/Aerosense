# Aerosense
#### Statie Meteo Mobila Autonoma, pentru culegerea datelor meteorologice, oricand si oriune.

-----

> [!NOTE]
> Acest ``README.md`` este strict dedicat partii technice al acestui proiect, pentru detalii mai generale, te rog visualizeaza prezentarea powerpoint, sau daca ai mai mult timp de citit, poti visualiza chiar si documentatia in intregime!


-----

### Ierarhie pagina Github:
La ``Aerosense/Scripts/Drone.ino`` se gaseste codul Microcontroller-ului, acesta gestioneaza atat senzorii,
 cat si trimiterea lor pe webserver prin intermediului unei cartele SIM, iar simultan datele inregistrate sunt salvate
 pe o cartela MicroSD.

La ``Aerosense/Scripts/Webserver/`` se gasesc script-urile pentru Webserver, acestea includ:
- ``server.js`` (functionalitatea principala a server-ului prin intermediul Node.js)
- ``index.html`` (codul de baza a front-end-ului)
- ``style.css`` (stilistica front-end)

-----

### Software utilizat:
| Software Name | Download Link |
|---|---|
| `Arduino IDE` | [`https://www.arduino.cc/en/software/`](https://www.arduino.cc/en/software/) |
| `Visual Studio Code` | [`https://code.visualstudio.com/`](https://code.visualstudio.com/) |
| `Fusion360` | [`https://www.autodesk.com/products/fusion-360/personal`](https://www.autodesk.com/products/fusion-360/personal) |
| `Creality Print` | [`https://www.creality.com/pages/download-software`](https://www.creality.com/pages/download-software) |
| `KiCad` | [`https://www.kicad.org/download/windows/`](https://www.kicad.org/download/windows/) |
| `Gemini Pro AI` | [`https://gemini.google.com/`](https://gemini.google.com/) |
| `Canva` | [`https://www.canva.com/`](https://www.canva.com/) |

-----

### Echipament utilizat:
* Ciocan de lipit (380C) | WCD
* Ciocan de lipid (280C) | Dedeman 
* Pistol Termic (380C) | WCD 
* Cositor SWCU 1/17 227C (fara plumb) | Dedeman
* Sursa programabila 161,2W (5.2A) | WAPTEK
* Imprimanta 3D Ender 3 V3 KE | Creality
* \+ Alte scule si unelte gasite acasa

-----

### Componentele electronice de baza:
 1. #### Circuit Principal:
    * Microprocessor | Arduino Nano (USB C)
    - Senzor temperatura, presiune si umiditate | BME280
    - Busola/Magnetometru | QMC5883L
    - Senzor analogic de gaz si fum | MQ-7
    - Senzor analogic de oxid de carbon (CO) | MQ-5
    - Modul accelerometru + airoscop | MPU6050
    - Modul cartela SIM GSM + GPRS (+ Antena + SIM orange prepay) | SIM800LV2 EVB 
    - Senzor calitatea aerului | SGP30
    - Modul GPS (+ Antena) | NEO6MV3
    - Modul MicroSD Card (+ MicroSDHD Card 32GB)

2. #### Circuit Alimentare:
   - Acumulator Li-Ion 3.7V (MAX 4.2) 3350mAh | Samsung INR18650-25E
   - Panou Solar 5V 1W | YX-107X61
   - Modul Incarcare solara acumulator 3.7V | MPPT
   - Voltmetru (incarcatura acumulator)
   - Intrerupator (opriria circuitului)
   - Ridicator de tensiune (3.7V -> 5V) 5A | XL6019


## Galerie Foto:

### Placa de baza a sistemului:
<img width="1008" height="756" alt="image" src="https://github.com/user-attachments/assets/4b457898-f970-4cd8-9a83-864a2e5c1666" />

### Sistemul de alimentare a placii:
<img width="756" height="1008" alt="image" src="https://github.com/user-attachments/assets/aeeb0074-df9d-4c69-9ec2-5033c10169de" />

### Schematica completa a sistemului din KiCad:
<img width="2339" height="1654" alt="image" src="https://github.com/user-attachments/assets/c6c07642-b449-4d43-a4ff-e8271ed79cd2" />


