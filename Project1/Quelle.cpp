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
	//#include <pair>

	#include <glad/glad.h>
	#include <GLFW/glfw3.h>

	#include <stb_image.h>

	#include <glm/glm.hpp>
	#include <glm/gtc/matrix_transform.hpp>
	#include <glm/gtc/type_ptr.hpp>

	#include <ft2build.h>
	#include FT_FREETYPE_H

	#include "Shader.h"

	#include <iostream>

	#include "imgui\imgui.h"
	#include "imgui\imgui_impl_glfw.h"
	#include "imgui\imgui_impl_opengl3.h"

	#include "mysql_connection.h"
	#include <cppconn/driver.h>
	#include <cppconn/exception.h>
	#include <cppconn/prepared_statement.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

#ifdef WIKI
/*
	Problem Stellung: 
			- Die String werden beim Dritten %s nicht Ordnungsgemäß an die Konsole übergeben. 
			Mögliche Idee, Prüfen der Umgebungsvariablen der Konsolen
			LÖSUNG: Achtung es scheint das System zu Überladen wenn 3 strings in Folge an die Konsole übergeben werden, anders Trennen der string Übertragung

			Zuweisen der Auswählbaren Missionen in eine Map legen, um diese im Auswahlfenster nur einmal zu haben.

*/
#endif

#ifndef DEFINITIONEN
#define DEFINITIONEN

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

	#define SQL_EINFÜGEN_IN string("INSERT INTO ")
	#define SQL_WÄHLE_ALLE_VON string("SELECT * FROM ")
	#define SQL_WÄHLE_EINZIGARTIGE string("SELECT DISTINCT " + MISSIONSNAME + ", " + MISSIONSTYP + " FROM ")
	#define SQL_WO string(" WHERE ")

	#define CAVEENTRACE string("`caveentrace`")
	#define INVENTORY string("inventory")
	#define MISSIONEN string("`mission`")
	
#endif

#ifndef BASICDEFINITIONEN
#define BASICDEFINITIONEN

	// settings
	const unsigned int SCR_WIDTH = 800;
	const unsigned int SCR_HEIGHT = 600;

	//for demonstration only. never save your password in the code!
	const std::string server = "tcp://127.0.0.1:3306";
	const std::string username = "root";
	const std::string password = "Drachenwolf!2";

	/// Holds all state information relevant to a character as loaded using FreeType
	struct Character
	{
		unsigned int TextureID; // ID handle of the glyph texture
		glm::ivec2   Size;      // Size of glyph
		glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
		unsigned int Advance;   // Horizontal offset to advance to next glyph
	};
	std::map<GLchar, Character> Characters;
	unsigned int VAO, VBO;

	using namespace std;
	
	vector<pair<string,string>> Missionsliste;
	string Missionsnamensuche;
#endif

inline void framebuffer_size_callback(GLFWwindow* window, int width, int height);
inline void processInput(GLFWwindow *window);
inline void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

inline void create_Texture(unsigned int &texture)
{
	// texture 1
	// ---------
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char *data = stbi_load("Resourcen/HQ_Map.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

inline void RenderText_aktivieren()
{
	// OpenGL state
	// ------------
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
};
inline void RenderText_deaktivieren()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
};

static void HelpMarker(const char* desc)
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
}

class Koordinaten
{
	int X, Y;
public:
	Koordinaten() {};
	Koordinaten(int X, int Y) : X(X), Y(Y) {};
	int getX() { return X; };
	int getY() { return Y; };
};

class Höhlendaten : public Koordinaten
{	

public:
	string Grundriss;
	int ErzknotenAnzahl;
	Höhlendaten(int X, int Y, string Grundriss, int Anzahl) : Koordinaten(X, Y), Grundriss(Grundriss), ErzknotenAnzahl(Anzahl)
	{};
	Höhlendaten(Koordinaten cords, string Grundriss, int Anzahl) : Koordinaten(cords.getX(), cords.getY()), Grundriss(Grundriss), ErzknotenAnzahl(Anzahl)
	{};

	operator Koordinaten()
	{
		return Koordinaten(this->getX(), this->getY());
	}
};

/* Die Koordinaten sollten für den Missionsbeginn beim Landungschiff stehen */
class Missionsdaten : public Koordinaten
{
	string Missionsname;
	string Missionstyp;
	
	int Belohnung_Ren, Belohnung_Exotic;
public:
	string Missionsbeschreibung;
	Missionsdaten(int X, int Y, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren, int Exotics) : Koordinaten(X, Y),
		Missionsname(Missionsname), Missionsbeschreibung(Missionsbeschreibung), Missionstyp(Missionstyp), Belohnung_Ren(Ren), Belohnung_Exotic(Exotics)
	{};
	Missionsdaten(Koordinaten cords, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren, int Exotics) : Koordinaten(cords.getX(), cords.getY()),
		Missionsname(Missionsname), Missionsbeschreibung(Missionsbeschreibung), Missionstyp(Missionstyp), Belohnung_Ren(Ren), Belohnung_Exotic(Exotics)
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
	int getExotics()
	{
		return Belohnung_Exotic;
	}
};

vector<Höhlendaten> Hölenentraces;
vector<Missionsdaten> MISSIONSDATEN;

class PRIVAT_MYSQL
{
	sql::Driver *driver;
	sql::Connection *con;
	sql::PreparedStatement *pstmt;

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
	}
	/* Aufräumen der Zeigerarythmetic (Ohne Smart Pointer = Speicherüberlauf "vorprogrammiert") */
	~PRIVAT_MYSQL()
	{	
		//if (driver != 0)	delete driver;
		if (pstmt != 0)		delete pstmt;
		if (con != 0)		delete con;
		if (result != 0)	delete result;
	}

	// Demo - Funktion Anhand des Standard Beispiels "inventroy" wird Demonstiert, wie der Aufbau ist.
	void Daten_Einfügen(string Objekt, int Anzahl) 
	{
		pstmt = con->prepareStatement(SQL_EINFÜGEN_IN + INVENTORY + "(name, quantity) VALUES(?,?);");

		pstmt->setString(1, Objekt);
		pstmt->setInt(2, Anzahl);
		pstmt->execute();
			cout << "One row in " + INVENTORY + "  inserted." << endl;
	}
	/* Funktion zum Einfügen der Höhlen Daten - Speicher der Position x + y, der Grundriss Datei und der Typischen Erz Knoten Anzahl*/
	void Daten_Einfügen(int X, int Y, string Grundriss, int Anzahl)
	{
		pstmt = con->prepareStatement(SQL_EINFÜGEN_IN + CAVEENTRACE +"(Position_X, Position_Y, Grundriss, Erzdeposits) VALUES(?,?,?,?);");

		pstmt->setInt(1, X);
		pstmt->setInt(2, Y);
		pstmt->setString(3, Grundriss);
		pstmt->setInt(4, Anzahl);

		pstmt->execute();
			cout << "One row in " + CAVEENTRACE + " inserted."<< endl;
	}
	/* Funktion zum Einfügen der Missiontexte - Es wird ein X/Y gespeichert um Mögliche unterschiedliche Missionsziele zu speichern */
	void Daten_Einfügen(int X, int Y, string Missionsname, string Missionstyp, string Missionsbeschreibung, int Ren = 0, int Exotics = 0)
	{
		pstmt = con->prepareStatement(SQL_EINFÜGEN_IN + MISSIONEN + "(Position_X, Position_Y, Missionsname, Missionstyp, Missionsbeschreibung, Ren, Exotics) VALUES(?,?,?,?,?,?,?);");

		pstmt->setInt(1, X);
		pstmt->setInt(2, Y);
		pstmt->setString(3, Missionsname);
		pstmt->setString(4, Missionstyp);
		pstmt->setString(5, Missionsbeschreibung);
		pstmt->setInt(6, Ren);
		pstmt->setInt(7, Exotics);
		
		pstmt->execute();
		cout << "One row in " + MISSIONEN + " inserted." << endl;
	}
	
	void Tabelle_erstellen()
	{
		stmt = con->createStatement();
		stmt->execute("DROP TABLE IF EXISTS " + INVENTORY);
			cout << "Finished dropping table " + INVENTORY + " (if existed)" << endl;
		stmt->execute("CREATE TABLE " + INVENTORY + " (id serial PRIMARY KEY, name VARCHAR(50), quantity INTEGER);");
			cout << "Finished creating table " + INVENTORY << endl;

		stmt->execute("DROP TABLE IF EXISTS " + CAVEENTRACE);
			cout << "Finished dropping table " + CAVEENTRACE +" (if existed)" << endl;
		stmt->execute("CREATE TABLE " + CAVEENTRACE + " (id serial PRIMARY KEY, Position_X INTEGER, Position_Y INTEGER, Grundriss VARCHAR(50), Erzdeposits INTEGER);");
			cout << "Finished creating table" + CAVEENTRACE << endl;

		stmt->execute("DROP TABLE IF EXISTS " + MISSIONEN);
			cout << "Finished dropping table " + MISSIONEN + " (if existed)" << endl;
		stmt->execute("CREATE TABLE " + MISSIONEN + " (id serial PRIMARY KEY, Position_X INTEGER, Position_Y INTEGER, Missionsname VARCHAR(50), Missionstyp VARCHAR(50), Missionsbeschreibung VARCHAR(50), Ren INTEGER, Exotics INTEGER);");
			cout << "Finished creating table" + MISSIONEN << endl;

		delete stmt;

		//Daten_Einfügen("banana", 150);
		//Daten_Einfügen("orange", 154);
		//Daten_Einfügen("apple", 100);

		Daten_Einfügen(4, 8, "Kleine", 10);

		Daten_Einfügen(1, 1, "Schneeblind", "Scan", "Landung fuer Scanne 3 Orte");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Landung fuer Scanne drei Orte", 250);
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Orte die ersten Absturzstelle");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Orte die zweite Absturzstelle");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Orte die letzte Absturzstelle");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Besiege die Kreatur");
		Daten_Einfügen(1, 1, "El Camino", "Expedition", "Orte die dritte Absturzstelle");
	}

	void Daten_Lesen()
	{
		//select  
		//pstmt = con->prepareStatement(SQL_WÄHLE_ALLE_VON + INVENTORY + ";");
		//result = pstmt->executeQuery();

		//while (result->next())
		//	printf("Reading from table Inventory=(%d, %s, %d)\n", result->getInt(1), result->getString(2).c_str(), result->getInt(3));

		//select  
		pstmt = con->prepareStatement(SQL_WÄHLE_ALLE_VON + CAVEENTRACE + ";");
		result = pstmt->executeQuery();
		Hölenentraces.clear();
		while (result->next())
		{			
			Hölenentraces.push_back(Höhlendaten(result->getInt(2), result->getInt(3), result->getString(4).c_str(), result->getInt(5)));
		}

		for (unsigned int i = 0; i < Hölenentraces.size(); i++)
		{
			printf("Reading from table Caveentrace=(%d, %d, %d, %s, %d)\n",  i +1 , Hölenentraces[i].getX(), Hölenentraces[i].getY(), Hölenentraces[i].Grundriss, Hölenentraces[i].ErzknotenAnzahl);
		}

		//select  
		pstmt = con->prepareStatement(SQL_WÄHLE_ALLE_VON + MISSIONEN + ";");
		result = pstmt->executeQuery();

		MISSIONSDATEN.clear();
		while (result->next())
		{
			MISSIONSDATEN.push_back(Missionsdaten(result->getInt(2), result->getInt(3), result->getString(4).c_str(), result->getString(5).c_str(), 
				result->getString(6).c_str(), result->getInt(7), result->getInt(8)));
		}

		for (unsigned int i = 0; i < MISSIONSDATEN.size(); i++)
		{
			//string text = MISSIONSDATEN[i].getBeschreibung();
			//int cX = MISSIONSDATEN[i].getX();
			//printf("Reading from table Mission=(%d, %d, %d, %s: %s, %s, %d, %d )\n", i + 1, MISSIONSDATEN[i].getX(), MISSIONSDATEN[i].getY(), MISSIONSDATEN[i].getName(), 
			//	MISSIONSDATEN[i].getTyp(),MISSIONSDATEN[i].getBeschreibung(), MISSIONSDATEN[i].getRen(), MISSIONSDATEN[i].getExotics());

			std::cout << "Reading from table Mission = (" << i + 1 << ", " << MISSIONSDATEN[i].getX() << ", " << MISSIONSDATEN[i].getY()
				<< ", " << MISSIONSDATEN[i].getName() << ": " << MISSIONSDATEN[i].getTyp(); 
			cout << ", " << MISSIONSDATEN[i].getBeschreibung() << ", " << MISSIONSDATEN[i].getRen() << ", " << MISSIONSDATEN[i].getExotics() << ")" << endl;
		}
	}

	/* Funktion zum Auslesen der eingetragenden Missionen (Gefiltert, jede Mission wird nur einmal aufgezählt) - Clean */
	vector<pair<string,string>> Daten_Lesen_Missionsname()
	{
		
		vector<pair<string,string>> temp;

		/* Aufruf der Anweisung zur Filterung der Missionen */
		pstmt = con->prepareStatement(SQL_WÄHLE_EINZIGARTIGE + MISSIONEN + ";");
		result = pstmt->executeQuery();

		while (result->next())
			temp.push_back(pair<string, string>(result->getString(1) , ": " + result->getString(2)));

		return temp;
	}

	/* DemoFunktion zum Speziellen Filtern nach Namen - Wird wichtig sobald nur noch die Typesierten Missionsinformationen geladen werden sollen */
	void Daten_Lesen(string name)
	{
		//select  
		pstmt = con->prepareStatement(SQL_WÄHLE_ALLE_VON + MISSIONEN + SQL_WO + MISSIONSNAME +  "='" + name + "'");

		result = pstmt->executeQuery();

		while (result->next())
		{
			cout << "Gelesen von Tabelle " + MISSIONEN + " = bei x=" << result->getInt(2) << " | y=" << result->getInt(3) << endl;
			cout << "Die Mission \"" << result->getString(4).c_str() << ": " << result->getString(5).c_str() << "\"" << endl;
			cout << "hat die Aufgabe: \"" << result->getString(6).c_str() << "\" und die Belohnungen " << result->getInt(7) << " Ren und " << result->getInt(8) << " Exotics." << endl;
		}		
	}

	/* Es ist nur eine Testfunktion */
	void Daten_Updaten(int Menge, string name)
	{
		//update
		//pstmt = con->prepareStatement("UPDATE inventory SET quantity = ? WHERE name = ?");
		//pstmt->setInt(1, Menge);
		//pstmt->setString(2, name);
		//pstmt->executeQuery();
		cout << FUNKTIONSLOS << endl; //cout << "Zeile Geupdatet" << endl;
		;
	}
	
	/* Es ist nur eine Testfunktion */
	void Daten_Löschen(string name)
	{
		//delete
		//pstmt = con->prepareStatement("DELETE FROM inventory WHERE name = ?");
		//pstmt->setString(1, name);
		//result = pstmt->executeQuery();
		cout << FUNKTIONSLOS << endl; //cout << "Zeile gelöscht." << endl;
		;
	}
};

class FREETYPE
{
	FT_Library ft;
public:
	int Initialisieren()
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
};

int main()
{
	PRIVAT_MYSQL SQL;
	FREETYPE Schrift;

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}	

	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("5.1.transform.vs", "5.1.transform.fs");

	// compile and setup the shader
	// ----------------------------
	Shader shader("text.vs", "text.fs");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	Schrift.Initialisieren();

	//Setup Dear ImGui 
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	bool show_demo_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);



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
		0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
		0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int VBO, VAO, EBO;
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


	// load and create a texture 
	// -------------------------
	unsigned int texture1, texture2;

	create_Texture(texture1);
	create_Texture(texture2);

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use();
	ourShader.setInt("texture1", 0);
	ourShader.setInt("texture2", 1);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);
		
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{

			ImGui::Begin("Demo Interface Interaktive Map");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("Dies ist die Datensteuerung fuer\ndie Interaktive Karte.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

			static bool EInfügen = false;
			static bool Updaten = false;
			static bool Löschen = false;
			static bool Inventory_Caveentrace = false;		// Fakse = Inventory True = Caveentrace
			
			if (ImGui::Button("SQL Tabelle"))                           
				SQL.Tabelle_erstellen();

			if (ImGui::Button("SQL Einfuegen"))                            
				EInfügen = !EInfügen;
			if (EInfügen)
			{
				if (!Inventory_Caveentrace)
				{
					if (ImGui::Button("Einfuegen ist Inventory"))
						Inventory_Caveentrace = true;
				}
				else
					if (ImGui::Button("Einfuegen ist Caveentrace"))
						Inventory_Caveentrace = false;

				if(!Inventory_Caveentrace)
				{
					static char str0[128] = "Hello, world!";
					ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
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

					static int i0 = 123;
					ImGui::InputInt("input int", &i0);

					if (ImGui::Button("Einfuegen"))
					{
						SQL.Daten_Einfügen(string(str0), i0); EInfügen = false;
					}
				}
				else
				{
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
						SQL.Daten_Einfügen(ix,iy,str0,ic); EInfügen = false;
					}
				}
			}

			if (ImGui::Button("SQL Lesen"))                            
				SQL.Daten_Lesen();

			if (ImGui::Button("SQL Lesen2"))
			{
				Missionsliste = SQL.Daten_Lesen_Missionsname();
			}
			if (ImGui::Button("SQL Lesen3"))
			{
				SQL.Daten_Lesen(Missionsnamensuche);
			}

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

					static int i1 = 1000;
					ImGui::InputInt("input int", &i1);

				if (ImGui::Button("Updaten"))
				{
					SQL.Daten_Updaten(i1, string(str1));
					Updaten = false;
				}
			}
				
			if (ImGui::Button("SQL Loeschen"))
				Löschen = !Löschen;
			if (Löschen)
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
					SQL.Daten_Löschen(string(str2));
					Löschen = false;
				}					
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			// Liste der Missionen
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
							Missionsnamensuche = Missionsliste[i].first;		
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

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		//glClear(GL_COLOR_BUFFER_BIT);
		
		// create transformations
		glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		transform = glm::translate(transform, glm::vec3(-0.25f, 0.0f, 0.0f));
		//transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(1.5, 2.f, 1.0f));

		// get matrix's uniform location and set matrix
		ourShader.use();
		unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		// render container
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	
		RenderText_aktivieren();

		RenderText(shader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		RenderText(shader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

		RenderText_deaktivieren();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		

		

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
inline void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
inline void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// render line of text
// -------------------
inline void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
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