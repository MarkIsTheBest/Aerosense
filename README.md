<img width="712" height="94" alt="image" src="https://github.com/user-attachments/assets/bd13d183-a637-44bc-bebc-6ede57f56171" />

#### *Statie Meteo Mobila Autonoma, pentru culegerea datelor meteorologice, oricand si oriune.*

---

> [!NOTE]
> Acest ``README.md`` este strict dedicat partii technice al acestui proiect, pentru detalii mai generale, te rog visualizeaza prezentarea powerpoint, sau daca ai mai mult timp de citit, poti visualiza chiar si documentatia in intregime!

---

### Ierarhie pagina Github:
La ``Aerosense/Scripts/Drone.ino`` se gaseste codul Microcontroller-ului, acesta gestioneaza atat senzorii,
 cat si trimiterea lor pe webserver prin intermediului unei cartele SIM, iar simultan datele inregistrate sunt salvate
 pe o cartela MicroSD.

La ``Aerosense/Scripts/Webserver/`` se gasesc script-urile pentru Webserver, acestea includ:
- ``server.js`` (functionalitatea principala a server-ului prin intermediul Node.js)
- ``index.html`` (codul de baza a front-end-ului)
- ``style.css`` (stilistica front-end)

La ``Aerosense/3DModels/`` se gaseste modelul 3d al carcasei printate, care faciliteaza locuinta intregului sistem. Acest model se gaseste in mai multe format-uri pentru a usura uzul acestora. (Fusion360)

La ``Aerosense/Datasheets/`` se gasesc documente specifice a diverselor parti furnizate extern al sistemului nostru, cum ar fi de exemplu: baterie ce noi o folosim.

La ``Aerosense/Documentation/`` se gaseste atat documentatia cat si o prezentare powerpoint, pentru a usura intelegerea proiectului/sistemului, fiind scrisa la un mod mai general decat acest ``README.md`` care este strict technic:
- ``Aerosense Documentation.pdf`` (asa numita documentatie de 12 pagini)
- ``Aerosense Presentation.pptx`` (prezentare powerpoint generala)

La ``Aerosense/Photos/`` se gasesc diverse poze, capturand infatisarea sistemului, atat pe dinafara cat si pe dinauntru. Aceste poze va pot ajuta in vizualizaera sistemului.

La ``Aerosense/Schematics/`` se gaseste schematica circuitului electric facut in KiCad. Aceasta schematica nu face posibila doar visualizarea conexiuniilor electrice intre diferitii senzori, ci si a sistemului de alimentare. Urmarind aceasta schematica, proiectul poate fii facut mult mai mic prin intermediul, unui PCB presonalizat.

La ``Aerosense/Identity/`` se gaseste identitatea produsului, adica logo-ul si titlul acestuia, ambele pot fii gasite si pe carcasa sistemului.

-----

# Hardware:

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
  
### Echipament utilizat:
| Tools | Manufacturer |
| --- | --- |
| `Ciocan de lipit (380C)` | `WCD` |
| `Ciocan de lipid (280C)` | `Dedeman` | 
| `Pistol Termic (380C)` | `WCD` |
| `Cositor SWCU 1/17 227C (fara plumb)` | Dedeman |
| `Sursa programabila 161,2W (5.2A)` | `WAPTEK` |
| `Imprimanta 3D Ender 3 V3 KE` | `Creality` |
| `+ Alte scule si unelte gasite` | `Acasa` |

-----

# Software:

### Librarii Folosite:

 1. #### Arduino:
    * ``SoftwareSerial.h`` (pentru a putea folosii oricare pini ca si TX/RX)
    * ``QMC5883LCompass.h`` (pentru gestionarea usoara si eficienta a busolei)
    * ``TinyGPSPlus.h`` (pentru interpretarea datelor primite de la sateliti)
    * ``I2Cdev.h`` (pentru conectarea si usurinta uzual a protocolului I2C)
    * ``MPU6050.h`` (pentru calibrarea si utilizarea eficienta a IMU-ului)
    * ``Wire.h`` (pentru a putea folosi protocolul de transmitere I2C)
    * ``sSense-BMx280I2C.h`` (pentru a banaliza folosirea senzorului ambiental)
    * ``Adafruit_SGP30.h`` (pentru a banaliza folosirea senzorului de calitate a aerului)
      
 2. #### Webserver:
    * ``http`` (pentru a facilita crearea unui server http)
    * ``fs`` (pentru a gestiona fisiere cu Node.js)
    * ``path`` (pentru a gestiona adresele fisierelor)
    * ``mysql2 + promise`` (pentru a efectua conexiunea cu baza de date mySQL)
    * ``leaflet(API)`` (pentru a avea acces la harta lumii)
    * ``gemini(API)`` (pentru a explica datele complicate utilizatorilor "non-experti")

### Baza de Date: MySQL

Pentru a stoca în mod persistent datele primite de la dronă, sistemul utilizează o bază de date ``MySQL``. Serverul ``Node.js`` acționează ca o punte de legătură între microcontroler și baza de date.
Conectarea la Baza de Date: Scriptul ``server.js`` folosește librăria ``mysql2/promise`` pentru a se conecta la serverul ``MySQL``. Detaliile de conectare (host, utilizator, parolă, numele bazei de date) sunt definite în obiectul ``dbConfig``.

#### Primirea și Inserarea Datelor:
1. Când microcontrolerul trimite date printr-o cerere ``POST`` la adresa ``/data``, serverul ``Node.js`` primește aceste informații în format ``JSON``.
2. Serverul adaugă un timestamp exact la momentul recepției pentru a asigura acuratețea temporală.
3. Datele sunt apoi inserate în tabelul ``drone_data`` din baza de date esp32_data folosind o interogare ``SQL`` de tip ``INSERT``. Utilizarea interogărilor parametrizate (cu ?) previne atacurile de tip ``SQL`` injection.
   
#### Livrarea Datelor către Front-End:
1. Pagina web (``index.html``) solicită datele de la server prin intermediul unei cereri ``GET`` la adresa ``/api/data``.
2. Serverul răspunde acestei cereri prin executarea unei interogări ``SQL`` de tip SELECT * FROM drone_data, extrăgând toate înregistrările.
3. Datele extrase sunt trimise către front-end în format JSON, unde sunt apoi folosite pentru a popula graficele și harta ``Leaflet``.

### Software extern utilizat:
| Software Name | Download Link |
|---|---|
| `Arduino IDE` | [`https://www.arduino.cc/en/software/`](https://www.arduino.cc/en/software/) |
| `Visual Studio Code` | [`https://code.visualstudio.com/`](https://code.visualstudio.com/) |
| `Fusion360` | [`https://www.autodesk.com/products/fusion-360/personal`](https://www.autodesk.com/products/fusion-360/personal) |
| `Creality Print` | [`https://www.creality.com/pages/download-software`](https://www.creality.com/pages/download-software) |
| `KiCad` | [`https://www.kicad.org/download/windows/`](https://www.kicad.org/download/windows/) |
| `MySQL` | [`https://www.mysql.com`](https://www.mysql.com) |
| `Gemini Pro AI` | [`https://gemini.google.com/`](https://gemini.google.com/) |
| `Canva` | [`https://www.canva.com/`](https://www.canva.com/) |

-----

## Galerie Foto:

### Placa de baza a sistemului:
<img width="1008" height="756" alt="image" src="https://github.com/user-attachments/assets/4b457898-f970-4cd8-9a83-864a2e5c1666" />

### Sistemul de alimentare a placii:
<img width="756" height="1008" alt="image" src="https://github.com/user-attachments/assets/aeeb0074-df9d-4c69-9ec2-5033c10169de" />

### Schematica completa a sistemului din KiCad:
<img width="1008" height="756" alt="image" src="https://github.com/user-attachments/assets/cba38ad6-4d2b-4e26-9eb9-47fbb8100417" />

### Webserver-ul dronei:
<img width="1008" height="756" alt="image" src="https://github.com/user-attachments/assets/ce487f56-4fcd-414a-9d1b-c897a84200d6" />


