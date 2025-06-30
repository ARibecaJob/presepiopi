# nativitypi / presepiopi
A light control system for nativity scenes using Raspberry Pi and Waveshare 8-ch Relay Expansion Board

*Sistema di controllo luci per presepe tramite Raspberry Pi e Waveshare 8-ch Relay Expansion Board*

## Description / Descrizione
**presepiopi** is an application developed to manage the progressive activation of lights in a nativity scene using 8 relays controlled by a Raspberry Pi.
The system behavior is defined by a configuration file named `presepiopi.txt`  
While the program is running, a file named `audio.mp3` will be played to accompany the sequence.  
It uses the [pigpio](https://abyz.me.uk/rpi/pigpio/) library to interface with the hardware and the included [miniaudio](https://miniaud.io/) library to play the audio file

**presepiopi** è un'applicazione sviluppata per gestire l'accensione progressiva di luci di un presepe attraverso 8 relè, comandati da un Raspberry Pi.  
Il comportamento del sistema è definito da un file di configurazione denominato `presepiopi.txt`.  
Durante l'esecuzione del programma, un file chiamato `audio.mp3` verrà riprodotto per accompagnare la sequenza.  
Utilizza la libreria [pigpio](https://abyz.me.uk/rpi/pigpio/) per l'interfacciamento hardware e la libreria [miniaudio](https://miniaud.io/) (inclusa) per la riproduzione del file audio.

## Configuration File Format / Formato del file di configurazione
The file `presepiopi.txt` must contain a series of lines in the following format:  
*Il file `presepiopi.txt` deve contenere una serie di righe nel seguente formato:*
```
ss,x,x,x,x,x,x,x,x
```

- `ss`: number of seconds to wait before executing the next action  
*`ss`: numero di secondi da attendere prima di eseguire l’azione successiva*
- `x`: state of each relay (total of 8), where:  
*`x`: stato del singolo relè (8 in totale), dove:*
  - `1` means the relay should be activated (on)  
  *`1` indica che il relè deve essere **attivato** (acceso)*
  - `0` means the relay should be **deactivated** (off)  
  *`0` indica che il relè deve essere **disattivato** (spento)*

### Example Configuration / Esempio di configurazione
```
5,1,0,0,0,0,0,0,0
4,1,1,0,0,0,0,0,0
3,1,1,1,0,0,0,0,0
```

### Interpretation / Interpretazione
- After **5 seconds**, the first relay is activated  
Dopo **5 secondi**, si attiva il **primo relè**
- After another **4 seconds** the **second relay** is activated keeping the first one on  
Dopo altri **4 secondi** si attiva anche il **secondo relè** mantenendo il primo acceso  
- After an additional **3 seconds**, the third relay is also activated, keeping the previous ones on  
Dopo ulteriori **3 secondi**, si attiva anche il **terzo relè**, lasciando accesi i precedenti  

At the end of the sequence, the relays *remain in their final state* with no automatic reset  
Al termine della sequenza, i relè **rimangono nello stato impostato** senza alcun reset automatico

## Hardware Interface / Interfaccia hardware
- Status **led** connected to **GPIO 24**  
***led** di stato collegato al **GPIO 24***
- Control **button** connected to **GPIO 17**  
***Pulsante** di controllo collegato al **GPIO 17***

## System Behavior / Comportamento del sistema
- On program **start**:  
All’**avvio** del programma:
  - The **led** is turned on to indicate a waiting state  
  Il **led** è acceso per indicare lo stato di attesa
 - On **button press and release**, the program:  
 *Alla **pressione e rilascio del pulsante**, il programma:*
   - Starts the relay sequence and plays the audio file
   *Avvia la sequenza dei relè ed esegue il file audio*
   - Turns off the status **led**  
   *Spegne il **led** di stato*

## During Execution / Durante l'esecuzione
- If **the button is pressed and released in under 1.5 seconds**:  
*Se il **pulsante viene premuto e rilasciato per meno di 1.5 secondi**:*
  - The program enters **pause mode**  
  *Il programma entra in **modalità pausa***
  - The **led** on **GPIO 24 blinks**  
  *Il **led** su **GPIO 24 lampeggia***
- If the **button is held for more than 1.5 seconds and then released:**  
*Se il **pulsante viene tenuto premuto e rilasciato per più di 1.5 secondi:***
  - The **led** briefly blinks and the system returns to the initial state  
*il **led** lampeggia brevemente e si torna allo stato iniziale*
  
## To enable automatic startup / Per attivarlo in esecuzione automatica
Edit the systemd service file:  
*Modificare il file di servizio systemd:*

```
sudo nano /etc/systemd/system/presepiopi.service
```

add / inserire


```
[Unit]
Description=PresepioPi Controller
After=multi-user.target network-online.target sound.target

[Service]
Type=simple
User=root
WorkingDirectory=/boot/firmware/presepiopi/
ExecStart=/boot/firmware/presepiopi/presepiopi
Restart=always
RestartSec=5
Nice=-10

[Install]
WantedBy=multi-user.target
```

## To enable the service / Per attivare il servizio
```
sudo systemctl daemon-reload
sudo systemctl enable presepiopi.service
sudo systemctl start presepiopi.service
```

## To check the service status / Per controllare lo stato del servizio
```
sudo systemctl status presepiopi.service
# In case of errors / in caso di errori
journalctl -u presepiopi.service -b
```

## To disable the service / Per disabilitare il servizio
```
sudo systemctl disable presepiopi.service
```

## Extra
The `support` folder contains two sources: one for testing the button functionality and one for generating an audio file.  
*La cartella `support` contiene due sorgenti: uno per testare il funzionamento del pulsante e uno per generare un file audio.*