/* Affectation des pins 
 *  MINI / UNO / NANO
 * pin 0 = RX
 * pin 1 = TX
 * pin 2 = CANInt
 * pin 3 = Led 0
 * pin 4 = Led 1
 * pin 5 = Led 2
 * pin 6 = Led 3
 * pin 7 = Led 4
 * pin 8 = Led 5
 * pin 9 = Led 6
 * pin 10 = CANCS
 * pin 11 = MOSI
 * pin 12 = MISO
 * pin 13 = SCK
 * pin A0 = Led 7
 * pin A1 = Led 8
 * pin A2 = Aiguille 0
 * pin A3 = Balise 0
 * pin A4 = Balise 1
 * pin A5 = Zone 0
 * pin A6 (nu)
 * pin A7 (nu)
 */ 
 
#if !defined ARDUINO_AVR_NANO && !defined ARDUINO_AVR_UNO
#if !defined VISUALSTUDIO
  #error CANNOT COMPILE - ONLY WORKS WITH AN ARDUINO UNO, NANO
#endif
#endif

#include <Arduino.h>
#include <EEPROM.h>

#include "Objet.h"

#include "Aiguille.h"
#include "Balise.h"
#include "Led.h"
#include "Zone.h"

#include "Bus.h"

#define LONGUEUR_ETAT	3

#define NB_LEDS			9
#define NB_AIGUILLES	1
#define NB_ZONES		1
#define NB_BALISES		2

const uint8_t leds_pins[] = { 3, 4, 5, 6, 7, 8, 9, A0, A1 };
const uint8_t aiguilles_pins[] = { A2 };
const uint8_t zones_pins[] = { A5 };
const uint8_t balises_pins[] = { A3, A4 };

const char *EEPROM_ID = { "LCDO" };

class Satellite
{
	// Identifiant du satellite
	uint8_t		id;

	// Liste de tous les objets g�r�s
	Objet*		objets[NB_LEDS + NB_AIGUILLES + NB_ZONES + NB_BALISES];
	uint8_t		nbObjets;

	void AddObjet(Objet *inpObjet, uint8_t inPin)
	{
		if (inpObjet != NULL && inPin != 255)
		{
			inpObjet->begin(inPin);
			this->objets[this->nbObjets++] = inpObjet;
		}
	}

	void EEPROM_sauvegarde()
	{
		int addr = 0;

		////////////////////////////////////// Partie ent�te

		/* Un descripteur du contenu est sauv� en premier.
		Si ce descripteur n'est pas conforme � la lecture, alors l'EEPROM sera consid�r�e vierge.
		Ca signifie aussi que changer la forme (nombre d'objets, taille d'un objet) annule toute sauvegarde !
		*/

    eeprom_write_block(EEPROM_ID, (void *)addr, 4);
		addr += 4;

		EEPROM.write(addr++, this->nbObjets);

		EEPROM.write(addr++, leds[0].GetEEPROMSize());
		EEPROM.write(addr++, aiguilles[0].GetEEPROMSize());
		EEPROM.write(addr++, balises[0].GetEEPROMSize());
		EEPROM.write(addr++, zones[0].GetEEPROMSize());

		////////////////////////////////////// Partie satellite

		EEPROM.write(addr++, this->id);

		////////////////////////////////////// Partie objets

		for (int i = 0; i < this->nbObjets; i++)
			addr = objets[i]->EEPROM_sauvegarde(addr);
	}

	/* Chargement de la configuration depuis l'EEPROM. La fonction retourne true si tout est bien charg�. */
	bool EEPROM_chargement()
	{
		int addr = 0;
		char buf[5];

		////////////////////////////////////// Partie ent�te

		// ID EEPROM
    eeprom_read_block(buf, (const void *)addr, 4);
		addr += 4;
		buf[4] = 0;

		for (int i = 0; i < 4; i++)
			if (buf[i] != EEPROM_ID[i])
				return false;

		// Nombre d'objets
		uint8_t nb = EEPROM.read(addr++);
		if (nb != this->nbObjets)
			return false;

		// taille des objets
    eeprom_read_block(buf, (const void *)addr, 4);
		addr += 4;

		if (buf[0] != leds[0].GetEEPROMSize())	return false;
		if (buf[1] != aiguilles[0].GetEEPROMSize())	return false;
		if (buf[2] != balises[0].GetEEPROMSize())	return false;
		if (buf[3] != zones[0].GetEEPROMSize())	return false;

		////////////////////////////////////// Partie satellite

		this->id = EEPROM.read(addr++);

		////////////////////////////////////// Partie objets

		for (int i = 0; i < this->nbObjets; i++)
			addr = objets[i]->EEPROM_chargement(addr);

		return true;
	}

public:

	uint8_t		etat[LONGUEUR_ETAT];

	Led			leds[NB_LEDS];
	Aiguille	aiguilles[NB_AIGUILLES];
	Zone		zones[NB_ZONES];
	Balise		balises[NB_BALISES];

	Satellite()
	{
		this->nbObjets = 0;
		for (int i = 0; i < NB_LEDS + NB_AIGUILLES + NB_ZONES + NB_BALISES; i++)
			objets[i] = NULL;
	}

	void begin()
	{
		this->nbObjets = 0;
		for (int i = 0; i < NB_LEDS; i++)		this->AddObjet(&this->leds[i], leds_pins[i]);
		for (int i = 0; i < NB_AIGUILLES; i++)	this->AddObjet(&this->aiguilles[i], aiguilles_pins[i]);
		for (int i = 0; i < NB_BALISES; i++)	this->AddObjet(&this->balises[i], balises_pins[i]);
		for (int i = 0; i < NB_ZONES; i++)		this->AddObjet(&this->zones[i], zones_pins[i]);

		if (EEPROM_chargement() == false)
			EEPROM_sauvegarde();
	}

	void loop()
	{
		for (int i = 0; i < this->nbObjets; i++)
			objets[i]->loop();
	}
};