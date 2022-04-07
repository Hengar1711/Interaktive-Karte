/* 
	Liezensierungen

		<a href="https://de.vecteezy.com/gratis-vektor/tropfen">Tropfen Vektoren von Vecteezy</a>
*/

#ifndef INCLUDES
	#define INCLUDES
	#include <stdlib.h>
	#include <iostream>
	#include <sstream>
	#include <stdexcept>

	#include <iostream>
	#include <map>
	#include <string>
	#include <vector>

	#include <glad/glad.h>
	#include <GLFW/glfw3.h>

	#include <stb_image.h>

	#include <glm/glm.hpp>
	#include <glm/gtc/matrix_transform.hpp>
	#include <glm/gtc/type_ptr.hpp>

	#include <ft2build.h>
	#include FT_FREETYPE_H

	#include "Shader.h"
	#include "lodepng.h"

	#include <iostream>

	#include "imgui\imgui.h"
	#include "imgui\imgui_impl_glfw.h"
	#include "imgui\imgui_impl_opengl3.h"

	#include "mysql_connection.h"
	#include <cppconn/driver.h>
	#include <cppconn/exception.h>
	#include <cppconn/prepared_statement.h>

	using namespace std;
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

/*	WIKI
	Problem Stellung: 
			- Die String werden beim Dritten %s nicht Ordnungsgem�� an die Konsole �bergeben. 
			M�gliche Idee, Pr�fen der Umgebungsvariablen der Konsolen
			L�SUNG: Achtung es scheint das System zu �berladen wenn 3 strings in Folge an die Konsole �bergeben werden, anders Trennen der string �bertragung

			Zuweisen der Ausw�hlbaren Missionen in eine Map legen, um diese im Auswahlfenster nur einmal zu haben.
			L�SUNG: Mithilfe von DISTINCT wird in SQL bereits gefiltert.

			Erstellen der Karte in einem Bereich "au�erhalb der Kamera" 
			Mapping eines 2ten Screen Bereichs aus das gezeichnete Viereck.
			Zeichnen der Markierungen mithilfe von Alpha Kanal und kleinen Vierecken wo die "Spitze" (Untere Mitte) auf dem Zielpunkt zeigt.
			Aktueller Weg: Zeichne die Plane als Semi "3d Objekt" - Deaktivere die Kamera Drehung - Begrenze die Bewegung auf die entsprechenden Kartenr�nder
			Erstellen der Hintergrund Karte: Done
			Die Grafik mit Alpha Kanal laden: Done
			Die 2D Karte als "semi 3D" erstellen: Done
			die bewegung auf den kartenbereich beschr�nken: Done
			Kleine Markierungssteine mit Tropfen erstellen: Done
			Bewegung der Kleinen Margierungsteine angleichen: Done | Es war eine Optische T�uschung der durch den z Versatz entstanden ist. (Verk�rzung von z zur Plane, zum Reduzieren der T�uschung)
			Diese Markierungsteine Verteilen: Done
			Die markierungsteine mithilfe der datenbankpositionen laden: Done
			Die markierungsteine im fenster verschieben k�nnen: offen - Auswahlfunktion mithilfe Raycast und vergleich bzw da die Karte "nur" 2D ist - Die Position der Karte + die Position der Maus umgerechnet mit der Position des Objektes vergleichen
			neue markeirungsteine setzten im fenster: offen



*/
class Koordinaten
{
	
public:
	float X, Y;
	Koordinaten() {};
	Koordinaten(float X, float Y) : X(X), Y(Y) {};
	Koordinaten operator() (float X, float Y) { this->X = X; this->Y = Y; return *this; }
	float getX() { return X; };
	float getY() { return Y; };
};

class MAUS
{
public:
	bool Is_Links_Klickt, Is_Rechts_Klickt;
	int Action;
	double X, Y, Z;
	float NX, NY;
};



class Simple_Cam
{
public:
	glm::vec3 C;
	glm::mat4 view = glm::mat4(1.0f); 
	glm::mat4 projection = glm::mat4(1.0f);
} Cam;

class H�hlendaten : public Koordinaten
{

public:
	string name;
	string Grundriss;
	int ErzknotenAnzahl;
	H�hlendaten(string name, int X, int Y, string Grundriss, int Anzahl) : name(name), Koordinaten(X, Y), Grundriss(Grundriss), ErzknotenAnzahl(Anzahl)
	{};
	H�hlendaten(Koordinaten cords, string Grundriss, int Anzahl) : Koordinaten(cords.getX(), cords.getY()), Grundriss(Grundriss), ErzknotenAnzahl(Anzahl)
	{};

	operator Koordinaten()
	{
		return Koordinaten(this->getX(), this->getY());
	}
	operator glm::vec2()
	{
		return glm::vec2(X, Y);
	}
};

class window: public Koordinaten
{
public:
	MAUS Maus;

	glm::vec3 MAUS()
	{
		return glm::vec3(Maus.X, Maus.Y, Maus.Z);
	}
	window() : Koordinaten(800, 600) {};
	glm::vec4 viewport = { 0,0,X,Y };
	H�hlendaten *ID;

	void operator() (int X, int Y) 
	{
		this->X = X; 
		this->Y = Y; 
		viewport = { 0,Y,X,-Y };
	}	
};

/* Die Koordinaten sollten f�r den Missionsbeginn beim Landungschiff stehen */
class Missionsdaten : public Koordinaten
{
	string Missionsname;
	string Missionstyp;

	int Belohnung_Ren;
public:
	string Missionsbeschreibung;
	Missionsdaten(int X, int Y, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren) : Koordinaten(X, Y),
		Missionsname(Missionsname), Missionsbeschreibung(Missionsbeschreibung), Missionstyp(Missionstyp), Belohnung_Ren(Ren)
	{};
	Missionsdaten(Koordinaten cords, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren) : Koordinaten(cords.getX(), cords.getY()),
		Missionsname(Missionsname), Missionsbeschreibung(Missionsbeschreibung), Missionstyp(Missionstyp), Belohnung_Ren(Ren)
	{};

	operator Koordinaten()
	{
		return Koordinaten(this->getX(), this->getY());
	}

	string getName()
	{
		return Missionsname;
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
};

vector<H�hlendaten> H�hlenentraces;
vector<Missionsdaten> MISSIONSDATEN;

string Testausgabe;

#ifndef DEFINITIONEN
	#define DEFINITIONEN
	typedef unsigned int uint;

	#define MISSIONSNAME string("Missionsname")
	#define MISSIONSTYP string("Missionstyp")
	#define POSITIONX string("Position_X")
	#define POSITIONY string("Position_Y")
	#define MISSIONSBESCHREIBUNG string("Missionsbeschreibung")
	#define REN string("Ren")
	#define EXOTICS string("Exotics")
	#define GRUNDRISS string("Grundriss")
	#define ERZDEPOSITS string("Erzdeposits")
	#define FUNKTIONSLOS string("Funktionslos")

	#define SQL_EINF�GEN_IN string("INSERT INTO ")
	#define SQL_W�HLE_ALLE_VON string("SELECT * FROM ")
	#define SQL_W�HLE_EINZIGARTIGE string("SELECT DISTINCT " + MISSIONSNAME + ", " + MISSIONSTYP + " FROM ")
	#define SQL_WO string(" WHERE ")

	#define CAVEENTRACE string("`caveentrace`")
	#define INVENTORY string("inventory")
	#define MISSIONEN string("`mission`")
	
#endif

#ifndef BASICDECLARATIONEN
#define BASICDECLARATIONEN

	//for demonstration only. never save your password in the code!
	const std::string server = "tcp://127.0.0.1:3306";
	const std::string username = "Hengar";
	const std::string password = "Affenbrot12";

	/// Holds all state information relevant to a character as loaded using FreeType
	struct Character
	{
		unsigned int TextureID; // ID handle of the glyph texture
		glm::ivec2   Size;      // Size of glyph
		glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
		unsigned int Advance;   // Horizontal offset to advance to next glyph
	};
	map<GLchar, Character> Characters;
	
	window Fensterdaten;
	
	vector<pair<string,string>> Missionsliste;
	string Missionsnamensuche;	

	uint   Roter_Tropfen, Blauer_Tropfen, Gelber_Tropfen, Schwarzer_Tropfen;

#endif

class PRIVAT_MYSQL
{
	sql::Driver *driver;
	sql::Connection *con;
	sql::PreparedStatement *pstmt;
	sql::PreparedStatement *Daten_Einf�gen_H�hle;
	sql::PreparedStatement *Daten_Einf�gen_Mission;

	sql::PreparedStatement *Alle_Daten_Lesen_H�hle;
	sql::PreparedStatement *Alle_Daten_Lesen_Mission;

	sql::PreparedStatement *Alle_Daten_Lesen_Mission_Einmalig;
	sql::PreparedStatement *Einzel_Daten_Lesen_Mission;

	sql::Statement *stmt;

	sql::ResultSet *result;

public:
	// Der Normalesierte Verbindungsaufbau des Systems 
	// TODO Erweiterung: Die Verbindungsdaten sp�ter �ber die GUI eintragen. Und erst dann die Verbindung herstellen.
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

		Preparestatments();
	}
	/* Aufr�umen der Zeigerarythmetic (Ohne Smart Pointer = Speicher�berlauf "vorprogrammiert") */
	~PRIVAT_MYSQL()
	{
		//if (driver != 0)	delete driver;
		if (Daten_Einf�gen_H�hle != 0)					delete Daten_Einf�gen_H�hle;
		if (Daten_Einf�gen_Mission != 0)				delete Daten_Einf�gen_Mission;

		if (Alle_Daten_Lesen_H�hle != 0)				delete Alle_Daten_Lesen_H�hle;
		if (Alle_Daten_Lesen_Mission != 0)				delete Alle_Daten_Lesen_Mission;
		if (Alle_Daten_Lesen_Mission_Einmalig != 0)		delete Alle_Daten_Lesen_Mission_Einmalig;
		if (Einzel_Daten_Lesen_Mission != 0)			delete Einzel_Daten_Lesen_Mission;

		//if (pstmt != 0)		delete pstmt;
		if (con != 0)									delete con;
		if (result != 0)								delete result;
	}

	void Preparestatments()
	{
		Daten_Einf�gen_H�hle = con->prepareStatement(SQL_EINF�GEN_IN + CAVEENTRACE + "(Name, Position_X, Position_Y, Grundriss, Erzdeposits) VALUES(?,?,?,?,?);");
		Daten_Einf�gen_Mission = con->prepareStatement(SQL_EINF�GEN_IN + MISSIONEN + "(Position_X, Position_Y, Missionsname, Missionstyp, Missionsbeschreibung, Ren) VALUES(?,?,?,?,?,?);");

		Alle_Daten_Lesen_H�hle = con->prepareStatement(SQL_W�HLE_ALLE_VON + CAVEENTRACE + ";");
		Alle_Daten_Lesen_Mission = con->prepareStatement(SQL_W�HLE_ALLE_VON + MISSIONEN + ";");

		Alle_Daten_Lesen_Mission_Einmalig = con->prepareStatement(SQL_W�HLE_EINZIGARTIGE + MISSIONEN + ";");
	}

	/* Funktion zum Einf�gen der H�hlen Daten - Speicher der Position x + y, der Grundriss Datei und der Typischen Erz Knoten Anzahl*/
	void Daten_Einf�gen(string name, int X, int Y, string Grundriss, int Anzahl)
	{
		Daten_Einf�gen_H�hle->setString(1, name);
		Daten_Einf�gen_H�hle->setInt(2, X);
		Daten_Einf�gen_H�hle->setInt(3, Y);
		Daten_Einf�gen_H�hle->setString(4, Grundriss);
		Daten_Einf�gen_H�hle->setInt(5, Anzahl);

		Daten_Einf�gen_H�hle->execute();
		cout << "One row in " + CAVEENTRACE + " inserted." << endl;
	}
	/* Funktion zum Einf�gen der Missiontexte - Es wird ein X/Y gespeichert um M�gliche unterschiedliche Missionsziele zu speichern */
	void Daten_Einf�gen(int X, int Y, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren = 0)
	{
		Daten_Einf�gen_Mission->setInt(1, X);
		Daten_Einf�gen_Mission->setInt(2, Y);
		Daten_Einf�gen_Mission->setString(3, Missionsname);
		Daten_Einf�gen_Mission->setString(4, Missionstyp);
		Daten_Einf�gen_Mission->setString(5, Missionsbeschreibung);
		Daten_Einf�gen_Mission->setInt(6, Ren);

		Daten_Einf�gen_Mission->execute();
		cout << "One row in " + MISSIONEN + " inserted." << endl;
	}

	/* Vorgefertigte Konfiguration */
	void Vorgefertigte_Daten_Einlesen()
	{
		Daten_Einf�gen("B1", 2, 2, "Kleine", 10);
		Daten_Einf�gen("C10", 4, 4, "Kleine", 10);
		Daten_Einf�gen("E7", 1, 1, "Kleine", 10);
		Daten_Einf�gen("G7South", 7, 7, "Kleine", 10);
		Daten_Einf�gen("G7North", 9, 9, "Kleine", 10);
		Daten_Einf�gen("H7", 6, 6, "Kleine", 10);

		Daten_Einf�gen(1, 1, "Brueckenkopf", "Aufklaerung", "Aufklaerung Waldzone - Landung");

		Daten_Einf�gen(1, 1, "Livewire", "Gelaende Scan", "Landung", 67 /* Versicherung  Zwangsaktiv*/);
		Daten_Einf�gen(1, 1, "Livewire", "Gelaende Scan", "Scanne Ort 1");
		Daten_Einf�gen(1, 1, "Livewire", "Gelaende Scan", "Scanne Ort 2");
		Daten_Einf�gen(1, 1, "Livewire", "Gelaende Scan", "Scanne Ort 3");

		Daten_Einf�gen(1, 1, "Grabstein", "Geo Forschung", "Landung", 75);
		Daten_Einf�gen(1, 1, "Grabstein", "Geo Forschung", "Setzte die Geostation am Standort Alpha.");
		Daten_Einf�gen(1, 1, "Grabstein", "Geo Forschung", "Setzte die Geostation am Standort Beta.");
		Daten_Einf�gen(1, 1, "Grabstein", "Geo Forschung", "Setzte die Geostation am Standort Gamma.");
		Daten_Einf�gen(1, 1, "Grabstein", "Geo Forschung", "Setzte das Uplink am Standort Delta ein.");

		Daten_Einf�gen(1, 1, "Argos", "Erkundung", "Erkunde Icarus - Landung");

		Daten_Einf�gen(1, 1, "Landwirtschaft", "Vorratslager", "Vorraete Wald", 250);

		Daten_Einf�gen(1, 1, "Seltsame Ernte", "Bio Forschung", "Bio Forschung: Waldbiom", 50);

		Daten_Einf�gen(1, 1, "Todesliste", "Vernichtung", "Landung", 125);
		Daten_Einf�gen(1, 1, "Todesliste", "Vernichtung", "Folge der Spur des Raubtiers");
		Daten_Einf�gen(1, 1, "Todesliste", "Vernichtung", "Toete das Raubtier");

		Daten_Einf�gen(1, 1, "Probelauf", "Expedition", "Expedition Canyons", 125);

		Daten_Einf�gen(1, 1, "Todesstrahl", "Scan", "Landung", 150);
		Daten_Einf�gen(1, 1, "Todesstrahl", "Scan", "Scanne Ort 1");
		Daten_Einf�gen(1, 1, "Todesstrahl", "Scan", "Scanne Ort 2");
		Daten_Einf�gen(1, 1, "Todesstrahl", "Scan", "Scanne Ort 3");

		Daten_Einf�gen(1, 1, "Payramide", "Aufbau", "Landung", 175);
		Daten_Einf�gen(1, 1, "Payramide", "Aufbau", "Erreiche den Bauplatz.");
		Daten_Einf�gen(1, 1, "Payramide", "Aufbau", "Errichte ein Jagdaussenposten.");

		Daten_Einf�gen(1, 1, "Sandige Bruecken", "Verlaengerte Untersuchung", "Fuehre eine Langzeit Untersuchung durch.", 300);

		Daten_Einf�gen(1, 1, "Sandsturm", "Erkundung", "Erkunde Icarus.");

		Daten_Einf�gen(1, 1, "Wasserwaage", "Untersuchung", "Landung", 150);
		Daten_Einf�gen(1, 1, "Wasserwaage", "Untersuchung", "Uebertrage Geodaten vom Standort Alpha.");
		Daten_Einf�gen(1, 1, "Wasserwaage", "Untersuchung", "Uebertrage Geodaten vom Standort Zulu.");

		Daten_Einf�gen(1, 1, "Preservation", "Stockpile", "Landung", 350);

		Daten_Einf�gen(1, 1, "Feld Test", "Erholung", "Landung", 150);
		Daten_Einf�gen(1, 1, "Feld Test", "Erholung", "Sammle die verlorene gegangenen Komponenten des Prototyps.");
		Daten_Einf�gen(1, 1, "Feld Test", "Erholung", "Untersuche Absturzstelle Alpha");
		Daten_Einf�gen(1, 1, "Feld Test", "Erholung", "Untersuche Absturzstelle Bravo");
		Daten_Einf�gen(1, 1, "Feld Test", "Erholung", "Untersuche Absturzstelle Delta");

		Daten_Einf�gen(1, 1, "Massiv Metall", "Schwere Vorraete", "Landung", 350);
		Daten_Einf�gen(1, 1, "Massiv Metall", "Schwere Vorraete", "Aufgelistete Ressourcen in Frachtkapsel ablegen.");

		Daten_Einf�gen(1, 1, "Eissturm", "Expedition", "Landung", 125);
		Daten_Einf�gen(1, 1, "Eissturm", "Expedition", "Platziere die Stoereinrichtung auf dem boden des arktischen Gletschers unter einem Underschlupf und aktiviere sie.");
		Daten_Einf�gen(1, 1, "Eissturm", "Expedition", "Berge Geraetekomponente 1.");
		Daten_Einf�gen(1, 1, "Eissturm", "Expedition", "Berge Geraetekomponente 2.");
		Daten_Einf�gen(1, 1, "Eissturm", "Expedition", "Berge Geraetekomponente 3.");
		Daten_Einf�gen(1, 1, "Eissturm", "Expedition", "Stelle das Geraet zusammen.");

		Daten_Einf�gen(1, 1, "Tiefe Adern", "Extraktion", "Landung", 150);
		Daten_Einf�gen(1, 1, "Tiefe Adern", "Extraktion", "Lokalesiere Exotische Materie.");
		Daten_Einf�gen(1, 1, "Tiefe Adern", "Extraktion", "Baue Exotische Materie ab.");
		Daten_Einf�gen(1, 1, "Tiefe Adern", "Extraktion", "Schaff edie Exotische Materie zum Landeschiff Lager.");

		Daten_Einf�gen(1, 1, "Scheinwerfer", "Scan", "Landung", 150);
		Daten_Einf�gen(1, 1, "Scheinwerfer", "Scan", "Scanne Ort 1");
		Daten_Einf�gen(1, 1, "Scheinwerfer", "Scan", "Scanne Ort 2");
		Daten_Einf�gen(1, 1, "Scheinwerfer", "Scan", "Scanne Ort 3");

		Daten_Einf�gen(1, 1, "Hochseilgarten", "Erkundung", "Erkunde Icarus");

		Daten_Einf�gen(1, 1, "Zufluss", "Aufbau", "Landung", 275);
		Daten_Einf�gen(1, 1, "Zufluss", "Aufbau", "Erreiche den markierten Ort.");
		Daten_Einf�gen(1, 1, "Zufluss", "Aufbau", "Errichte eine Kaserne.");

		Daten_Einf�gen(1, 1, "Zerbrochene Pfeile", "Erholung", "Landung", 150);
		Daten_Einf�gen(1, 1, "Zerbrochene Pfeile", "Erholung", "Beschaffe den Biosprengkopf von Absturzstelle Omega.");
		Daten_Einf�gen(1, 1, "Zerbrochene Pfeile", "Erholung", "Beschaffe den Biosprengkopf von Absturzstelle Psi.");
		Daten_Einf�gen(1, 1, "Zerbrochene Pfeile", "Erholung", "Beschaffe den Biosprengkopf von Absturzstelle Chi.");

		Daten_Einf�gen(1, 1, "Zahltag", "Extraktion", "Landung", 400);
		Daten_Einf�gen(1, 1, "Zahltag", "Extraktion", "Lokalesiere Exotische Materie.");
		Daten_Einf�gen(1, 1, "Zahltag", "Extraktion", "Baue ein exotisches Materievorkommen vollstaeding ab.");
		Daten_Einf�gen(1, 1, "Zahltag", "Extraktion", "Kehre mit exotischer Materie in den Orbit zurueck.");

		Daten_Einf�gen(1, 1, "Zahltag", "Extraktion", "Landung", 250);
		Daten_Einf�gen(1, 1, "Zahltag", "Extraktion", "Stelle Anbauausruestung her.");
		Daten_Einf�gen(1, 1, "Zahltag", "Extraktion", "Errichte Anbauflaechen in einem Gewaechshaus(Glas-Gebaeude).");
		Daten_Einf�gen(1, 1, "Zahltag", "Extraktion", "Deponiere Aufgesitete (In Anbauflaechen) angebaute Ressourcen in der Frachtkapsel.");

		Daten_Einf�gen(1, 1, "Einfall", "Scan", "Landung", 200);
		Daten_Einf�gen(1, 1, "Einfall", "Scan", "Scanne Ort 1");
		Daten_Einf�gen(1, 1, "Einfall", "Scan", "Scanne Ort 2");
		Daten_Einf�gen(1, 1, "Einfall", "Scan", "Scanne Ort 3");

		Daten_Einf�gen(1, 1, "Aufstocken", "Vorraete", "Landung", 600);
		Daten_Einf�gen(1, 1, "Aufstocken", "Vorraete", "Aufgelistete Ressourcen in Frachtkapsel ablegen.");

		Daten_Einf�gen(1, 1, "Edelweiss", "Erkundung", "Erkunde Icarus");

		Daten_Einf�gen(1, 1, "Erhebung", "Bio-Forschung", "Landung", 200);
		Daten_Einf�gen(1, 1, "Erhebung", "Bio-Forschung", "Finde und untersuche die ungewoehnliche Flora.");
		Daten_Einf�gen(1, 1, "Einfall", "Bio-Forschung", "Finde die Daten zur Geode und grabe Sie aus.");

		Daten_Einf�gen(1, 1, "Lawine", "Expedition", "Landung", 600);
		Daten_Einf�gen(1, 1, "Lawine", "Expedition", "Erreiche den Aktispass(M)");
		Daten_Einf�gen(1, 1, "Lawine", "Expedition", "Kundschafte das auf der Karte markierte Gebiet aus(M).");

		Daten_Einf�gen(1, 1, "Feuergang", "Lieferung", "Landung", 250);
		Daten_Einf�gen(1, 1, "Feuergang", "Lieferung", "Terraforming Fechette abrufen,");
		Daten_Einf�gen(1, 1, "Feuergang", "Lieferung", "Liefern sie die Flechette an ihr Dropship.");

		Daten_Einf�gen(1, 1, "Drecksarbeit", "Vernichtung", "Landung", 250);
		Daten_Einf�gen(1, 1, "Drecksarbeit", "Vernichtung", "Suche nach Tierspuren.");
		Daten_Einf�gen(1, 1, "Drecksarbeit", "Vernichtung", "Suche nach der Hoehle des Tiers");
		Daten_Einf�gen(1, 1, "Drecksarbeit", "Vernichtung", "Toete das Raubtier");

		Daten_Einf�gen(1, 1, "Gebuendelt", "Verlaengerte Untersuchung", "Landung", 300);
		Daten_Einf�gen(1, 1, "Gebuendelt", "Verlaengerte Untersuchung", "Fuehre eine Langzeit Untersuchung durch");

		Daten_Einf�gen(1, 1, "Concealment", "Recovery", "Landung", 400);
		Daten_Einf�gen(1, 1, "Concealment", "Recovery", "Zerstoere Arachnideneidaemmungseinheit 1");
		Daten_Einf�gen(1, 1, "Concealment", "Recovery", "Zerstoere Arachnideneidaemmungseinheit 2");

		Daten_Einf�gen(1, 1, "Schneeblind", "Scan", "Landung", 250);
		Daten_Einf�gen(1, 1, "Schneeblind", "Scan", "Scanne Ort 1");
		Daten_Einf�gen(1, 1, "Schneeblind", "Scan", "Scanne Ort 2");
		Daten_Einf�gen(1, 1, "Schneeblind", "Scan", "Scanne Ort 3");

		Daten_Einf�gen(1, 1, "El Camino", "Expedition", "Landung", 250);
		Daten_Einf�gen(1, 1, "El Camino", "Expedition", "Orte die ersten Absturzstelle");
		Daten_Einf�gen(1, 1, "El Camino", "Expedition", "Orte die zweite Absturzstelle");
		Daten_Einf�gen(1, 1, "El Camino", "Expedition", "Orte die letzte Absturzstelle");
		Daten_Einf�gen(1, 1, "El Camino", "Expedition", "Besiege die Kreatur");
		Daten_Einf�gen(1, 1, "El Camino", "Expedition", "Orte die dritte Absturzstelle");

		Daten_Einf�gen(1, 1, "Sieben Saulen", "Scan", "Landung", 200);
		Daten_Einf�gen(1, 1, "Sieben Saulen", "Scan", "Scanne Ort 1");
		Daten_Einf�gen(1, 1, "Sieben Saulen", "Scan", "Scanne Ort 2");
		Daten_Einf�gen(1, 1, "Sieben Saulen", "Scan", "Scanne Ort 3");

		Daten_Einf�gen(1, 1, "Bonze", "Vorraete", "Ladnung");
		Daten_Einf�gen(1, 1, "Nachtache", "Geo-Forschung", "Ladnung");
		Daten_Einf�gen(1, 1, "Gelobtes Land", "Erkundung", "Ladnung");
		Daten_Einf�gen(1, 1, "Abstauben", "Vernichtung", "Ladnung");
		Daten_Einf�gen(1, 1, "Sandkasten", "Aufbau", "Ladnung");
		Daten_Einf�gen(1, 1, "Schneeunfall", "Erholung", "Ladnung");
		Daten_Einf�gen(1, 1, "Saeuberung", "Vernichtung", "Ladnung");
		Daten_Einf�gen(1, 1, "Erweiterte Bestellung", "Vorraete", "Ladnung");
		Daten_Einf�gen(1, 1, "Tundra", "Erkundung", "Ladnung");
		Daten_Einf�gen(1, 1, "Station zu Station", "Verlaengerte Untersuchung", "Ladnung");
		Daten_Einf�gen(1, 1, "Reisende", "Erholung", "Ladnung");
		Daten_Einf�gen(1, 1, "Pilgerreise", "Erkundung", "Ladnung");
		Daten_Einf�gen(1, 1, "Kryogen", "Forschung", "Ladnung");
		Daten_Einf�gen(1, 1, "Ausgegraben", "Forschung", "Ladnung");
	}

	/* Basisfunktion sofern noch keine Tabelle erstellt worden ist */
	void Tabelle_erstellen()
	{
		stmt = con->createStatement();
		stmt->execute("DROP TABLE IF EXISTS " + CAVEENTRACE);
		cout << "Finished dropping table " + CAVEENTRACE + " (if existed)" << endl;
		stmt->execute("CREATE TABLE " + CAVEENTRACE + " (id serial PRIMARY KEY, Name VARCHAR(50), Position_X FLOAT, Position_Y FLOAT, Grundriss VARCHAR(50), Erzdeposits INTEGER);");
		cout << "Finished creating table" + CAVEENTRACE << endl;

		stmt->execute("DROP TABLE IF EXISTS " + MISSIONEN);
		cout << "Finished dropping table " + MISSIONEN + " (if existed)" << endl;
		stmt->execute("CREATE TABLE " + MISSIONEN + " (id serial PRIMARY KEY, Position_X FLOAT, Position_Y FLOAT, Missionsname VARCHAR(50), Missionstyp VARCHAR(50), Missionsbeschreibung VARCHAR(150), Ren INTEGER);");
		cout << "Finished creating table" + MISSIONEN << endl;

		delete stmt;

		Vorgefertigte_Daten_Einlesen();
	}

	/* Dies ist eine Demo Funktion zum Au�lesen aller Daten ungefiltert und unsortiert */
	void Daten_Lesen()
	{
		result = Alle_Daten_Lesen_H�hle->executeQuery();
		H�hlenentraces.clear();
		while (result->next())
		{
			H�hlenentraces.push_back(H�hlendaten(result->getString(2).c_str(), result->getDouble(3), result->getDouble(4), result->getString(5).c_str(), result->getInt(6)));
		}

		for (unsigned int i = 0; i < H�hlenentraces.size(); i++)
		{
			printf("Reading from table Caveentrace=(%d, %d, %d, %s, %d)\n", i + 1, H�hlenentraces[i].getX(), H�hlenentraces[i].getY(), H�hlenentraces[i].Grundriss, H�hlenentraces[i].ErzknotenAnzahl);
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

	/* Funktion zum Auslesen der eingetragenden Missionen (Gefiltert, jede Mission wird nur einmal aufgez�hlt) - Clean */
	vector<pair<string, string>> Daten_Lesen_Missionsname()
	{

		vector<pair<string, string>> temp;

		result = Alle_Daten_Lesen_Mission_Einmalig->executeQuery();

		while (result->next())
			temp.push_back(pair<string, string>(result->getString(1), result->getString(2)));

		return temp;
	}

	vector<glm::vec2> H�lenpositionen()
	{
		vector<glm::vec2> temp;
		result = Alle_Daten_Lesen_H�hle->executeQuery();
		H�hlenentraces.clear();
		while (result->next())
		{
			int tx = result->getInt(3);
			int ty = result->getInt(4);
			H�hlenentraces.push_back(H�hlendaten(result->getString(2).c_str(), tx, ty, result->getString(5).c_str(), result->getInt(6)));
			temp.push_back(glm::vec2(tx, ty));
		}
		return temp;
	};

	/* Funktion zum Auslesen der Ausgew�hlten Missionsinformationen - Clean(Konsole) - TODO(Im Programm): R�ckgabe als Datensatz zum weiterverarbeiten innerhalb der Schleife */
	void Daten_Lesen(pair<string, string> name)
	{
		vector<Missionsdaten> temp;

		Einzel_Daten_Lesen_Mission = con->prepareStatement(SQL_W�HLE_ALLE_VON + MISSIONEN + SQL_WO + MISSIONSNAME + "='" + name.first + "'" + " AND " + MISSIONSTYP + "='" + name.second + "'");

		result = Einzel_Daten_Lesen_Mission->executeQuery();
		bool Erste_Missionsinfo = false;

		while (result->next())
		{
			temp.push_back(Missionsdaten(result->getInt(2), result->getInt(3), result->getString(4).c_str(), result->getString(5).c_str(), result->getString(6).c_str(), result->getInt(7)));

			temp.back().getX();

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

		Einzel_Daten_Lesen_Mission->close();
	}

	/* TODO(Im Programm) nach Loginfenster zum Verschieben einzelner Objekte als Datenbank Manager */
	void Daten_Updaten(string name, float x, float y)
	{
		//update
		pstmt = con->prepareStatement("UPDATE " + CAVEENTRACE + " SET Position_X = ?, Position_Y = ? WHERE name = ?");

		pstmt->setString(1, name);
		pstmt->setDouble(2, x);
		pstmt->setDouble(3, y);

		pstmt->executeQuery();
		cout << FUNKTIONSLOS << endl; //cout << "Zeile Geupdatet" << endl;
		;
	}

	/* TODO(Im Programm) nach Loginfenster zum entfernen einzelner Objekte als Datenbank Manager */
	void Daten_L�schen(string name)
	{
		//delete
		//pstmt = con->prepareStatement("DELETE FROM inventory WHERE name = ?");
		//pstmt->setString(1, name);
		//result = pstmt->executeQuery();
		cout << FUNKTIONSLOS << endl; //cout << "Zeile gel�scht." << endl;
		;
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

	/* Die Funktion um das Text R�ndern zu erm�glichen */
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

class Objektmarker
{
	uint VBO, VAO, EBO;
	uint instanceVBO;
	vector<glm::vec2> translations;
public:
	Objektmarker()
	{
		int index = 0;
		float offset = 0.1f;
		for (int y = -10; y < 10; y += 2)
		{
			for (int x = -10; x < 10; x += 2)
			{
				glm::vec2 translation;
				translation.x = (float)x / 1.0f + offset;
				translation.y = (float)y / 1.0f + offset;
				index++;
				translations.push_back(translation);
			}
		}

		// store instance data in an array buffer		
		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * translations.size(), &translations[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Der Speicher f�r den Zeichenbereich der Markierung
		float vertices[] = {
			// positions          // texture coords
				0.1f, 0.3f, 0.0f,   1.0f, 1.0f, // top right
				0.1f, 0.0f, 0.0f,   1.0f, 0.0f, // bottom right
			-0.1f, 0.0f, 0.0f,   0.0f, 0.0f, // bottom left

			-0.1f, 0.3f, 0.0f,   0.0f, 1.0f,  // top left 
				0.1f, 0.3f, 0.0f,   1.0f, 1.0f, // top right
			-0.1f, 0.0f, 0.0f,   0.0f, 0.0f // bottom left
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		/* In GLSL der Speicherbereich f�r die Verxtex(Punkt) Positionen */
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		/* IN GLSL der Speicherbereich f�r die Texturen Position */
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

// Immer wenn die Fenstergr��e ge�ndert wird, f�hre diesen Callback aus.
inline void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	Fensterdaten(width, height);
}

// Immer wenn das Mausrad gedreht wird, f�hre diesen Callback aus.
inline void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Cam.C.z += yoffset;
};

//inline void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
//{
//}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		Fensterdaten.Maus.Is_Links_Klickt = GLFW_PRESS;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_PRESS)
	{
		Fensterdaten.Maus.Is_Links_Klickt = 0;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		Fensterdaten.Maus.Is_Rechts_Klickt = GLFW_PRESS;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_PRESS)
	{
		Fensterdaten.Maus.Is_Rechts_Klickt = 0;
	}

	if (action == GLFW_PRESS)
	{
		Fensterdaten.Maus.Action = action;
	}
}

bool Statusanzeige = false;

glm::vec3 CreateRay() 
{
	// these positions must be in range [-1, 1] (!!!), not [0, width] and [0, height]
	//float mouseX = getMousePositionX() / (getWindowWidth()  * 0.5f) - 1.0f;
	//float mouseY = getMousePositionY() / (getWindowHeight() * 0.5f) - 1.0f;

	glm::mat4 proj = Cam.projection;//glm::perspective(FoV, AspectRatio, Near, Far);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f), { 0.0,0.0,1.0 }, glm::vec3{0.0,1.0,0.0});

	glm::mat4 invVP = glm::inverse(proj * view);
	glm::vec4 screenPos = glm::vec4(Fensterdaten.Maus.NX, -Fensterdaten.Maus.NY, 1.0f, 1.0f);
	glm::vec4 worldPos = invVP * screenPos;

	glm::vec3 dir = glm::normalize(glm::vec3(worldPos));

	return dir;
}

glm::vec3 test(glm::mat4 Projection)
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


	//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	for (float i = -1.0; i <= 1.0; i += 0.1)
	{
		glm::vec3 t1 = glm::unProjectNO(glm::vec3(Fensterdaten.Maus.X, Fensterdaten.Maus.Y, i), model, proj, Fensterdaten.viewport);
		cout << "erhalte ich bei z"<< i << " | die Koordianten" << t1.x << "," << t1.y << "," << t1.z << endl;
	}
	glm::vec3 t1 = glm::unProjectNO(glm::vec3(Fensterdaten.Maus.X, Fensterdaten.Maus.Y, 0.0), model, proj, Fensterdaten.viewport);
	return t1;

	// mouse coords in range (-1 -> 1) glm::normalize(glm::vec3(glm::inverse(proj) * glm::vec4(Fensterdaten.Maus.NX, Fensterdaten.Maus.NY, 1.0f, 1.0f)));
}

bool compare(glm::vec3 Cursor, H�hlendaten Objekt)
{
	/* Wenn das X und Y  innerhalb des Bereichs liegt - gebe den namen zur�ck */
	if (Cursor.x >= (Objekt.getX() + (-0.1)) && Cursor.x <= (Objekt.getX() + (0.1)) && Cursor.y >= (Objekt.getY() + (-0.1)) && Cursor.y <= (Objekt.getY() + (0.1)))
		return true;
	return false;
}
/*
void ScreenPosToWorldRay(
	//int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
	//int screenWidth, int screenHeight,  // Window size, in pixels
	glm::mat4 ViewMatrix,               // Camera position and orientation
	glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
	glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
	glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
) {

	// The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
	glm::vec4 lRayStart_NDC(
		(Fensterdaten.Maus.X / Fensterdaten.X - 0.5f) * 1.0f, // [0,1024] -> [-1,1]
		(Fensterdaten.Maus.Y / Fensterdaten.Y - 0.5f) * 1.0f, // [0, 768] -> [-1,1]
		-1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
		1.0f
	);
	glm::vec4 lRayEnd_NDC( (Fensterdaten.Maus.X / Fensterdaten.X - 0.5f) * 1.0f, (Fensterdaten.Maus.Y / Fensterdaten.Y - 0.5f) * 1.0f, 0.0, 1.0f);


	// The Projection matrix goes from Camera Space to NDC.
	// So inverse(ProjectionMatrix) goes from NDC to Camera Space.
	glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);

	// The View Matrix goes from World Space to Camera Space.
	// So inverse(ViewMatrix) goes from Camera Space to World Space.
	glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);

	glm::vec4 lRayStart_camera	= InverseProjectionMatrix	* lRayStart_NDC;    lRayStart_camera	/= lRayStart_camera.w;
	glm::vec4 lRayStart_world	= InverseViewMatrix			* lRayStart_camera; lRayStart_world		/= lRayStart_world.w;
	glm::vec4 lRayEnd_camera	= InverseProjectionMatrix	* lRayEnd_NDC;      lRayEnd_camera		/= lRayEnd_camera.w;
	glm::vec4 lRayEnd_world		= InverseViewMatrix			* lRayEnd_camera;   lRayEnd_world		/= lRayEnd_world.w;


	// Faster way (just one inverse)
	//glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
	//glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
	//glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;


	glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
	lRayDir_world = glm::normalize(lRayDir_world);


	out_origin		= glm::vec3(lRayStart_world);
	cout << out_origin.x << "," << out_origin.y << "," << out_origin.z << endl;
	out_direction	= glm::normalize(lRayDir_world);
	cout << out_direction.x << "," << out_direction.y << "," << out_direction.z << endl;
}

bool TestRayOBBIntersection(
	glm::vec3 ray_origin,        // Ray origin, in world space
	glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
	glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
	glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
	float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
) {

	// Intersection method from Real-Time Rendering and Essential Mathematics for Games

	float tMin = 0.0f;
	float tMax = 100000.0f;

	glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

	glm::vec3 delta = OBBposition_worldspace - ray_origin;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
		glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);

		if (fabs(f) > 0.001f) { // Standard case

			float t1 = (e + aabb_min.x) / f; // Intersection with the "left" plane
			float t2 = (e + aabb_max.x) / f; // Intersection with the "right" plane
			// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

			// We want t1 to represent the nearest intersection, 
			// so if it's not the case, invert t1 and t2
			if (t1 > t2) {
				float w = t1; t1 = t2; t2 = w; // swap t1 and t2
			}

			// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
			if (t2 < tMax)
				tMax = t2;
			// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
			if (t1 > tMin)
				tMin = t1;

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (tMax < tMin)
				return false;

		}
		else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
			if (-e + aabb_min.x > 0.0f || -e + aabb_max.x < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Y axis
	// Exactly the same thing than above.
	{
		glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if (fabs(f) > 0.001f) {

			float t1 = (e + aabb_min.y) / f;
			float t2 = (e + aabb_max.y) / f;

			if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;
			if (tMin > tMax)
				return false;

		}
		else {
			if (-e + aabb_min.y > 0.0f || -e + aabb_max.y < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Z axis
	// Exactly the same thing than above.
	{
		glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray_direction, zaxis);

		if (fabs(f) > 0.001f) {

			float t1 = (e + aabb_min.z) / f;
			float t2 = (e + aabb_max.z) / f;

			if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;
			if (tMin > tMax)
				return false;

		}
		else {
			if (-e + aabb_min.z > 0.0f || -e + aabb_max.z < 0.0f)
				return false;
		}
	}

	intersection_distance = tMin;
	return true;

}
*/

// Verarbeite hier s�mtlichen Input, zum trennen von Renderdaten und Tastertur
inline void processInput(GLFWwindow *window)
{
	// Die Default Steuerung zum Schlie�en des Fensters
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	/* Abschnitt der Kartnebewegung mit der Maus */
	if (Fensterdaten.Maus.Is_Links_Klickt && Fensterdaten.ID == 0)
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
	else if (Fensterdaten.Maus.Is_Links_Klickt && Fensterdaten.ID != 0)
	{
		double N_X, N_Y, CX, CY;

		glfwGetCursorPos(window, &N_X, &N_Y);

		CX = N_X - Fensterdaten.Maus.X;

		CY = Fensterdaten.Maus.Y - N_Y;

		Fensterdaten.ID ->X += (CX / 100.0);
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
	if ((glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS || Fensterdaten.Maus.Is_Rechts_Klickt) && !Statusanzeige)
	{
		//cout << Fensterdaten.Maus.X << "," << Fensterdaten.Maus.Y << endl;
		//cout << Cam.X << "," << Cam.Y << endl;
			   
		glm::vec3 T0 = test(Cam.projection);//CreateRay();
		//cout << T0.x << "," << T0.y << "," << T0.z << endl;
		//vector<glm::vec3> positions;

		//for (uint i = 0; i < H�hlenentraces.size(); i++)
		//	positions.push_back(H�hlenentraces[i]);
		//
		//glm::mat4 view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		//		glm::vec3 ray_origin;
		//		glm::vec3 ray_direction;
		//		ScreenPosToWorldRay(
		//			//1024 / 2, 768 / 2,
		//			//1024, 768,
		//			view,
		//			Cam.projection,
		//			ray_origin,
		//			ray_direction
		//		);

		for (uint i = 0; i < H�hlenentraces.size(); i++)
		{
			//float intersection_distance; // Output of TestRayOBBIntersection()
			//glm::vec3 aabb_min(-0.1f, 0.0f, 0.0f);
			//glm::vec3 aabb_max( 0.1f, 0.3f, 0.0f);

			// The ModelMatrix transforms :
			// - the mesh to its desired position and orientation
			// - but also the AABB (defined with aabb_min and aabb_max) into an OBB
			//glm::mat4 RotationMatrix = glm::mat4(1.0f);
			//glm::mat4 TranslationMatrix = glm::translate(TranslationMatrix, glm::vec3(0.0 + Cam.C.x +H�hlenentraces[i].X, 0.0f + Cam.C.y + H�hlenentraces[i].Y, -0.001 + Cam.C.z)); //glm::translate(glm::mat4(), positions[i]);
			//glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;

			if (compare(T0, H�hlenentraces[i])) // if (TestRayOBBIntersection(ray_origin, ray_direction, aabb_min, aabb_max, ModelMatrix, intersection_distance)) //
			{
				Fensterdaten.ID = &H�hlenentraces[i];
				Testausgabe = H�hlenentraces[i].name;
				if (Fensterdaten.ID != 0)
					cout << Fensterdaten.ID->name << endl;
				break;
			}
			else
			{
				if(Fensterdaten.ID != 0)
					SQL.Daten_Updaten(Fensterdaten.ID->name, Fensterdaten.ID->X, Fensterdaten.ID->Y);
				Fensterdaten.ID = 0;
			}
				
		}
		Statusanzeige = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F) != GLFW_PRESS && !Fensterdaten.Maus.Is_Rechts_Klickt)
	{
		Statusanzeige = false;
	}

	// Die Steuerung um sich auf der Karte nach Oben zu bewegen.
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		if (Cam.C.y > -20.0)
			Cam.C.y -= 0.001;
	}
	// Die Steuerung um sich auf der Karte nach Unten zu bewegen.
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		if (Cam.C.y < 20.0)

			Cam.C.y += 0.001;
	}
	// Die Steuerung um sich auf der Karte nach Rechts zu bewegen.
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		if (Cam.C.x > -20.0)
			Cam.C.x -= 0.001;
	}
	// Die Steuerung um sich auf der Karte nach Links zu bewegen.
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		if (Cam.C.x < 20.0)
			Cam.C.x += 0.001;
	}
}

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


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	const char* glsl_version = "#version 130";
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(Fensterdaten.getX(), Fensterdaten.getY(), "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}	

	//PRIVAT_MYSQL SQL;
	FREETYPE Schrift;
	MAP Map;
	Objektmarker Entrace;   /* Marker f�r H�hleneing�nge */
	Objektmarker Mission;   /* Marker f�r Missionen */
	Objektmarker Exospots;  /* Marker f�r Exospots */
	Objektmarker Bossspots; /* Marker f�r BossSpots */

	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("5.1.transform.vs", "5.1.transform.fs");

	// build and compile our shader zprogram
	// ------------------------------------
	Shader instanzer("MatrixShader.vs", "MatrixShader.fs");

	// compile and setup the shader
	// ----------------------------
	Shader shader("text.vs", "text.fs");
	//shader.use();

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

	
		
	// load and create a texture 
	// -------------------------
	unsigned int texture1, texture2;

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

	static bool Einf�gen = false;
	static bool Updaten = false;
	static bool L�schen = false;
	static bool Mission_Caveentrace = false;		// Fakse = Inventory True = Caveentrace
	static bool Mission_Ausgew�hlt = false;

	bool Wechsel = false;


	float radians = 0.0;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		{
		// input
		// -----
		processInput(window);
			   		 
		vector<glm::vec2> Tempo;
		for (uint i = 0; i < H�hlenentraces.size(); i++)
			Tempo.push_back(H�hlenentraces[i]);
		Entrace.update(Tempo);


		Cam.view = glm::translate(Cam.view, glm::vec3(0.0f, 0.0f, -3.0f));
		Cam.projection = glm::perspective(glm::radians(45.0f), (float)Fensterdaten.getX() / (float)Fensterdaten.getY(), 0.1f, 100.0f);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			ImGui::Begin("Interface Interaktive Map");                          // Create a window called "Hello, world!" and append into it.

			if (Mission_Ausgew�hlt)
			{
				if (ImGui::ArrowButton("##left", ImGuiDir_Left)) //ImGui::SameLine(0.0f, spacing);
				{
					Missionsliste = SQL.Daten_Lesen_Missionsname();
					Mission_Ausgew�hlt = false;
				}
			}

			ImGui::Text("Dies ist die Datensteuerung fuer\ndie Interaktive Karte.");               // Display some text (you can use a format strings too)
			ImGui::Text(Testausgabe.c_str());               // Display some text (you can use a format strings too)

			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

			if (ImGui::Button("SQL Tabelle"))
				SQL.Tabelle_erstellen();

			if (ImGui::Button("SQL Einfuegen"))
				Einf�gen = !Einf�gen;
			if (Einf�gen)
			{
				if (!Mission_Caveentrace)
				{
					if (ImGui::Button("Einfuegen ist Inventory"))
						Mission_Caveentrace = true;
				}
				else
					if (ImGui::Button("Einfuegen ist Caveentrace"))
						Mission_Caveentrace = false;

				if (!Mission_Caveentrace)
				{
					static char Missionname[128] = "Hello, world!";
					ImGui::InputText("Missionsname", Missionname, IM_ARRAYSIZE(Missionname));
					ImGui::SameLine(); HelpMarker(
						"USER:\n"
						"Hold SHIFT or use mouse to select text.\n"
						"CTRL+Left/Right to word jump.\n"
						"CTRL+A or double-click to select all.\n"
						"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
						"CTRL+Z,CTRL+Y undo/redo.\n"
						"ESCAPE to revert.\n\n"
						"PROGRAMMER:\n"
						"You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() "
						"to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated "
						"in imgui_demo.cpp).");

					static char Missionstyp[128] = "Hello, world!";
					ImGui::InputText("Missionstyp", Missionstyp, IM_ARRAYSIZE(Missionstyp));
					ImGui::SameLine(); HelpMarker(
						"USER:\n"
						"Hold SHIFT or use mouse to select text.\n"
						"CTRL+Left/Right to word jump.\n"
						"CTRL+A or double-click to select all.\n"
						"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
						"CTRL+Z,CTRL+Y undo/redo.\n"
						"ESCAPE to revert.\n\n"
						"PROGRAMMER:\n"
						"You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() "
						"to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated "
						"in imgui_demo.cpp).");

					static char Missionsbeschreibung[512] = "Hello, world!";
					ImGui::InputText("Missionsbeschreibung", Missionsbeschreibung, IM_ARRAYSIZE(Missionsbeschreibung));
					ImGui::SameLine(); HelpMarker(
						"USER:\n"
						"Hold SHIFT or use mouse to select text.\n"
						"CTRL+Left/Right to word jump.\n"
						"CTRL+A or double-click to select all.\n"
						"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
						"CTRL+Z,CTRL+Y undo/redo.\n"
						"ESCAPE to revert.\n\n"
						"PROGRAMMER:\n"
						"You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() "
						"to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated "
						"in imgui_demo.cpp).");

					static int ren = 123;
					ImGui::InputInt("Ren", &ren);

					static int exo = 123;
					ImGui::InputInt("Exotics", &exo);

					if (ImGui::Button("Einfuegen"))
					{
						SQL.Daten_Einf�gen(1, 1, Missionname, Missionstyp, Missionsbeschreibung, ren); Einf�gen = false;
					}
				}
				else
				{
					static char name[128] = "Kleine Pilzhoehle";
					ImGui::InputText("Name", name, IM_ARRAYSIZE(name));
					ImGui::SameLine(); HelpMarker(
						"USER:\n"
						"Hold SHIFT or use mouse to select text.\n"
						"CTRL+Left/Right to word jump.\n"
						"CTRL+A or double-click to select all.\n"
						"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
						"CTRL+Z,CTRL+Y undo/redo.\n"
						"ESCAPE to revert.\n\n"
						"PROGRAMMER:\n"
						"You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() "
						"to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated "
						"in imgui_demo.cpp).");

					static char str0[128] = "Kleine Pilzhoehle";
					ImGui::InputText("Grundriss", str0, IM_ARRAYSIZE(str0));
					ImGui::SameLine(); HelpMarker(
						"USER:\n"
						"Hold SHIFT or use mouse to select text.\n"
						"CTRL+Left/Right to word jump.\n"
						"CTRL+A or double-click to select all.\n"
						"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
						"CTRL+Z,CTRL+Y undo/redo.\n"
						"ESCAPE to revert.\n\n"
						"PROGRAMMER:\n"
						"You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() "
						"to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated "
						"in imgui_demo.cpp).");

					static int ix = 2;
					ImGui::InputInt("X", &ix);
					static int iy = 2;
					ImGui::InputInt("Y", &iy);
					static int ic = 2;
					ImGui::InputInt("Anzahl", &ic);

					if (ImGui::Button("Einfuegen"))
					{
						SQL.Daten_Einf�gen(name,ix, iy, str0, ic); Einf�gen = false;
					}
				}
			}

			if (ImGui::Button("SQL Lesen"))
			{
				vector<glm::vec2> Temp = SQL.H�lenpositionen();
				Entrace.update(Temp);
			}

			int x = H�hlenentraces[0].getX();
			int y = H�hlenentraces[0].getY();

			if (ImGui::Button("SQL Updaten"))
				Updaten = !Updaten;
			if (Updaten)
			{
				static char str1[128] = "orange";
				ImGui::InputText("input text", str1, IM_ARRAYSIZE(str1));
				ImGui::SameLine(); HelpMarker(
					"USER:\n"
					"Hold SHIFT or use mouse to select text.\n"
					"CTRL+Left/Right to word jump.\n"
					"CTRL+A or double-click to select all.\n"
					"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
					"CTRL+Z,CTRL+Y undo/redo.\n"
					"ESCAPE to revert.\n\n"
					"PROGRAMMER:\n"
					"You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() "
					"to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated "
					"in imgui_demo.cpp).");


				static int px = 1000;
				ImGui::InputInt("input int", &px);

				static int py = 1000;
				ImGui::InputInt("input int", &py);

				if (ImGui::Button("Updaten"))
				{
					SQL.Daten_Updaten(string(str1),px,py);
					Updaten = false;
				}
			}

			if (ImGui::Button("SQL Loeschen"))
				L�schen = !L�schen;
			if (L�schen)
			{
				static char str2[128] = "orange";
				ImGui::InputText("input text", str2, IM_ARRAYSIZE(str2));
				ImGui::SameLine(); HelpMarker(
					"USER:\n"
					"Hold SHIFT or use mouse to select text.\n"
					"CTRL+Left/Right to word jump.\n"
					"CTRL+A or double-click to select all.\n"
					"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
					"CTRL+Z,CTRL+Y undo/redo.\n"
					"ESCAPE to revert.\n\n"
					"PROGRAMMER:\n"
					"You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() "
					"to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated "
					"in imgui_demo.cpp).");

				if (ImGui::Button("Loeschen"))
				{
					SQL.Daten_L�schen(string(str2));
					L�schen = false;
				}
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			// Liste der Missionen
			if (!Mission_Ausgew�hlt)
			{
				ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
				ImGui::BeginChild("ChildR", ImVec2(0, 260), true, window_flags);
				if (ImGui::BeginTable("split", 1, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
				{
					for (int i = 0; i < Missionsliste.size(); i++)
					{
						//char buf[32];
						//sprintf(buf, "%03d", i);
						ImGui::TableNextColumn();
						string combi = Missionsliste[i].first + Missionsliste[i].second;
						if (ImGui::Button(combi.c_str(), ImVec2(-FLT_MIN, 0.0f)))
						{
							SQL.Daten_Lesen(Missionsliste[i]);
							Mission_Ausgew�hlt = true;
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

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Map.Render(&ourShader,&texture1,&texture2);

		Entrace.Render(&instanzer, Blauer_Tropfen, Blauer_Tropfen);
		//Bossspots.Render(&instanzer, Roter_Tropfen, Roter_Tropfen);
		//Exospots.Render(&instanzer, Schwarzer_Tropfen, Schwarzer_Tropfen);
		//Mission.Render(&instanzer, Gelber_Tropfen, Gelber_Tropfen);

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