/* 
	Liezensierungen

		<a href="https://de.vecteezy.com/gratis-vektor/tropfen">Tropfen Vektoren von Vecteezy</a>	
*/
/*
	Bibliotheken:

		glfw 3.3.6		|		https://www.glfw.org/
		opengl32		|		Basic in Windows
		freetype		|		https://freetype.org/												-> Kann "erspart bleiben" wenn der Bereich für Freetype und Schrift ausgeklammert wird. Aktuell "Fehlerhafter Code"
		mysqlconn 8.0	|		https://dev.mysql.com/doc/connector-cpp/8.0/en/connector-cpp-introduction.html
*/



#include "Shader.h"
#include "lodepng.h"

#include "Interaktive_Karte.h"
#include "Verbindung.h"

using namespace std;

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_Service.hpp> 
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using boost::asio::ip::tcp;

/* Netzwerk System */
boost::asio::io_service ioservice;

#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

/*	
	WIKI
	Problem Stellung: 
			- Die String werden beim Dritten %s nicht Ordnungsgemäß an die Konsole übergeben. 
			Mögliche Idee, Prüfen der Umgebungsvariablen der Konsolen
			LÖSUNG: Achtung es scheint das System zu Überladen wenn 3 strings in Folge an die Konsole übergeben werden, anders Trennen der string Übertragung

			Zuweisen der Auswählbaren Missionen in eine Map legen, um diese im Auswahlfenster nur einmal zu haben.
			LÖSUNG: Mithilfe von DISTINCT wird in SQL bereits gefiltert.

			Erstellen der Karte in einem Bereich "außerhalb der Kamera" 
			Mapping eines 2ten Screen Bereichs aus das gezeichnete Viereck.
			Zeichnen der Markierungen mithilfe von Alpha Kanal und kleinen Vierecken wo die "Spitze" (Untere Mitte) auf dem Zielpunkt zeigt.
			Aktueller Weg: Zeichne die Plane als Semi "3d Objekt" - Deaktivere die Kamera Drehung - Begrenze die Bewegung auf die entsprechenden Kartenränder
			Erstellen der Hintergrund Karte: Done
			Die Grafik mit Alpha Kanal laden: Done
			Die 2D Karte als "semi 3D" erstellen: Done
			die bewegung auf den kartenbereich beschränken: Done
			Kleine Markierungssteine mit Tropfen erstellen: Done
			Bewegung der Kleinen Margierungsteine angleichen: Done | Es war eine Optische Täuschung der durch den z Versatz entstanden ist. (Verkürzung von z zur Plane, zum Reduzieren der Täuschung)
			Diese Markierungsteine Verteilen: Done
			Die markierungsteine mithilfe der datenbankpositionen laden: Done
			Die markierungsteine im fenster verschieben können: Semi Funktionierend - Bugfix: Korregiere die Selectionsauswahl auserhalb der Mitte des Fensters 
			neue markeirungsteine setzten im fenster: offen


			"Verbannen" der SQl Tabelle erstellen
			Nach der Auswahl der Mission die "Knoten" Tabelle mit IMGUI ausgeben
				Sollte man auf "den" Knoten klicken >> Springe mit der Karte zum Mittelpunkt
				Wähle Mithilfe der Anwahl den Knoten aus

				Einfügen sobald "eingelogt"
*/
	   


/* Basisklasse für den Namen - Für einen Vererbungszugriff - Trennt mögliche andere string  */
class NAME
{
public:
	string name;
	/* Zero Access - Spätere Terminierung "zum erzwingen einen Namen zu Übergeben" */
	NAME() {};
	NAME(string name) : name(name) {};
	/* Möglichkeit nach der Zuweisung weitere Zugriffe zu ermöglichen */
	NAME operator() (string name) { this->name = name; return *this; };
	string getName() { return name; };
};

/* Klasse zur Speicherung der Maus Steuerinformationen für einen Vereinfachten Zugriff */
class MAUS
{
public:
	bool Is_Links_Klickt, Is_Rechts_Klickt;
	int Action;
	double X, Y, Z;
	float NX, NY;
};

/* Stark Vereinfachte Kamera - Positionierung (C) mithilfe von vec3 - Speicher der View und projection Matrix */
class Simple_Cam
{
public:
	glm::vec3 C;
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
} Cam;

/* Klassen für die Klassenvererbung von Koordinaten und Name - Zusammengefaster/Polymorpher Zugriff zu den "Targets" */
class BASISATTRIBUTE : public Koordinaten, public NAME
{
public:
	int Index;
	string Zusatz;

	BASISATTRIBUTE() {};
	BASISATTRIBUTE(float x, float y, int Index, string name, string Zusatz) : Index(Index)
	{
		this->X = x;
		this->Y = y;
		this->name = name;
		this->Zusatz = Zusatz;
	}

	operator glm::vec2()
	{
		return glm::vec2(X, Y);
	}

	glm::vec2 getKoordinaten()
	{
		return  glm::vec2(X, Y);
	}
};

/* Klasse für die Zusammenfassung der Höhlendaten und verbinden mit der Basisanwahl */
class Höhlendaten : public BASISATTRIBUTE
{

public:
	string Grundriss;
	int ErzknotenAnzahl;
	Höhlendaten(string name, int X, int Y, string Grundriss, int Anzahl) : Grundriss(Grundriss), ErzknotenAnzahl(Anzahl)
	{
		Index = 0;
		this->name = name;
		Zusatz = this->Grundriss;
		this->X = X;
		this->Y = Y;
	};
	Höhlendaten(Koordinaten cords, string Grundriss, int Anzahl) : Grundriss(Grundriss), ErzknotenAnzahl(Anzahl)
	{
		Index = 0;
		this->name = name;
		Zusatz = this->Grundriss;
		this->X = cords.getX();
		this->Y = cords.getY();
	};

	operator Koordinaten()
	{
		return Koordinaten(this->getX(), this->getY());
	}
	operator glm::vec2()
	{
		return glm::vec2(X, Y);
	}
};

/* Klasse für die Missionspunkte - Empfohlen - Ladungspunkt die Belohnungen zu übergeben.  Klasse zur Verbindung mit der Basisanwahl*/
class Missionsdaten : public BASISATTRIBUTE
{

public:
	string Missionsbeschreibung;
	string Missionstyp;
	int Belohnung_Ren;

	Missionsdaten(float X, float Y, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren) :
		Missionsbeschreibung(Missionsbeschreibung), Missionstyp(Missionstyp), Belohnung_Ren(Ren)
	{
		Index = 1;
		Zusatz = this->Missionsbeschreibung;
		this->name = Missionsname;
		this->X = X;
		this->Y = Y;
	};
	Missionsdaten(Koordinaten cords, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren) :
		Missionsbeschreibung(Missionsbeschreibung), Missionstyp(Missionstyp), Belohnung_Ren(Ren)
	{
		Index = 1;
		Zusatz = this->Missionsbeschreibung;
		//*Zusatz = 
		this->name = Missionsname;
		this->X = cords.getX();
		this->Y = cords.getY();
	};

	operator Koordinaten()
	{
		return Koordinaten(this->getX(), this->getY());
	}
	operator glm::vec2()
	{
		return glm::vec2(X, Y);
	}

	glm::vec2 getKoordinaten()
	{
		return  glm::vec2(X, Y);
	}
	string getName()
	{
		return name;
	};
	string getTyp()
	{
		return Missionstyp;
	};
	string getBeschreibung()
	{
		return Missionsbeschreibung;
	};
	int getRen()
	{
		return Belohnung_Ren;
	}

	int *getInt()
	{
		return &Belohnung_Ren;
	}
};

/* Klasse für die Fensterdaten - Größe des Fensters sowie "anwahl" */
class window : public Koordinaten
{
public:
	MAUS Maus;

	glm::vec3 MAUS()
	{
		return glm::vec3(Maus.X, Maus.Y, Maus.Z);
	}
	window() : Koordinaten(800, 600) {};
	glm::vec4 viewport = { 0,0,X,Y };


	BASISATTRIBUTE *ID;	/* Variable für die Basisanwahl */
	unsigned int Arrayindex = 99999;

	void operator() (int X, int Y)
	{
		this->X = X;
		this->Y = Y;
		viewport = { 0,Y,X,-Y };
	}
}Fensterdaten;

typedef unsigned int uint;



//for demonstration only. never save your password in the code!
const std::string server = "tcp://127.0.0.1:3306";
const std::string username = "Hengar";
const std::string password = "Affenbrot12";

std::string Benutzername = "";
std::string Passwort = "";
int loginlvl = -1;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character
{
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};
map<GLchar, Character> Characters;
uint   Roter_Tropfen, Blauer_Tropfen, Gelber_Tropfen, Schwarzer_Tropfen, texture1, texture2;

vector<pair<string,string>> Missionsliste;	/* Datenspeicher für die Missionsauswahl */
vector<Höhlendaten> Höhlenentraces;			/* Datenspeicher für die "vorhandenen" Höhleneingänge */
vector<Missionsdaten> MISSIONSDATEN;		/* Datenspeicher für die "vorhandenen" Missionsdaten */
vector<BASISATTRIBUTE> EXOTICknoten;		/* Datenspeicher für die "vorhandenen" Exotic Spots */
vector<BASISATTRIBUTE> BOSSEknoten;			/* Datenspeicher für die "vorhandenen" Bosse */


class CHATSPEICHER
{
	vector<string> Chatspeicher;
public:


};


bool SQL_EINLESEN = false;				/* Bit zur Steuerung, ob beim Start das SQL eingelesen werden soll */
bool Interface_Hovered;					/* Bit zur Steuerung, das sobald dass Interface "angewählt" ist, nicht die Karte "bewegt" wird */
bool Statusanzeige = false;				/* Bit zur Steuerung, dass die Anzeige nur "einaml" pro klick stattfindet */
bool Updaten = false;
bool Löschen = false;
bool Mission_Caveentrace = false;		// Fakse = Inventory True = Caveentrace
bool Mission_Ausgewählt = false;
bool Reset = false;
bool Wechsel = false;

bool compare(std::string t1, char* t2)
{
	return t1 == string(t2);
}

string PasswortVerschlüsseln(string input)
{
	string ret;
	char key = 't';
	for(uint i = 0; i < input.size(); i ++)
		ret = input[i] ^ key;
	return ret;
}

/* Klasse für die Datenspeicherung innerhalb eines SQL Systems */
class PRIVAT_MYSQL
{
	sql::Driver *driver;
	sql::Connection *con;
	sql::PreparedStatement *pstmt;
	sql::PreparedStatement *Daten_Einfügen_Höhle;
	sql::PreparedStatement *Daten_Einfügen_Mission;

	sql::PreparedStatement *Alle_Daten_Lesen_Höhle;
	sql::PreparedStatement *Alle_Daten_Lesen_Mission;

	sql::PreparedStatement *Alle_Daten_Lesen_Mission_Einmalig;
	sql::PreparedStatement *Einzel_Daten_Lesen_Mission;

	sql::PreparedStatement *Benutzerlesen;
	sql::PreparedStatement *Benutzereinfügen;

	sql::Statement *stmt;

	sql::ResultSet *result;

public:
	// Der Normalesierte Verbindungsaufbau des Systems 
	// TODO Erweiterung: Die Verbindungsdaten später über die GUI eintragen. Und erst dann die Verbindung herstellen.
	PRIVAT_MYSQL()
	{
		try
		{
			driver = get_driver_instance();
			con = driver->connect(server, username, password);
		}
		catch (sql::SQLException e)
		{
			cout << "Could not connect to server. Error message: " << e.what() << endl;
			system("pause");
			exit(1);
		}

		//please create database "quickstartdb" ahead of time
		con->setSchema("world");

		try
		{
			Preparestatments();
			if (SQL_EINLESEN) Vorgefertigte_Daten_Einlesen();
			SQL_EINLESEN = false;
		}
		catch (...)
		{
			Tabelle_erstellen();
		}
		
	}
	/* Aufräumen der Zeigerarythmetic (Ohne Smart Pointer = Speicherüberlauf "vorprogrammiert") */
	~PRIVAT_MYSQL()
	{
		//if (driver != 0)	delete driver;
		if (Daten_Einfügen_Höhle != 0)					delete Daten_Einfügen_Höhle;
		if (Daten_Einfügen_Mission != 0)				delete Daten_Einfügen_Mission;

		if (Alle_Daten_Lesen_Höhle != 0)				delete Alle_Daten_Lesen_Höhle;
		if (Alle_Daten_Lesen_Mission != 0)				delete Alle_Daten_Lesen_Mission;
		if (Alle_Daten_Lesen_Mission_Einmalig != 0)		delete Alle_Daten_Lesen_Mission_Einmalig;
		if (Einzel_Daten_Lesen_Mission != 0)			delete Einzel_Daten_Lesen_Mission;

		//if (pstmt != 0)		delete pstmt;
		if (con != 0)									delete con;
		if (result != 0)								delete result;
	}

	void Preparestatments()
	{
		Daten_Einfügen_Höhle = con->prepareStatement(SQL_EINFÜGEN_IN + CAVEENTRACE + "(Name, Position_X, Position_Y, Grundriss, Erzdeposits) VALUES(?,?,?,?,?);");
		Daten_Einfügen_Mission = con->prepareStatement(SQL_EINFÜGEN_IN + MISSIONEN + "(Position_X, Position_Y, Missionsname, Missionstyp, Missionsbeschreibung, Ren, Knoteninfo) VALUES(?,?,?,?,?,?,?);");

		Alle_Daten_Lesen_Höhle = con->prepareStatement(SQL_WÄHLE_ALLE_VON + CAVEENTRACE + ";");
		Alle_Daten_Lesen_Mission = con->prepareStatement(SQL_WÄHLE_ALLE_VON + MISSIONEN + ";");

		Alle_Daten_Lesen_Mission_Einmalig = con->prepareStatement(SQL_WÄHLE_EINZIGARTIGE + MISSIONEN + ";");

		Benutzereinfügen = con->prepareStatement(SQL_EINFÜGEN_IN + BENUTZERVERWALTUNG + "(name, passwort, level) VALUES(?,?,?);");
	}

	/* Funktion zum Einfügen der Höhlen Daten - Speicher der Position x + y, der Grundriss Datei und der Typischen Erz Knoten Anzahl*/
	void Daten_Einfügen(string name, float X, float Y, string Grundriss, int Anzahl)
	{
		Daten_Einfügen_Höhle->setString(1, name);
		Daten_Einfügen_Höhle->setDouble(2, X);
		Daten_Einfügen_Höhle->setDouble(3, Y);
		Daten_Einfügen_Höhle->setString(4, Grundriss);
		Daten_Einfügen_Höhle->setInt(5, Anzahl);

		Daten_Einfügen_Höhle->execute();
		cout << "One row in " + CAVEENTRACE + " inserted." << endl;
	}
	/* Funktion zum Einfügen der Missiontexte - Es wird ein X/Y gespeichert um Mögliche unterschiedliche Missionsziele zu speichern */
	void Daten_Einfügen(float X, float Y, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren = 0, int Knoteninfo = 0)
	{
		try
		{
			Daten_Einfügen_Mission->setDouble(1, X);
			Daten_Einfügen_Mission->setDouble(2, Y);
			Daten_Einfügen_Mission->setString(3, Missionsname);
			Daten_Einfügen_Mission->setString(4, Missionstyp);
			Daten_Einfügen_Mission->setString(5, Missionsbeschreibung);
			Daten_Einfügen_Mission->setInt(6, Ren);
			Daten_Einfügen_Mission->setInt(7, Knoteninfo);

			Daten_Einfügen_Mission->execute();
			cout << "One row in " + MISSIONEN + " inserted." << endl;
		}
		catch (...)
		{
			cout << "Achtung schreibfehler, umlaute müssen in \"alt\" geschrieben werden." << endl;
		}
	}
	/* Funktion zum Einfügen eines neuen Benutzers - Für den Login*/
	void Daten_Einfügen(string name, string passwort, int Lvl)
	{
		Benutzereinfügen->setString(1, name);
		/* Passwort Verschlüsselungsfunktion hier einfügen*/
		Benutzereinfügen->setString(2, PasswortVerschlüsseln(passwort));
		Benutzereinfügen->setInt(3, Lvl);

		Benutzereinfügen->execute();
		cout << "Neuen Benutzer hinzugefuegt." << endl;
	}
	
	/* Vorgefertigte Konfiguration */
	void Vorgefertigte_Daten_Einlesen()
	{
		Daten_Einfügen("B1", 2, 2, "Kleine", 10);
		Daten_Einfügen("C10", 4, 4, "Kleine", 10);
		Daten_Einfügen("E7", 1, 1, "Kleine", 10);
		Daten_Einfügen("G7South", 7, 7, "Kleine", 10);
		Daten_Einfügen("G7North", 9, 9, "Kleine", 10);
		Daten_Einfügen("H7", 6, 6, "Kleine", 10);

		Daten_Einfügen(1, 1, "Brueckenkopf", "Aufklaerung", "Aufklaerung Waldzone - Landung");

		Daten_Einfügen(1, 1, "Livewire", "Gelaende Scan", "Landung", 67 /* Versicherung  Zwangsaktiv*/);
		Daten_Einfügen(1, 1, "Livewire", "Gelaende Scan", "Scanne Ort 1");
		Daten_Einfügen(1, 1, "Livewire", "Gelaende Scan", "Scanne Ort 2");
		Daten_Einfügen(1, 1, "Livewire", "Gelaende Scan", "Scanne Ort 3");
		Daten_Einfügen(1, 1, "Livewire", "Gelaende Scan", "Exotic", 0, 1);

		Daten_Einfügen(1, 1, "Grabstein", "Geo Forschung", "Landung", 75);
		Daten_Einfügen(1, 1, "Grabstein", "Geo Forschung", "Setzte die Geostation am Standort Alpha.");
		Daten_Einfügen(1, 1, "Grabstein", "Geo Forschung", "Setzte die Geostation am Standort Beta.");
		Daten_Einfügen(1, 1, "Grabstein", "Geo Forschung", "Setzte die Geostation am Standort Gamma.");
		Daten_Einfügen(1, 1, "Grabstein", "Geo Forschung", "Setzte das Uplink am Standort Delta ein.");
		Daten_Einfügen(1, 1, "Grabstein", "Geo Forschung", "Exotic", 0, 1);

		Daten_Einfügen(1, 1, "Argos", "Erkundung", "Erkunde Icarus - Landung");

		Daten_Einfügen(1, 1, "Landwirtschaft", "Vorratslager", "Vorraete Wald", 250);

		Daten_Einfügen(1, 1, "Seltsame Ernte", "Bio Forschung", "Bio Forschung: Waldbiom", 50);

		Daten_Einfügen(1, 1, "Todesliste", "Vernichtung", "Landung", 125);
		Daten_Einfügen(1, 1, "Todesliste", "Vernichtung", "Folge der Spur des Raubtiers");
		Daten_Einfügen(1, 1, "Todesliste", "Vernichtung", "Toete das Raubtier");
		Daten_Einfügen(1, 1, "Todesliste", "Vernichtung", "Exotic", 0, 1);
		Daten_Einfügen(4.4, -16.0402, "Todesliste", "Vernichtung", "Wolf", 0, 2);

		Daten_Einfügen(1, 1, "Probelauf", "Expedition", "Expedition Canyons", 125);

		Daten_Einfügen(1, 1, "Todesstrahl", "Scan", "Landung", 150);
		Daten_Einfügen(1, 1, "Todesstrahl", "Scan", "Scanne Ort 1");
		Daten_Einfügen(1, 1, "Todesstrahl", "Scan", "Scanne Ort 2");
		Daten_Einfügen(1, 1, "Todesstrahl", "Scan", "Scanne Ort 3");

		Daten_Einfügen(1, 1, "Payramide", "Aufbau", "Landung", 175);
		Daten_Einfügen(1, 1, "Payramide", "Aufbau", "Erreiche den Bauplatz.");
		Daten_Einfügen(1, 1, "Payramide", "Aufbau", "Errichte ein Jagdaussenposten.");

		Daten_Einfügen(1, 1, "Sandige Bruecken", "Verlaengerte Untersuchung", "Fuehre eine Langzeit Untersuchung durch.", 300);

		Daten_Einfügen(1, 1, "Sandsturm", "Erkundung", "Erkunde Icarus.");

		Daten_Einfügen(1, 1, "Wasserwaage", "Untersuchung", "Landung", 150);
		Daten_Einfügen(1, 1, "Wasserwaage", "Untersuchung", "Uebertrage Geodaten vom Standort Alpha.");
		Daten_Einfügen(1, 1, "Wasserwaage", "Untersuchung", "Uebertrage Geodaten vom Standort Zulu.");

		Daten_Einfügen(1, 1, "Preservation", "Stockpile", "Landung", 350);

		Daten_Einfügen(1, 1, "Feld Test", "Erholung", "Landung", 150);
		Daten_Einfügen(1, 1, "Feld Test", "Erholung", "Sammle die verlorene gegangenen Komponenten des Prototyps.");
		Daten_Einfügen(1, 1, "Feld Test", "Erholung", "Untersuche Absturzstelle Alpha");
		Daten_Einfügen(1, 1, "Feld Test", "Erholung", "Untersuche Absturzstelle Bravo");
		Daten_Einfügen(1, 1, "Feld Test", "Erholung", "Untersuche Absturzstelle Delta");

		Daten_Einfügen(1, 1, "Massiv Metall", "Schwere Vorraete", "Landung", 350);
		Daten_Einfügen(1, 1, "Massiv Metall", "Schwere Vorraete", "Aufgelistete Ressourcen in Frachtkapsel ablegen.");

		Daten_Einfügen(1, 1, "Eissturm", "Expedition", "Landung", 125);
		Daten_Einfügen(1, 1, "Eissturm", "Expedition", "Platziere die Stoereinrichtung auf dem boden des arktischen Gletschers unter einem Underschlupf und aktiviere sie.");
		Daten_Einfügen(1, 1, "Eissturm", "Expedition", "Berge Geraetekomponente 1.");
		Daten_Einfügen(1, 1, "Eissturm", "Expedition", "Berge Geraetekomponente 2.");
		Daten_Einfügen(1, 1, "Eissturm", "Expedition", "Berge Geraetekomponente 3.");
		Daten_Einfügen(1, 1, "Eissturm", "Expedition", "Stelle das Geraet zusammen.");

		Daten_Einfügen(1, 1, "Tiefe Adern", "Extraktion", "Landung", 150);
		Daten_Einfügen(1, 1, "Tiefe Adern", "Extraktion", "Lokalesiere Exotische Materie.");
		Daten_Einfügen(1, 1, "Tiefe Adern", "Extraktion", "Baue Exotische Materie ab.");
		Daten_Einfügen(1, 1, "Tiefe Adern", "Extraktion", "Schaff edie Exotische Materie zum Landeschiff Lager.");

		Daten_Einfügen(1, 1, "Scheinwerfer", "Scan", "Landung", 150);
		Daten_Einfügen(1, 1, "Scheinwerfer", "Scan", "Scanne Ort 1");
		Daten_Einfügen(1, 1, "Scheinwerfer", "Scan", "Scanne Ort 2");
		Daten_Einfügen(1, 1, "Scheinwerfer", "Scan", "Scanne Ort 3");

		Daten_Einfügen(1, 1, "Hochseilgarten", "Erkundung", "Erkunde Icarus");

		Daten_Einfügen(9.12004, 9.80004, "Zufluss", "Aufbau", "Landung", 275);
		Daten_Einfügen(11.94, -15.51, "Zufluss", "Aufbau", "Erreiche den markierten Ort.");
		Daten_Einfügen(12.06, -15.55, "Zufluss", "Aufbau", "Errichte eine Kaserne.");
		Daten_Einfügen(14.23, -9.60001, "Zufluss", "Aufbau", "Exotic1");
		Daten_Einfügen(12.05, -15.45, "Zufluss", "Aufbau", "6 warme + geschuetzte Schlafsaecke");
		Daten_Einfügen(12, -15.54, "Zufluss", "Aufbau", "Kanonenofen");
		Daten_Einfügen(12, -15.54, "Zufluss", "Aufbau", "Kochstelle");
		Daten_Einfügen(11.97, -15.58, "Zufluss", "Aufbau", "Oxidator");


		Daten_Einfügen(1, 1, "Zerbrochene Pfeile", "Erholung", "Landung", 150);
		Daten_Einfügen(1, 1, "Zerbrochene Pfeile", "Erholung", "Beschaffe den Biosprengkopf von Absturzstelle Omega.");
		Daten_Einfügen(1, 1, "Zerbrochene Pfeile", "Erholung", "Beschaffe den Biosprengkopf von Absturzstelle Psi.");
		Daten_Einfügen(1, 1, "Zerbrochene Pfeile", "Erholung", "Beschaffe den Biosprengkopf von Absturzstelle Chi.");

		Daten_Einfügen(1, 1, "Zahltag", "Extraktion", "Landung", 400);
		Daten_Einfügen(1, 1, "Zahltag", "Extraktion", "Lokalesiere Exotische Materie.");
		Daten_Einfügen(1, 1, "Zahltag", "Extraktion", "Baue ein exotisches Materievorkommen vollstaeding ab.");
		Daten_Einfügen(1, 1, "Zahltag", "Extraktion", "Kehre mit exotischer Materie in den Orbit zurueck.");

		Daten_Einfügen(1, 1, "Landschaft", "Hydrokultur", "Landung", 250);
		Daten_Einfügen(1, 1, "Landschaft", "Hydrokultur", "Stelle Anbauausruestung her.");
		Daten_Einfügen(1, 1, "Landschaft", "Hydrokultur", "Errichte Anbauflaechen in einem Gewaechshaus(Glas-Gebaeude).");
		Daten_Einfügen(1, 1, "Landschaft", "Hydrokultur", "Deponiere Aufgesitete (In Anbauflaechen) angebaute Ressourcen in der Frachtkapsel.");

		Daten_Einfügen(1, 1, "Einfall", "Scan", "Landung", 200);
		Daten_Einfügen(1, 1, "Einfall", "Scan", "Scanne Ort 1");
		Daten_Einfügen(1, 1, "Einfall", "Scan", "Scanne Ort 2");
		Daten_Einfügen(1, 1, "Einfall", "Scan", "Scanne Ort 3");

		Daten_Einfügen(1, 1, "Aufstocken", "Vorraete", "Landung", 600);
		Daten_Einfügen(1, 1, "Aufstocken", "Vorraete", "Aufgelistete Ressourcen in Frachtkapsel ablegen.");

		Daten_Einfügen(1, 1, "Edelweiss", "Erkundung", "Erkunde Icarus");

		Daten_Einfügen(1, 1, "Erhebung", "Bio-Forschung", "Landung", 200);
		Daten_Einfügen(1, 1, "Erhebung", "Bio-Forschung", "Finde und untersuche die ungewoehnliche Flora.");
		Daten_Einfügen(1, 1, "Einfall", "Bio-Forschung", "Finde die Daten zur Geode und grabe Sie aus.");

		Daten_Einfügen(1, 1, "Lawine", "Expedition", "Landung", 600);
		Daten_Einfügen(1, 1, "Lawine", "Expedition", "Erreiche den Aktispass(M)");
		Daten_Einfügen(1, 1, "Lawine", "Expedition", "Kundschafte das auf der Karte markierte Gebiet aus(M).");

		Daten_Einfügen(1, 1, "Feuergang", "Lieferung", "Landung", 250);
		Daten_Einfügen(1, 1, "Feuergang", "Lieferung", "Terraforming Fechette abrufen,");
		Daten_Einfügen(1, 1, "Feuergang", "Lieferung", "Liefern sie die Flechette an ihr Dropship.");

		Daten_Einfügen(1, 1, "Drecksarbeit", "Vernichtung", "Landung", 250);
		Daten_Einfügen(1, 1, "Drecksarbeit", "Vernichtung", "Suche nach Tierspuren.");
		Daten_Einfügen(1, 1, "Drecksarbeit", "Vernichtung", "Suche nach der Hoehle des Tiers");
		Daten_Einfügen(1, 1, "Drecksarbeit", "Vernichtung", "Toete das Raubtier");

		Daten_Einfügen(1, 1, "Gebuendelt", "Verlaengerte Untersuchung", "Landung", 300);
		Daten_Einfügen(1, 1, "Gebuendelt", "Verlaengerte Untersuchung", "Fuehre eine Langzeit Untersuchung durch");

		Daten_Einfügen(1, 1, "Concealment", "Recovery", "Landung", 400);
		Daten_Einfügen(1, 1, "Concealment", "Recovery", "Zerstoere Arachnideneidaemmungseinheit 1");
		Daten_Einfügen(1, 1, "Concealment", "Recovery", "Zerstoere Arachnideneidaemmungseinheit 2");

		Daten_Einfügen(1, 1, "Schneeblind", "Scan", "Landung", 250);
		Daten_Einfügen(1, 1, "Schneeblind", "Scan", "Scanne Ort 1");
		Daten_Einfügen(1, 1, "Schneeblind", "Scan", "Scanne Ort 2");
		Daten_Einfügen(1, 1, "Schneeblind", "Scan", "Scanne Ort 3");

		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Landung", 250);
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Orte die ersten Absturzstelle");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Orte die zweite Absturzstelle");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Orte die letzte Absturzstelle");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Besiege die Kreatur");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Orte die dritte Absturzstelle");

		Daten_Einfügen(1, 1, "Sieben Saulen", "Scan", "Landung", 200);
		Daten_Einfügen(1, 1, "Sieben Saulen", "Scan", "Scanne Ort 1");
		Daten_Einfügen(1, 1, "Sieben Saulen", "Scan", "Scanne Ort 2");
		Daten_Einfügen(1, 1, "Sieben Saulen", "Scan", "Scanne Ort 3");

		Daten_Einfügen(1, 1, "Bonze", "Vorraete", "Ladnung");
		Daten_Einfügen(1, 1, "Nachtache", "Geo-Forschung", "Ladnung");
		Daten_Einfügen(1, 1, "Gelobtes Land", "Erkundung", "Ladnung");
		Daten_Einfügen(1, 1, "Abstauben", "Vernichtung", "Ladnung");
		Daten_Einfügen(1, 1, "Sandkasten", "Aufbau", "Ladnung");
		Daten_Einfügen(1, 1, "Schneeunfall", "Erholung", "Ladnung");
		Daten_Einfügen(1, 1, "Saeuberung", "Vernichtung", "Ladnung");
		Daten_Einfügen(1, 1, "Erweiterte Bestellung", "Vorraete", "Ladnung");
		Daten_Einfügen(1, 1, "Tundra", "Erkundung", "Ladnung");
		Daten_Einfügen(1, 1, "Station zu Station", "Verlaengerte Untersuchung", "Ladnung");
		Daten_Einfügen(1, 1, "Reisende", "Erholung", "Ladnung");
		Daten_Einfügen(1, 1, "Pilgerreise", "Erkundung", "Ladnung");
		Daten_Einfügen(1, 1, "Kryogen", "Forschung", "Ladnung");
		Daten_Einfügen(1, 1, "Ausgegraben", "Forschung", "Ladnung");

		Daten_Einfügen(1, 1, "Meridian", "Extraktion", "Ladnung", 250);
		Daten_Einfügen(1, 1, "Meridian", "Extraktion", "Sammel die Ausruestung aus dem experimentalen Abbau Depo ein.");
		Daten_Einfügen(1, 1, "Meridian", "Extraktion", "Sammel die Materialien und liefer die in das experimentale Abbau Depo.");
	}

	/* Basisfunktion sofern noch keine Tabelle erstellt worden ist */
	void Tabelle_erstellen()
	{
		stmt = con->createStatement();
		stmt->execute("DROP TABLE IF EXISTS " + CAVEENTRACE);
		std::cout << "Finished dropping table " + CAVEENTRACE + " (if existed)" << endl;
		stmt->execute("CREATE TABLE " + CAVEENTRACE + " (id serial PRIMARY KEY, Name VARCHAR(50), Position_X FLOAT, Position_Y FLOAT, Grundriss VARCHAR(50), Erzdeposits INTEGER);");
		cout << "Finished creating table" + CAVEENTRACE << endl;

		stmt->execute("DROP TABLE IF EXISTS " + MISSIONEN);
		cout << "Finished dropping table " + MISSIONEN + " (if existed)" << endl;
		stmt->execute("CREATE TABLE " + MISSIONEN + " (id serial PRIMARY KEY, Position_X FLOAT, Position_Y FLOAT, Missionsname VARCHAR(50), Missionstyp VARCHAR(50), Missionsbeschreibung VARCHAR(150), Ren INTEGER, Knoteninfo INTEGER);");
		cout << "Finished creating table" + MISSIONEN << endl;

		stmt->execute("DROP TABLE IF EXISTS " + BENUTZERVERWALTUNG);
		cout << "Finished dropping table " + BENUTZERVERWALTUNG + " (if existed)" << endl;
		stmt->execute("CREATE TABLE " + BENUTZERVERWALTUNG + " (id serial PRIMARY KEY, name VARCHAR(50), passwort VARCHAR(50), level INTEGER);");
		cout << "Finished creating table" + BENUTZERVERWALTUNG << endl;

		delete stmt;

		Vorgefertigte_Daten_Einlesen();
	}

	/* Demo Funktion - Dies ist eine Demo Funktion zum Außlesen aller Daten ungefiltert und unsortiert */
	void Daten_Lesen()
	{
		result = Alle_Daten_Lesen_Höhle->executeQuery();
		Höhlenentraces.clear();
		while (result->next())
		{
			Höhlenentraces.push_back(Höhlendaten(result->getString(2).c_str(), result->getDouble(3), result->getDouble(4), result->getString(5).c_str(), result->getInt(6)));
		}

		for (unsigned int i = 0; i < Höhlenentraces.size(); i++)
		{
			printf("Reading from table Caveentrace=(%d, %d, %d, %s, %d)\n", i + 1, Höhlenentraces[i].getX(), Höhlenentraces[i].getY(), Höhlenentraces[i].Grundriss, Höhlenentraces[i].ErzknotenAnzahl);
		}

		result = Alle_Daten_Lesen_Mission->executeQuery();

		MISSIONSDATEN.clear();
		while (result->next())
		{
			MISSIONSDATEN.push_back(Missionsdaten(result->getDouble(2), result->getDouble(3), result->getString(4).c_str(), result->getString(5).c_str(),
				result->getString(6).c_str(), result->getInt(7)));
		}

		for (unsigned int i = 0; i < MISSIONSDATEN.size(); i++)
		{
			std::cout << "Reading from table Mission = (" << i + 1 << ", " << MISSIONSDATEN[i].getX() << ", " << MISSIONSDATEN[i].getY()
				<< ", " << MISSIONSDATEN[i].getName() << ": " << MISSIONSDATEN[i].getTyp();
			cout << ", " << MISSIONSDATEN[i].getBeschreibung() << ")" << endl;
		}
	}

	/* Funktion zum Auslesen der eingetragenden Missionen (Gefiltert, jede Mission wird nur einmal aufgezählt) - Clean */
	vector<pair<string, string>> Daten_Lesen_Missionsname()
	{
		vector<pair<string, string>> temp;

		result = Alle_Daten_Lesen_Mission_Einmalig->executeQuery();

		while (result->next())
			temp.push_back(pair<string, string>(result->getString(1), result->getString(2)));

		return temp;
	}

	/* Funktion zum auslesen der Höhlenpositionen und deren Daten */
	vector<glm::vec2> Hölenpositionen()
	{
		vector<glm::vec2> temp;
		result = Alle_Daten_Lesen_Höhle->executeQuery();
		Höhlenentraces.clear();
		while (result->next())
		{
			int tx = result->getInt(3);
			int ty = result->getInt(4);
			Höhlenentraces.push_back(Höhlendaten(result->getString(2).c_str(), tx, ty, result->getString(5).c_str(), result->getInt(6)));
			temp.push_back(glm::vec2(tx, ty));
		}
		return temp;
	};

	/* Funktion zum Auslesen der Ausgewählten Missionsinformationen - Clean(Konsole) - TODO(Im Programm): Rückgabe als Datensatz zum weiterverarbeiten innerhalb der Schleife */
	vector<Missionsdaten> Daten_Lesen(pair<string, string> name)
	{
		vector<Missionsdaten> temp;

		Einzel_Daten_Lesen_Mission = con->prepareStatement(SQL_WÄHLE_ALLE_VON + MISSIONEN + SQL_WO + MISSIONSNAME + "='" + name.first + "'" + " AND " + MISSIONSTYP + "='" + name.second + "'");

		result = Einzel_Daten_Lesen_Mission->executeQuery();
		bool Erste_Missionsinfo = false;

		while (result->next())
		{
			if (result->getInt(8) == 0)
			{
				temp.push_back(Missionsdaten(result->getDouble(2), result->getDouble(3), result->getString(4).c_str(), result->getString(5).c_str(), result->getString(6).c_str(), result->getInt(7)));

				if (!Erste_Missionsinfo)
				{
					cout << "Gelesen von Tabelle " + MISSIONEN + " = bei x=" << temp.back().getX() << " | y=" << temp.back().getY() << endl;
					cout << "Die Mission \"" << temp.back().getName() << ": " << temp.back().getTyp() << "\"" << endl;
					Erste_Missionsinfo = true;
				}

				cout << "Die Aufgabe ist: \"" << temp.back().getBeschreibung() << "\"";
				if (temp.back().getRen() != 0)
					cout << " und als Belohnungen gibt es " << temp.back().getRen() << " Ren ";
				cout << endl;
			}
			else if (result->getInt(8) == 1)
			{
				EXOTICknoten.push_back(BASISATTRIBUTE(result->getDouble(2), result->getDouble(3), 2, result->getString(4).c_str(), result->getString(6).c_str()));
			}
			else if (result->getInt(8) == 2)
			{
				BOSSEknoten.push_back(BASISATTRIBUTE(result->getDouble(2), result->getDouble(3), 3, result->getString(4).c_str(), result->getString(6).c_str()));
			}			
		}

		Einzel_Daten_Lesen_Mission->close();
		return temp;
	}

	/* Funktion zum auslesen eines bestimmten Benutzernamens - soll das Passwort und das Benutzerlevel wiedergeben */
	pair<string, int> Daten_Lesen(string name)
	{
		Benutzerlesen = con->prepareStatement(SQL_WÄHLE_ALLE_VON + BENUTZERVERWALTUNG + SQL_WO + "name='" + name + "'");

		pair<string, int> temp;
		temp.second = -2;							/* Loginname nicht gefunden */

		result = Benutzerlesen->executeQuery();
		while (result->next())
		{
			temp.first = result->getString(3);
			temp.second = result->getInt(4);
		}
		return temp;
	}

	/* TODO(Im Programm) nach Loginfenster zum Verschieben einzelner Objekte als Datenbank Manager */
	void Daten_Updaten(BASISATTRIBUTE *ID)
	{
		//update
		switch (ID->Index)
		{
		case 1:
		case 2:
		case 3:
			pstmt = con->prepareStatement("UPDATE " + MISSIONEN + " SET Position_X = ?, Position_Y = ? WHERE Missionsname = ? AND Missionsbeschreibung = ?");
			break;
		case 0:
		default:
			pstmt = con->prepareStatement("UPDATE " + CAVEENTRACE + " SET Position_X = ?, Position_Y = ? WHERE name = ?");
			break;
		}
		
		pstmt->setDouble(1, ID->X);
		pstmt->setDouble(2, ID->Y);
		pstmt->setString(3, ID->name);
		if(ID->Index == 1 || ID->Index == 2 || ID->Index == 3)
			pstmt->setString(4, ID->Zusatz);
					
		pstmt->executeQuery();
	}
	/* Die Funktion dient zum Updaten der als Missionsdaten gepsiecherten Informationen in SQL */
	void Daten_Updaten(int ID, char* Name, char* Typ, char* Beschreibung, float x, float y, int ren, BASISATTRIBUTE *t)
	{
		/* Deklarierung */
		uint Unr = 0;
		ostringstream ss;
		bool PN = 0, PT = 0, PB = 0, PX = 0, PY = 0, PR = 0;
		int te = 0;

		/* Prüfung */
		switch (ID)
		{
			/*
				ID 0->Wenn der Name oder der Typ unterschiedlich ist->Führe ein Update von Name und Typ durch
				ID 0->Wenn X oder Y oder Ren Unterschiedlich sind - Führe ein Update bei Missionsbeschreibung = "Landung" durch
			*/
		case 0:
			t = &MISSIONSDATEN[0];
			
			

			PN = !compare(t->getName(), Name); 
			PT = !compare(MISSIONSDATEN[0].getTyp(), Typ); 
			PB = !compare(t->Zusatz, Beschreibung); 
			PX = !(t->X == x);
			PY = !(t->Y == y);
			PR = !(MISSIONSDATEN[0].getRen() == ren);

			break;
			/*
				ID 2 -> Wenn X oder Y oder Beschreibung unterschiedlich sind - Führe ein Update mithilfe von Mission und Beschreibung durch
			*/
		case 2:
			if (t->Zusatz.compare(Beschreibung)							!= 0)						{ PB = true; };
			if (t->getX()												!= x)						{ PX = true; };
			if (t->getY()												!= y)						{ PY = true; };
			break;
			/*
				ID 3 -> Wenn X oder Y unterschiedlich sind					 - Führe ein Update mithilfe von Mission und Beschreibung durch
			*/
		case 3:
			if (t->getX()												!= x)						{ PX = true; };
			if (t->getY()												!= y)						{ PY = true; };
			break;
			/*
				ID 4 -> Wenn X oder Y oder Beschreibung unterschiedlich sind - Führe ein Update mithilfe von Mission und Beschreibung durch
			*/
		case 4:
			if (t->Zusatz.compare(Beschreibung)							!= 0)						{ PB = true; };
			if (t->getX()												!= x)						{ PX = true; };
			if (t->getY()												!= y)						{ PY = true; };
			break;
		default:
			throw "Falscher Index";
		}
		
		if (ID == 0)
		{
			/* Updaten von "Allen" Namen und Typen */
			if ((PN || PT))
			{
				ss << "UPDATE " << MISSIONEN << " SET Missionsname = ?, Missionstyp = ?";
				ss << " WHERE Missionsname = ? AND Missionstyp = ?";
				pstmt = con->prepareStatement(ss.str().c_str());

				Unr++; pstmt->setString(Unr, Name);
				Unr++; pstmt->setString(Unr, Typ);
				Unr++; pstmt->setString(Unr, MISSIONSDATEN[0].getName());
				Unr++; pstmt->setString(Unr, MISSIONSDATEN[0].getTyp());

				pstmt->executeQuery();
				for (uint i = 0; i < MISSIONSDATEN.size(); i++)
				{
					MISSIONSDATEN[i].name = Name;
					MISSIONSDATEN[i].Missionstyp = Typ;
				}
				
				PN = PT = false;
			}
			Unr = 0;
			ss = ostringstream();
		}
		
		if (!PN && !PT && !PB && !PX && !PY && !PR) return;
		/* Erstellung des Basistextes */
		ss << "UPDATE " << MISSIONEN << " SET";

		if (PN) { ss << " Missionsname = ?";			if (PT || PB || PX || PY || PR) { ss << ","; }; };
		if (PT) { ss << " Missionstyp = ?";				if		 (PB || PX || PY || PR) { ss << ","; }; };
		if (PB) { ss << " Missionsbeschreibung = ?";	if			   (PX || PY || PR) { ss << ","; }; };
		if (PX) { ss << " Position_X = ?";				if					 (PY || PR) { ss << ","; }; };
		if (PY) { ss << " Position_Y = ?";				if						   (PR) { ss << ","; }; };
		if (PR) { ss << " Ren = ?"; };

		ss << " WHERE Missionsname = ?" << " AND Missionsbeschreibung = ?";
		
		pstmt = con->prepareStatement(ss.str().c_str());// "UPDATE " + MISSIONEN + " SET Position_X = ?, Position_Y = ? WHERE Missionsname = ? AND Missionsbeschreibung = ?");

		/* Eingabe der Werte */
		if (PN) { Unr++; pstmt->setString(Unr, Name); };
		if (PT) { Unr++; pstmt->setString(Unr, Typ); };
		if (PB) { Unr++; pstmt->setString(Unr, Beschreibung); };
		if (PX) { Unr++; pstmt->setDouble(Unr, x); };
		if (PY) { Unr++; pstmt->setDouble(Unr, y); };
		if (PR) { Unr++; pstmt->setInt   (Unr, ren); };
				  Unr++; pstmt->setString(Unr, t->getName());
				  Unr++; pstmt->setString(Unr, t->Zusatz);
				  
		pstmt->executeQuery();
		
		if(PX) t->X = x;
		if(PY) t->Y = y;
		if (PR) MISSIONSDATEN[0].Belohnung_Ren = ren;
		if (PB) 
		{
			t->Zusatz = Beschreibung;
			if (ID == 0 || ID == 2)
				MISSIONSDATEN[Fensterdaten.Arrayindex].Missionsbeschreibung = Beschreibung; 
		};
	}
	
	/* Funktion zum Updaten von Höhlendaten mit vorher bekannten Namen */
	void Daten_Updaten(char* Name, char* Typ, float x, float y, BASISATTRIBUTE *t)
	{
		/* Deklarierung */
		uint Unr = 0;
		bool UpdateX = 0, UpdateY = 0, UpdateGrund = 0, UpdateName = 0;
		ostringstream ss;		

		/* Prüfung */
		if (t->getX() != x) UpdateX = true;
		if (t->getY() != y) UpdateY = true;
		if (Höhlenentraces[Fensterdaten.Arrayindex].Grundriss.find(Typ) == Höhlenentraces[Fensterdaten.Arrayindex].Grundriss.npos) UpdateGrund = true;
		if (t->name.find(Name) == t->name.npos) UpdateName = true;

		/* Vorbereitung des Statments */
		ss << "UPDATE " << CAVEENTRACE << " SET";
		if (UpdateX) {		ss << " Position_X = ?";	if (UpdateY || UpdateGrund || UpdateName)	ss << ","; };
		if (UpdateY) {		ss << " Position_Y = ?";	if			  (UpdateGrund || UpdateName)	ss << ","; };
		if (UpdateGrund) {	ss << " Grundriss= ?";		if							 (UpdateName)	ss << ","; };
		if (UpdateName)		ss << "  name = ?";
		ss << " WHERE name = ?";
		pstmt = con->prepareStatement(ss.str().c_str());				//"UPDATE " + CAVEENTRACE + " SET Position_X = ?, Position_Y = ?, Grundriss= ?, name = ? WHERE name = ?");

		/* Eintragen der Werte */
		if (UpdateX)		{ Unr++; pstmt->setDouble(Unr, x); };
		if (UpdateY)		{ Unr++; pstmt->setDouble(Unr, y); };
		if (UpdateGrund)	{ Unr++; pstmt->setString(Unr, Typ); };
		if (UpdateName)		{ Unr++; pstmt->setString(Unr, Name); };
							  Unr++; pstmt->setString(Unr, t->getName());

		/* Ersetzen in der Datenbank*/
		pstmt->executeQuery();
		/* Ersetzen im Internen Speicher */
		t->name = Name;
		Höhlenentraces[Fensterdaten.Arrayindex].Zusatz = Höhlenentraces[Fensterdaten.Arrayindex].Grundriss = Typ;
		Höhlenentraces[Fensterdaten.Arrayindex].X = x;
		Höhlenentraces[Fensterdaten.Arrayindex].Y = y;
	}

	/* Funktion zum aktualisieren des Passwortes */
	void Daten_Updaten(string name, string passwort, int level)
	{

		pstmt = con->prepareStatement("UPDATE " + BENUTZERVERWALTUNG + " SET passwort = ?, level = ? WHERE name = ?");

		/* Verschlüssselung hier einfügen */
		pstmt->setString(1, PasswortVerschlüsseln(passwort));
		pstmt->setInt(2, level);
		pstmt->setString(3, name);

		pstmt->executeQuery();
	}

	/* TODO(Im Programm) nach Loginfenster zum entfernen einzelner Objekte als Datenbank Manager 
		Löschen von einer "Mission" und damit Verbundene Unterpunkte	- 'Missionsname', 'Missionstyp', 0
		Löschen einer Höhle												- 'Name'		,			 '', 1
	*/
	void Daten_Löschen(string name,string Zusatz, int index)
	{
		/* Variablen Deklaration */
		ostringstream ss;
		uint Unr = 0;

		/* Basisn Initialiseriung */
		ss << "DELETE FROM ";
		/* Ablauf für die Löschung einer Mission*/
		if (index == 0)
		{
			ss << MISSIONEN;
			ss << " WHERE " ;
			ss << "Missionsname = ?" << " AND ";		
			ss << "Missionstyp = ?";
		}
		/* Ablauf für die Löschung einer Höhle*/
		else if (index == 1)
		{
			ss << CAVEENTRACE;
			ss << " WHERE name = ?";
		}
		/* Ablauf für die Löschung eines Missionsziels, Eco Spots, Boss Spots*/
		else if (index == 2 || index == 3 || index == 4)
		{
			ss << MISSIONEN;
			ss << " WHERE ";
			ss << "Missionsname = ?" << " AND ";
			ss << "Missionsbeschreibung = ?";			
		}

		pstmt = con->prepareStatement(ss.str().c_str());

		Unr++; pstmt->setString(Unr, name);
		if (index != 1) { Unr++; pstmt->setString(Unr, Zusatz); }

		result = pstmt->executeQuery();	

		if (index == 0)
		{
			Missionsliste = SQL.Daten_Lesen_Missionsname();
			MISSIONSDATEN.clear();
			EXOTICknoten.clear();
			BOSSEknoten.clear();
			Mission_Ausgewählt = false;
		}
		else if (index == 1)
		{			
			Höhlenentraces.erase(Höhlenentraces.begin() + Fensterdaten.Arrayindex);
		}
		else if (index == 2)
		{
			MISSIONSDATEN.erase(MISSIONSDATEN.begin() + Fensterdaten.Arrayindex);
		}
		else if (index == 3)
		{
			EXOTICknoten.erase(EXOTICknoten.begin() + Fensterdaten.Arrayindex);
		}
		else if (index == 4)
		{
			BOSSEknoten.erase(BOSSEknoten.begin() + Fensterdaten.Arrayindex);
		}
		Fensterdaten.ID = 0;
		Fensterdaten.Arrayindex = 99999;
	}
}SQL;

class FREETYPE
{
	FT_Library ft;
	uint VAO, VBO;
public:
	~FREETYPE()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	inline int Initialisieren()
	{
		// FreeType
		// --------

		// All functions return a value different than 0 whenever an error occurred
		if (FT_Init_FreeType(&ft))
		{
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			return -1;
		}

		// find path to font
		std::string font_name = "Resourcen/28 Days Later.ttf";
		if (font_name.empty())
		{
			std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
			return -1;
		}

		// load font as face
		FT_Face face;
		if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			return -1;
		}
		else {
			// set size to load glyphs as
			FT_Set_Pixel_Sizes(face, 0, 48);

			// disable byte-alignment restriction
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// load first 128 characters of ASCII set
			for (unsigned char c = 0; c < 128; c++)
			{
				// Load character glyph 
				if (FT_Load_Char(face, c, FT_LOAD_RENDER))
				{
					std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
					continue;
				}
				// generate texture
				unsigned int texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexImage2D(
					GL_TEXTURE_2D,
					0,
					GL_RED,
					face->glyph->bitmap.width,
					face->glyph->bitmap.rows,
					0,
					GL_RED,
					GL_UNSIGNED_BYTE,
					face->glyph->bitmap.buffer
				);
				// set texture options
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				// now store character for later use
				Character character = {
					texture,
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					static_cast<unsigned int>(face->glyph->advance.x)
				};
				Characters.insert(std::pair<char, Character>(c, character));
			}
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		// destroy FreeType once we're finished
		FT_Done_Face(face);
		FT_Done_FreeType(ft);
	}

	/* Die Funktion um das Text Rändern zu ermöglichen */
	inline void aktivieren()
	{
		// OpenGL state
		// ------------
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	};

	inline void deaktivieren()
	{
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
	};

	// Erstelle die Text Grafik - Render the Text Line
	inline void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
	{
		//GLboolean t;
		//glGetBooleanv(GL_CULL_FACE,&t);


		// activate corresponding render state	
		shader.use();
		glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);

		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
			};
			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

/* Klasse zum Zeichnen der Karte */
class MAP
{
	uint VBO, VAO, EBO;
public:
	MAP()
	{
		// configure VAO/VBO for texture quads
	// -----------------------------------
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		// set up vertex data (and buffer(s)) and configure vertex attributes
		// ------------------------------------------------------------------
		float vertices[] = {
			// positions          // texture coords
			20.5f,  20.5f, 0.0f,   1.0f, 1.0f, // top right
			20.5f, -20.5f, 0.0f,   1.0f, 0.0f, // bottom right
			-20.5f, -20.5f, 0.0f,   0.0f, 0.0f, // bottom left
			-20.5f,  20.5f, 0.0f,   0.0f, 1.0f  // top left 
		};
		unsigned int indices[] = {
			0, 1, 3, // first triangle
			1, 2, 3  // second triangle
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// texture coord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	};
	~MAP()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}


	void Render(Shader *shader, uint *tex0, uint *tex1)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *tex0); // texture1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, *tex1); // texture2

		// get matrix's uniform location and set matrix
		shader->use();

		// create transformations
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		glm::mat4 view = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0 + Cam.C.x, 0.0f + Cam.C.y, Cam.C.z));

		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		// retrieve the matrix uniform locations
		unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
		unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		shader->setMat4("projection", Cam.projection);


		// render container
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
};

/* Klasse für die Knotenpunktanzeige - 90% Identisch mit "Map" */
class Objektmarker
{
	uint VBO, VAO, EBO;
	uint instanceVBO;
	vector<glm::vec2> translations;
public:
	Objektmarker()
	{
		glm::vec2 translation(0);
		translations.push_back(translation);

		// store instance data in an array buffer		
		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * translations.size(), &translations[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Der Speicher für den Zeichenbereich der Markierung
		float vertices[] = {
			// positions         // texture coords
			 0.1f, 0.3f, 0.0f,   1.0f, 1.0f,	// top right
			 0.1f, 0.0f, 0.0f,   1.0f, 0.0f,	// bottom right
			-0.1f, 0.0f, 0.0f,   0.0f, 0.0f,	// bottom left

			-0.1f, 0.3f, 0.0f,   0.0f, 1.0f,	// top left 
			 0.1f, 0.3f, 0.0f,   1.0f, 1.0f,	// top right
			-0.1f, 0.0f, 0.0f,   0.0f, 0.0f		// bottom left
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		/* In GLSL der Speicherbereich für die Verxtex(Punkt) Positionen */
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		/* IN GLSL der Speicherbereich für die Texturen Position */
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		/* Die Instanzdaten */
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
	};
	~Objektmarker()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void update(glm::vec2 Pos, int i)
	{
		translations[i] = Pos;
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * translations.size(), &translations[0], GL_DYNAMIC_DRAW);
	};
	void update(vector<glm::vec2> Pos)
	{

		translations = Pos;
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * translations.size(), &translations[0], GL_DYNAMIC_DRAW);
	};

	void Render(Shader *shader, uint &tex0, uint &tex1)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0); // texture1

		// get matrix's uniform location and set matrix
		shader->use();

		// create transformations
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		glm::mat4 view = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0 + Cam.C.x, 0.0f + Cam.C.y, -0.001 + Cam.C.z));

		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
			   
		// retrieve the matrix uniform locations
		unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
		unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		shader->setMat4("projection", Cam.projection);
		
		// render container
		glBindVertexArray(VAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, translations.size()); // 100 triangles of 6 vertices each//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
};

/* Callback zur Abfrage der Fenstergröße */
inline void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	Fensterdaten(width, height);
}

/* Callback zur Abfrage des Mausrades*/
inline void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!Interface_Hovered)
	{
		Cam.C.z += yoffset;
	}
};

/* Callback für die Abfrage der Maussteuerung und Übertragen auf den Internen Speicher */
inline void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		Fensterdaten.Maus.Is_Links_Klickt = GLFW_PRESS;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_PRESS)
	{
		Fensterdaten.Maus.Is_Links_Klickt = 0;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		Fensterdaten.Maus.Is_Rechts_Klickt = GLFW_PRESS;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_PRESS)
	{
		Fensterdaten.Maus.Is_Rechts_Klickt = 0;
	}

	if (action == GLFW_PRESS)
	{
		Fensterdaten.Maus.Action = action;
	}
}

/* Funktion zur Umwandlung der X-Y Koordinaten des "Displays" in 3D Weltkoordinaten - TODO Korrektur der Anwahl - Nicht Vollständig */
inline glm::vec3 Coursor_Cast(glm::mat4 Projection)
{
	glReadBuffer(GL_FRONT);
	glReadPixels(Fensterdaten.Maus.X, Fensterdaten.Maus.Y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Fensterdaten.Maus.Z);

	glm::mat4 model = glm::mat4{ 1.0f };
	glm::mat4 view  = glm::mat4{ 1.0f };
	glm::mat4 proj  = glm::mat4{ 1.0f };

	model = glm::translate(model, glm::vec3(0.0 + Cam.C.x, 0.0f + Cam.C.y, -0.001 + Cam.C.z));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); 

	proj = Projection * view;

	cout << "Mit der Cam bei:" << Cam.C.x << "," << Cam.C.y << "," << Cam.C.z << endl;
	cout << "und dem Mauszeiger bei:" << Fensterdaten.Maus.X << "," << Fensterdaten.Maus.Y << endl;

	for (float i = -1.0; i <= 1.0; i += 0.1)
	{
		glm::vec3 t1 = glm::unProjectNO(glm::vec3(Fensterdaten.Maus.X, Fensterdaten.Maus.Y, i), model, proj, Fensterdaten.viewport);
		cout << "erhalte ich bei z"<< i << " | die Koordianten" << t1.x << "," << t1.y << "," << t1.z << endl;
	}
	glm::vec3 t1 = glm::unProjectNO(glm::vec3(Fensterdaten.Maus.X, Fensterdaten.Maus.Y, 0.0), model, proj, Fensterdaten.viewport);
	return t1;
}

/* Der AABB vergleich zur Anwahl  - Clean*/
inline bool compare(glm::vec3 Cursor, BASISATTRIBUTE *Objekt)
{
	/* Wenn das X und Y  innerhalb des Bereichs liegt - gebe den namen zurück */
	if (Cursor.x >= (Objekt ->getX() + (-0.1)) && Cursor.x <= (Objekt->getX() + (0.1)) &&
		Cursor.y >= (Objekt ->getY() + (-0.1)) && Cursor.y <= (Objekt->getY() + (0.1)))
		return true;
	return false;
}

/* Die Maus Anwahlfunktion, mithilfe von Hovering */
inline void Objektanwahl()
{
	/* Daten des Coursors im 3D Raum, für den vergleich zur Anwahl */
	glm::vec3 T0 = Coursor_Cast(Cam.projection);

	Statusanzeige = true;

	if (Fensterdaten.ID == 0)
	{
		/* Auswahl ist eine Höhle */
		for (uint i = 0; i < Höhlenentraces.size(); i++)
		{
			if (compare(T0, &Höhlenentraces[i]))
			{
				Fensterdaten.ID = &Höhlenentraces[i];
				Fensterdaten.Arrayindex = i;
				if (Fensterdaten.ID != 0)
					cout << Fensterdaten.ID->name << endl;
				return;
			}
		}
		/* Auswahl ist ein Missionsziel */
		for (uint i = 0; i < MISSIONSDATEN.size(); i++)
		{
			if (compare(T0, &MISSIONSDATEN[i]))
			{
				Fensterdaten.ID = &MISSIONSDATEN[i];
				Fensterdaten.Arrayindex = i;
				if (Fensterdaten.ID != 0)
					cout << Fensterdaten.ID->name << endl;
				return;
			}
		}
		/* Auswahl ist ein Exotic Spot */
		for (uint i = 0; i < EXOTICknoten.size(); i++)
		{
			if (compare(T0, &EXOTICknoten[i]))
			{
				Fensterdaten.ID = &EXOTICknoten[i];
				Fensterdaten.Arrayindex = i;
				if (Fensterdaten.ID != 0)
					cout << Fensterdaten.ID->name << endl;
				return;
			}
		}
		/* Auswahl ist ein Boss Spot */
		for (uint i = 0; i < BOSSEknoten.size(); i++)
		{
			if (compare(T0, &BOSSEknoten[i]))
			{
				Fensterdaten.ID = &BOSSEknoten[i];
				Fensterdaten.Arrayindex = i;
				if (Fensterdaten.ID != 0)
					cout << Fensterdaten.ID->name << endl;
				return;
			}
		}
	}
	/* Bereich, falls kein Objekt angewählt wurde*/
	if (Fensterdaten.ID != 0)
	{
		SQL.Daten_Updaten(Fensterdaten.ID);
	}		
	Fensterdaten.ID = 0;
	Fensterdaten.Arrayindex = 99999;
}

/* Verarbeiten der Tastertur und Mauseingaben - Abtrennung zum Rendering */
inline void processInput(GLFWwindow *window)
{
	// Die Default Steuerung zum Schließen des Fensters
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		ioservice.stop();
	}

	if(!Interface_Hovered)
	{
		/* Abschnitt der Kartnebewegung mit der Maus */
		if (Fensterdaten.Maus.Is_Links_Klickt && Fensterdaten.ID == 0 && !Interface_Hovered)
		{
			double N_X, N_Y, CX, CY;

			glfwGetCursorPos(window, &N_X, &N_Y);

			CX = N_X - Fensterdaten.Maus.X;

			CY = Fensterdaten.Maus.Y - N_Y;

			Cam.C.x += (CX / 100.0);
			if (Cam.C.x < -20.0)
				Cam.C.x = -20.0;
			if (Cam.C.x > 20.0)
				Cam.C.x = 20.0;

			Cam.C.y += (CY / 100.0);
			if (Cam.C.y < -20.0)
				Cam.C.y = -20.0;
			if (Cam.C.y > 20.0)
				Cam.C.y = 20.0;

		}
		else if (Fensterdaten.Maus.Is_Links_Klickt && Fensterdaten.ID != 0 && !Interface_Hovered)
		{
			double N_X, N_Y, CX, CY;

			glfwGetCursorPos(window, &N_X, &N_Y);

			CX = N_X - Fensterdaten.Maus.X;

			CY = Fensterdaten.Maus.Y - N_Y;

			Fensterdaten.ID->X += (CX / 100.0);
			if (Fensterdaten.ID->X < -20.0)
				Fensterdaten.ID->X = -20.0;
			if (Fensterdaten.ID->X > 20.0)
				Fensterdaten.ID->X = 20.0;

			Fensterdaten.ID->Y += (CY / 100.0);
			if (Fensterdaten.ID->Y < -20.0)
				Fensterdaten.ID->Y = -20.0;
			if (Fensterdaten.ID->Y > 20.0)
				Fensterdaten.ID->Y = 20.0;
		}

		glfwGetCursorPos(window, &Fensterdaten.Maus.X, &Fensterdaten.Maus.Y);
		Fensterdaten.Maus.NX = (Fensterdaten.Maus.X / (Fensterdaten.X * 0.5)) - 1.0;
		Fensterdaten.Maus.NY = (Fensterdaten.Maus.Y / (Fensterdaten.Y * 0.5)) - 1.0;

		// Die Steuerung um sich auf der Karte nach Oben zu bewegen.
		if (Fensterdaten.Maus.Is_Rechts_Klickt && !Statusanzeige)
		{
			Objektanwahl();
		}
		if (!Fensterdaten.Maus.Is_Rechts_Klickt)
		{
			Statusanzeige = false;
		}

		/* Die Tastertur-Steuerung um sich auf der Karte nach */
		{
			/* Oben zu bewegen. */
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				if (Cam.C.y > -20.0)
					Cam.C.y -= 0.001;
			}
			/* Unten zu bewegen. */
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				if (Cam.C.y < 20.0)
					Cam.C.y += 0.001;
			}
			/* Rechts zu bewegen. */
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			{
				if (Cam.C.x > -20.0)
					Cam.C.x -= 0.001;
			}
			/* Links zu bewegen. */
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				if (Cam.C.x < 20.0)
					Cam.C.x += 0.001;
			}
		}
	}
}

/* Funktion zum laden von Texturen - Returnwert die Texture - Clean*/
inline uint loadTexture(char const * path)
{
	uint temp;
	glGenTextures(1,&temp);
	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, temp);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);		

		stbi_image_free(data);
	};

	return temp;
};

/* Funktion um ein Hilfstext anzuzeigen - Clean (Weil Beispielfunktion)*/
inline void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
};

/* Funktion zur Konvertierung von "const char*" zu "char*" - Clean */
inline void Conv_Str_char(char * buf, string t)
{
	uint i;
	for (i = 0; i < t.size(); i++)
		buf[i] = t[i];
	while (buf[i] != 0)
	{
		buf[i] = ' ';
		i++;
	}
}

#define SYNCHRON_SERVER

bool THREAD_DONE = false;
bool socket_soffen = false;

class Netzwerk
{	
	/* Netzwerk System Komponente - Verarbeitung des IO ASIO System */
	boost::asio::io_service &io;

	/* Aufbau der Ip Auflösung */
	tcp::resolver resolv;

	/* 1. Schritt Portnummer des Systems */
	unsigned short port_num = 3333;
public:
	Netzwerk(io_service &io) : io(io), resolv(io)
	{

	};

	#ifdef SYNCHRON_SERVER
	void process(boost::asio::ip::tcp::socket & sock)
	{
		/* Datensatz */
		boost::asio::streambuf buf;
		/* Datenempfang */
		boost::asio::read_until(sock, buf, "\n");
		/* Daten Umwandeln in String*/
		std::string data = boost::asio::buffer_cast<const char*>(buf.data());
		/* Ausgabe in der Server Konsole*/
		std::cout << "Client's request is: " << data << std::endl;

		/* Datenerstellung */
		data = "Hello from server";

		std::stringstream response;
		response << "HTTP/1.1 200 OK" << HTML_END_COLUM;
		response << HTML_END_COLUM;
		response << "<html>" << HTML_END_COLUM;
		response << "<body>" << HTML_END_COLUM;
		response << "<h1>Hallo Welt </h1>" << HTML_END_COLUM;
		response << "</body>" << HTML_END_COLUM;
		response << "</html>" << HTML_END_COLUM;


		/* Daten schreiben */
		boost::asio::write(sock, boost::asio::buffer(response.str()));
		/* Ausgabe in der Server Konsole*/
		std::cout << "server sent data : " << response.str() << std::endl;

	}
	#endif

	void run()
	{
		try
		{
#ifdef SYNCHRON_SERVER
			// Anzahl der Verbindungen
			const int BACKLOG_SIZE = 30;

			// Step 2. Erstelle ein server endpoint.
			boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port_num);

			// Step 3. Erstelle und öffne ein Acceptor socket.
			boost::asio::ip::tcp::acceptor acceptor(ioservice, ep.protocol());
			// Step 4. Binde ein acceptor socket für einen Server Endpunkt.
			acceptor.bind(ep);
			// Step 5. Starte auf einkommende Verbindungabfragen zu hören.
			acceptor.listen(BACKLOG_SIZE);

			// Step 6. Erstelle ein aktiven Internet Socket.
			boost::asio::ip::tcp::socket sock(ioservice);
			// Step 7. Processing the next connection request and connecting the active socket to the client.
			acceptor.accept(sock);

			//all steps for creating socket using boost::asio are done.

			//Now perform read write operations in a function.
			process(sock);

			//tcp::acceptor acceptor(ioservice, tcp::endpoint(tcp::v4(), 13));

			for (;;)
			{
				tcp::socket socket(ioservice);
				acceptor.accept(socket);

				//std::string message = "hi";

				std::stringstream response;
				response << "HTTP/1.1 200 OK" << HTML_END_COLUM;
				response << HTML_END_COLUM;
				response << "<html>" << HTML_END_COLUM;
				response << "<body>" << HTML_END_COLUM;
				response << "<h1>Hallo Welt </h1>" << HTML_END_COLUM;
				response << "</body>" << HTML_END_COLUM;
				response << "</html>" << HTML_END_COLUM;

				boost::system::error_code ignored_error;
				boost::asio::write(socket, boost::asio::buffer(response.str()), ignored_error);

				cout << response.str() << endl;
			}
#endif

#ifdef SYNCHRON_CLIENT
			unsigned short port_num = 3333;

			tcp::resolver::query q{ "127.0.0.1", "3333" };

			// Erstellen des Sockets
			boost::asio::ip::tcp::socket socket(ioservice);

			// Verbindung erstellen
			boost::asio::ip::tcp::endpoint ep = *(resolv.resolve(q));//(boost::asio::ip::address_v4::any(), port_num);
			socket.connect(ep);

			// Nachricht die zum Server gesendet wird
			const std::string msg = "Hello from Client!\n";

			boost::system::error_code error;
			// Schreibe die Nachricht zum Socket
			boost::asio::write(socket, boost::asio::buffer(msg), error);
			if (!error)
			{
				/* Datensatz */
				boost::asio::streambuf buf;
				/* Datenempfang */
				boost::asio::read_until(socket, buf, "\n");
				/* Daten Umwandeln in String*/
				std::string data = boost::asio::buffer_cast<const char*>(buf.data());
				/* Ausgabe in der Server Konsole*/
				std::cout << "Server Response is is: " << data << std::endl;
			}
			else
			{
				std::cout << "send failed: " << error.message() << std::endl;
			}

			//  Erhalten der Antwort vom Server
			boost::asio::streambuf receive_buffer;
			boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
			if (error && error != boost::asio::error::eof)
			{
				std::cout << "receive failed: " << error.message() << std::endl;
			}
			else
			{
				const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
				std::cout << "Received " << data << std::endl;
			}

#endif			
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		catch (...)
		{
		}

	}
}Telegram(ioservice);



	/* 
		Zu Transferierende Informationen

			- Aktualliserungsaufruf Übermitteln

			- Sofern Unterschiedliche Datenbanken . Datenbank Verbindung zum Server Senden . Prüfen - Datenaustausch bei Neu - Update - Löschen

			- Chat Neue Nachrichten an alle Teilnehmer senden

	 */

void threads()
{	
	Telegram.run();
	THREAD_DONE = false;
}

int main()
{
	/* Initialisieren und Konfigurieren von GLFW */
	const char* glsl_version = "#version 130";
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Fenster Erstellung mithilfe von GLFW */
	GLFWwindow* window = glfwCreateWindow(Fensterdaten.getX(), Fensterdaten.getY(), "Interaktive Karte", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
		
	/* Setzten der Callback Funktionen, zum erhalten der Daten */
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
		
	/* Lade mithilfe von Glad alle Opengl Funktionszeiger */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}	
		

	/* Initialisieren der Schleifen/Programmnotwengien Variablen */
	FREETYPE Schrift;
	MAP Map;
	Objektmarker Entrace;   /* Marker für Höhleneingänge */
	Objektmarker Mission;   /* Marker für Missionen */
	Objektmarker Exospots;  /* Marker für Exospots */
	Objektmarker Bossspots; /* Marker für BossSpots */

	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("5.1.transform.vs", "5.1.transform.fs");

	// build and compile our shader zprogram
	// ------------------------------------
	Shader instanzer("MatrixShader.vs", "MatrixShader.fs");

	// compile and setup the shader
	// ----------------------------
	Shader shader("text.vs", "text.fs");

	Schrift.Initialisieren();

	//Setup Dear ImGui 
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	bool show_demo_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	Missionsliste = SQL.Daten_Lesen_Missionsname();
	Entrace.update(SQL.Hölenpositionen());
	Mission.update(vector<glm::vec2>(0));
	Exospots.update(vector<glm::vec2>(0));
	Bossspots.update(vector<glm::vec2>(0));
	
	texture1 = loadTexture("Resourcen/HQ_Map.png");
	texture2 = loadTexture("Resourcen/HQ_Map.png");

	Schwarzer_Tropfen = loadTexture("Resourcen/Schwarzer_Tropfen.tga");
	Roter_Tropfen = loadTexture("Resourcen/Roter_Tropfen.tga");
	Blauer_Tropfen = loadTexture("Resourcen/Blauer_Tropfen.tga");
	Gelber_Tropfen = loadTexture("Resourcen/Gelber_Tropfen.tga");
	

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use();
	ourShader.setInt("texture1", 0);
	ourShader.setInt("texture2", 1);
	
	float radians = 0.0;
	


	//t.join();

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		if(!THREAD_DONE)
		{
			THREAD_DONE = true;
			boost::thread t{ threads };
		}

		{
		/* Tastertur und Mauseingaben */
		processInput(window);
		
		/* Updaten der Zeichnungarrays */
		{
			vector<glm::vec2> Tempo;
			for (uint i = 0; i < Höhlenentraces.size(); i++)
				Tempo.push_back(Höhlenentraces[i]);
			Entrace.update(Tempo);

			Tempo.clear();
			for (uint i = 0; i < MISSIONSDATEN.size(); i++)
				Tempo.push_back(MISSIONSDATEN[i]);
			Mission.update(Tempo);

			Tempo.clear();
			for (uint i = 0; i < EXOTICknoten.size(); i++)
				Tempo.push_back(EXOTICknoten[i]);
			Exospots.update(Tempo);

			Tempo.clear();
			for (uint i = 0; i < BOSSEknoten.size(); i++)
				Tempo.push_back(BOSSEknoten[i]);
			Bossspots.update(Tempo);
		}
			
		Cam.view = glm::translate(Cam.view, glm::vec3(0.0f, 0.0f, -3.0f));
		Cam.projection = glm::perspective(glm::radians(45.0f), Fensterdaten.getX() / Fensterdaten.getY(), 0.1f, 100.0f);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		/* Dies ist die Ausführung der ImGui::ShowDemoWindow - Dies dient als Dokumentation mithilfe von Beispiele */
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		/* Nütze Beginn und End wie eine Klammer und führe innerhalb das selbst erstellte Fenster aus - mit dem ImGui Namespace*/
		{
			ImGui::Begin("Interface Interaktive Map");                         /* Das Klammer aus für das eigene Interface mit Namensübergabe */
			Interface_Hovered = ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsItemHovered() || ImGui::IsAnyItemActive();// || ImGui::IsMouseHoveringRect();
			
			/* 
				Einloggen - 1. Feld  Benutzername  2.Feld  Passwort   3. Feld Login
				Login anklicken
				try Select SQL - Ob der Name existent ist
				Wenn Ja Vergleichen der Passwörter
				Wenn Ja Setze eingeloggt auf true
			*/
			if (loginlvl < 0)
			{
				static char BENUTZERNAMEN[128] = "Benutzername";
				static char PASSWORT[128] = "Passwort";
				//static int Benutzerlevel = 2;

				ImGui::InputText("Benutzername", BENUTZERNAMEN, IM_ARRAYSIZE(BENUTZERNAMEN));							HILFSMARKER
				ImGui::InputText("Passwort", PASSWORT, IM_ARRAYSIZE(PASSWORT), ImGuiInputTextFlags_Password);			HILFSMARKER
				//ImGui::InputInt("Anzahl", &Benutzerlevel);

				if (ImGui::Button("Einloggen"))
				{
					pair<string, int> temp;
					/* SQL Suche namen in Datenbank */
					temp = SQL.Daten_Lesen(BENUTZERNAMEN);

					/* Prüfen ob Benutzerlevel nicht -2 ist */
					/* Passwort vergleichen */
					if (temp.second != -2)
					{
						if (PasswortVerschlüsseln(PASSWORT) == temp.first)
						{
							loginlvl = temp.second;
						}
					}
				}ImGui::SameLine();				
				/**if (ImGui::Button("Updaten"))
				{
					pair<string, int> temp;
					/* SQL Suche namen in Datenbank *
					temp = SQL.Daten_Lesen(BENUTZERNAMEN);

					/* Prüfen ob Benutzerlevel nicht -2 ist *
					/* Passwort vergleichen *
					if (temp.second != -2)
					{
						SQL.Daten_Updaten(BENUTZERNAMEN, PASSWORT, 2);
					}
				}ImGui::SameLine();	/**/
				/**/if (ImGui::Button("Registieren"))
				{
					pair<string, int> temp;
					/* SQL Suche namen in Datenbank */
					temp = SQL.Daten_Lesen(BENUTZERNAMEN);

					/* Prüfen ob Benutzerlevel nicht -2 ist */
					/* Passwort vergleichen */
					if (temp.second == -2)
					{
						SQL.Daten_Einfügen(BENUTZERNAMEN, PASSWORT, 1);
					}
				}/**/
			}
			else if (loginlvl >= 0)
			{
				if (ImGui::Button("Ausloggen"))
				{
					loginlvl = -1;
				}
			}

			if (Mission_Ausgewählt)
			{
				if (ImGui::ArrowButton("##left", ImGuiDir_Left)) //ImGui::SameLine(0.0f, spacing);
				{
					Missionsliste = SQL.Daten_Lesen_Missionsname();
					MISSIONSDATEN.clear();
					EXOTICknoten.clear();
					BOSSEknoten.clear();
					Mission_Ausgewählt = false;
				}
			}

			/* Ausgabe des Angewählten Zieles */
			if (Fensterdaten.ID != 0)
			{
				/* Ausgabe Deklarieren */
				std::ostringstream ss;	

				if (Fensterdaten.ID->Index == 1)
				{
					ss << "Angewaehlt ist der Punkt " << endl << Fensterdaten.ID->X << "," << Fensterdaten.ID->Y << endl;
					ss << "Dies ist aus die Aufgabe " << Fensterdaten.ID->Zusatz;
				}					
				else if (Fensterdaten.ID->Index == 2)
				{
					ss << "Angewaehlt ist der Exoticspot " << endl << Fensterdaten.ID->X << "," << Fensterdaten.ID->Y << endl;
				}
				else if (Fensterdaten.ID->Index == 3)
				{
					ss << "Angewaehlt ist der Bossspot " << endl << Fensterdaten.ID->X << "," << Fensterdaten.ID->Y << endl;
				}
				else
				{
					ss << "Angewaehlt ist die Hoehle '" << Fensterdaten.ID->name << "'" << endl << "bei " << Fensterdaten.ID->X << "," << Fensterdaten.ID->Y << endl;
				}
				
				/* Ausgabe anhand des Index */
				ImGui::Text(ss.str().c_str());
			}

			/* Ausgabe der Frame Zeit */
			ImGui::Text("Durchschnittliche Framezeit:\n%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			
			/* Ansteuerung der Auswahl - Unterscheidung zwischen Knoten und Missionen */
			if (Mission_Ausgewählt)
			{
				ostringstream ss; 
				ss << MISSIONSDATEN[0].getName() << ": " << MISSIONSDATEN[0].getTyp();
				ImGui::Text(ss.str().c_str()); ImGui::Text("Waehle das Ziel aus: ");
			}
			if (!Mission_Ausgewählt){ImGui::Text("Waehle die Mission aus: ");}

			float x = Höhlenentraces[0].X;
			x = Höhlenentraces[1].X;
			x = Höhlenentraces[2].X;
			x = Höhlenentraces[3].X;

			/* Zur Steuerung von Update, Löschen und Hinzufügen muss man eingeloggt sein */
			if (loginlvl >= 0)
			{
				/* Hinzufügen */
				{
					/* Das Einfügen wird mithilfe einer Auswahlbox durchgeführt, welche Abhängig der stands angepasst wird */
					static int Einfügen_current = 0;
					/* Auswahl zwischen Mission angewählt oder nicht */
					if (!Mission_Ausgewählt)
					{
						const char* Einfügen[] = { "Nichts einfuegen", "Hoehle einfuegen", "Mission einfuegen" };
						ImGui::Combo("combo", &Einfügen_current, Einfügen, IM_ARRAYSIZE(Einfügen));
					}
					else
					{
						const char* Einfügen[] = { "Nichts einfuegen", "Hoehle einfuegen", "Mission Ziel einfuegen", "Exo Spot einfuegen", "Boss Spot einfuegen" };
						ImGui::Combo("combo", &Einfügen_current, Einfügen, IM_ARRAYSIZE(Einfügen));
					};


					/* Hoehle einfuegen */
					if (Einfügen_current == 1)
					{
						static char name[128] = "Name der Hoehle";
						static char Grundriss[128] = "Kleine Pilzhoehle";
						static int ic = 2;

						ImGui::InputText("Name der Hoehle", name, IM_ARRAYSIZE(name));									HILFSMARKER
						ImGui::InputText("Grundriss", Grundriss, IM_ARRAYSIZE(Grundriss));								HILFSMARKER
						ImGui::InputInt("Anzahl", &ic);

						if (ImGui::Button("Einfuegen"))
						{
							Höhlenentraces.push_back(Höhlendaten(name, (-1 * Cam.C.x), (-1 * Cam.C.y), Grundriss, ic));
							SQL.Daten_Einfügen(name, (-1 * Cam.C.x), (-1 * Cam.C.y), Grundriss, ic);																Einfügen_current = false;
						}
					}
					/* Mission einfuegen - Einzugebende Parameter: Name,Typ und Ren | Fertige Parameter: Beschreibung, Koordinaten, Knotentyp */
					else if (Einfügen_current == 2 && !Mission_Ausgewählt)
					{
						static char Missionname[128] = "Missionsname";
						static char Missionstyp[128] = "Missionstyp";
						static int ren = 123;

						ImGui::InputText("Missionsname", Missionname, IM_ARRAYSIZE(Missionname));						HILFSMARKER
						ImGui::InputText("Missionstyp", Missionstyp, IM_ARRAYSIZE(Missionstyp));						HILFSMARKER
						ImGui::InputInt("Ren", &ren);

						if (ImGui::Button("Einfuegen"))
						{
							Missionsliste.push_back({ Missionname, Missionstyp });
							SQL.Daten_Einfügen((-1 * Cam.C.x), (-1 * Cam.C.y), Missionname, Missionstyp, "Ladnung", ren);											Einfügen_current = false;
						}
					}
					/* Missions Ziel einfuegen  - Einzugebende Parameter: Beschreibung | Fertige Parameter: Name, Typ, Ren, Koordinaten, Knotentyp */
					else if (Einfügen_current == 2 && Mission_Ausgewählt)
					{
						static char Missionsbeschreibung[512] = "Zielbeschreibung";
						ImGui::InputText("Zielbeschreibung", Missionsbeschreibung, IM_ARRAYSIZE(Missionsbeschreibung));	HILFSMARKER

						if (ImGui::Button("Einfuegen"))
						{
							MISSIONSDATEN.push_back(Missionsdaten((-1 * Cam.C.x), (-1 * Cam.C.y), MISSIONSDATEN[0].getName(), MISSIONSDATEN[0].getTyp(), string(Missionsbeschreibung), 0));
							SQL.Daten_Einfügen((-1 * Cam.C.x), (-1 * Cam.C.y), MISSIONSDATEN[0].getName(), MISSIONSDATEN[0].getTyp(), Missionsbeschreibung);		Einfügen_current = false;
						}
					}
					/* Exo Spot einfuegen */
					else if (Einfügen_current == 3 && Mission_Ausgewählt)
					{
						if (ImGui::Button("Einfuegen"))
						{
							ostringstream ss;
							ss << "Exotic" << (EXOTICknoten.size() + 1);

							EXOTICknoten.push_back(BASISATTRIBUTE((-1 * Cam.C.x), (-1 * Cam.C.y), 2, MISSIONSDATEN[0].getName(), ss.str()));
							SQL.Daten_Einfügen((-1 * Cam.C.x), (-1 * Cam.C.y), MISSIONSDATEN[0].getName(), MISSIONSDATEN[0].getTyp(), ss.str(), 0, 1);				Einfügen_current = false;
						}
					}
					/* Boss Spot einfuegen */
					else if (Einfügen_current == 4 && Mission_Ausgewählt)
					{
						static char Missionsbeschreibung[128] = "Bossname";
						ImGui::InputText("Bossname", Missionsbeschreibung, IM_ARRAYSIZE(Missionsbeschreibung));			HILFSMARKER

						if (ImGui::Button("Einfuegen"))
						{
							BOSSEknoten.push_back(BASISATTRIBUTE((-1 * Cam.C.x), (-1 * Cam.C.y), 3, MISSIONSDATEN[0].getName(), Missionsbeschreibung));
							SQL.Daten_Einfügen((-1 * Cam.C.x), (-1 * Cam.C.y), MISSIONSDATEN[0].getName(), MISSIONSDATEN[0].getTyp(), Missionsbeschreibung, 0, 2);	Einfügen_current = false;
						}
					};
				}

				/* Updaten */
				{
				/* 
					Zum Updaten wird eine Objektanwahl benötigt - Abhängig vom Objekt kann jeder Parameter angepasst werden 
					- Das Verschieben darf auch nur währen eingeloggt sein passieren
				*/
					if (Fensterdaten.ID != 0 || Mission_Ausgewählt)
					{
						/* Damit soll das Updaten "aktiviert" werden - damit nicht ausversehen etwas verändert wird */
						if (ImGui::Button("SQL Updaten"))
						{
							if (!Updaten)
								Reset = true;
							Updaten = !Updaten;
						}
						if (Updaten)
						{
							static char Name[128], Typ[128], Beschreibung[512];
							static float px, py;
							static int ren;

							/* Updaten von Mission Information - es ist der Name, der Typ, die Positionierung und die Renmenge veränderbar */
							if (Fensterdaten.ID == 0)
							{
								if (Reset)
								{
									Conv_Str_char(Name, MISSIONSDATEN[0].getName());
									Conv_Str_char(Typ, MISSIONSDATEN[0].getTyp());
									Conv_Str_char(Beschreibung, MISSIONSDATEN[0].getBeschreibung());
									px = MISSIONSDATEN[0].getX();
py = MISSIONSDATEN[0].getY();
ren = MISSIONSDATEN[0].getRen();
Reset = false;
								}

								ImGui::InputText("Missionsname", Name, IM_ARRAYSIZE(Name));		HILFSMARKER
									ImGui::InputText("Missionstyp", Typ, IM_ARRAYSIZE(Typ));		HILFSMARKER
									ImGui::InputFloat("X Position", &px);
								ImGui::InputFloat("Y Position", &py);
								ImGui::InputInt("Ren", &ren);
							}
							/* Updaten von Höhleneingang - es ist der Name, die Positionierung und die Form */
							else if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 0)
							{
							if (Reset)
							{
								Conv_Str_char(Name, Fensterdaten.ID->getName());
								Conv_Str_char(Typ, Fensterdaten.ID->Zusatz);
								px = Fensterdaten.ID->getX();
								py = Fensterdaten.ID->getY();
								Reset = false;
							}

							ImGui::InputText("Höhlennamen", Name, IM_ARRAYSIZE(Name));		HILFSMARKER
								ImGui::InputText("Höhlenform", Typ, IM_ARRAYSIZE(Typ));			HILFSMARKER
								ImGui::InputFloat("X Position", &px);
							ImGui::InputFloat("Y Position", &py);
							}
							/* Updaten von Missionsziel  - Anzupassen ist die Beschreibung und die Positionierung */
							else if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 1)
							{
							if (Reset)
							{
								Conv_Str_char(Beschreibung, Fensterdaten.ID->Zusatz);
								px = Fensterdaten.ID->getX();
								py = Fensterdaten.ID->getY();
								Reset = false;
							}
							ImGui::InputText("Beschreibung", Beschreibung, IM_ARRAYSIZE(Beschreibung));		HILFSMARKER
								ImGui::InputFloat("X Position", &px);
							ImGui::InputFloat("Y Position", &py);
							}
							/* Updaten von Exotic Spot  - Anzupassen ist die Positionierung */
							else if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 2)
							{
							if (Reset)
							{
								px = Fensterdaten.ID->getX();
								py = Fensterdaten.ID->getY();
								Reset = false;
							}
							ImGui::InputFloat("X Position", &px);
							ImGui::InputFloat("Y Position", &py);
							}
							/* Updaten von Boss Spot  - Anpassen des Namens und der Positionierung */
							else if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 3)
							{
							if (Reset)
							{
								Conv_Str_char(Beschreibung, Fensterdaten.ID->Zusatz);
								px = Fensterdaten.ID->getX();
								py = Fensterdaten.ID->getY();
								Reset = false;
							}
							ImGui::InputText("Beschreibung", Beschreibung, IM_ARRAYSIZE(Beschreibung));		HILFSMARKER
								ImGui::InputFloat("X Position", &px);
							ImGui::InputFloat("Y Position", &py);
							}

							if (ImGui::Button("Updaten"))
							{
								/*
									Übergebe Abhängig von den zu Übertragenden Werte
									0 -> Ist die Aktualisierung der Mission Namen -
									Achtung:
										Namens Änderungen beziehen sich auf "alle" einträge
										Ren und Positionänderungen beziehen sich auf "Landung"

									1 -> Höhlendaten  - Dieser Eintrag sollte "nur" einmalig existieren,				wodurch keine Besonderheit beachtet werden muss
									2 -> Missionsziel - Dieser Eintrag sollte "nur" einmalig exisiteren,				wodurch keine Besonderheit beachtet werden muss
									3 -> Exotic Spot  - Dieser Eintrag ist mithilfe von Beschreibung nur "einmalig",	wodurch keine Besonderheit beachtet werden muss
									4 -> Boss Spot	  - Dieser Eintrag sollte "nur" einmalig existieren, 				wodurch keine Besonderheit beachtet werden muss
								*/
								if (Fensterdaten.ID == 0)
									SQL.Daten_Updaten(0, Name, Typ, Beschreibung, px, py, ren, Fensterdaten.ID);
								if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 0)
									SQL.Daten_Updaten(Name, Typ, px, py, Fensterdaten.ID);
								if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 1)
									SQL.Daten_Updaten(2, Name, Typ, Beschreibung, px, py, ren, Fensterdaten.ID);
								if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 2)
									SQL.Daten_Updaten(3, Name, Typ, Beschreibung, px, py, ren, Fensterdaten.ID);
								if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 3)
									SQL.Daten_Updaten(4, Name, Typ, Beschreibung, px, py, ren, Fensterdaten.ID);
								Updaten = false;
							}
						}
					}
				}

				/* Löschen */
				{
					if (ImGui::Button("Auswahl Loeschen?"))
						Löschen = !Löschen;
					if (Löschen)
						{
							if (ImGui::Button("Sicher?"))
							{
								if (Fensterdaten.ID == 0)
									SQL.Daten_Löschen(MISSIONSDATEN[0].name, MISSIONSDATEN[0].Missionstyp, 0);
								else if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 0)
									SQL.Daten_Löschen(Fensterdaten.ID->name, "", 1);
								else if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 1)
									SQL.Daten_Löschen(Fensterdaten.ID->name, Fensterdaten.ID->Zusatz, 2);
								else if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 2)
									SQL.Daten_Löschen(Fensterdaten.ID->name, Fensterdaten.ID->Zusatz, 3);
								else if (Fensterdaten.ID != 0 && Fensterdaten.ID->Index == 3)
									SQL.Daten_Löschen(Fensterdaten.ID->name, Fensterdaten.ID->Zusatz, 4);
								Löschen = false;
							}
						}
				}
			}
				
			// Liste der Missionen
			if (!Mission_Ausgewählt)
			{
				ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
				ImGui::BeginChild("ChildR", ImVec2(0, 260), true, window_flags);
				if (ImGui::BeginTable("split", 1, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
				{
					for (int i = 0; i < Missionsliste.size(); i++)
					{
						ImGui::TableNextColumn();
						string combi = Missionsliste[i].first + " " + Missionsliste[i].second;
						if (ImGui::Button(combi.c_str(), ImVec2(-FLT_MIN, 0.0f)))
						{
							MISSIONSDATEN = SQL.Daten_Lesen(Missionsliste[i]);
							vector<glm::vec2> temp;
							for (uint i = 0; i < MISSIONSDATEN.size(); i++)
								temp.push_back(MISSIONSDATEN[i]);
							Mission.update(temp);
							Mission_Ausgewählt = true;
						}
					}
					ImGui::EndTable();
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
			}
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		}

		/* Standard Renderroutine */
		{
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			Map.Render(&ourShader,&texture1,&texture2);
			Entrace.Render(&instanzer, Blauer_Tropfen, Blauer_Tropfen);
			Mission.Render(&instanzer, Gelber_Tropfen, Gelber_Tropfen);
			Exospots.Render(&instanzer, Schwarzer_Tropfen, Schwarzer_Tropfen);
			Bossspots.Render(&instanzer, Roter_Tropfen, Roter_Tropfen);
		}

		/* Renderroutine von Zusätzlichen Texten */
		{
			Schrift.aktivieren();

			Schrift.RenderText(shader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
			Schrift.RenderText(shader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

			Schrift.deaktivieren();

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}	

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}