#pragma once
#ifndef Interaktive_Karte
#define Interaktive_Karte

#include <string>

#include <stdlib.h>
#include <stdexcept>
#include <stb_image.h>

#include <iostream>
#include <sstream>

#include <map>
#include <vector>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

/* Makros */

#define HILFSMARKER ImGui::SameLine(); HelpMarker("USER:\n""Hold SHIFT or use mouse to select text.\n""CTRL+Left/Right to word jump.\n""CTRL+A or double-click to select all.\n""CTRL+X,CTRL+C,CTRL+V clipboard.\n""CTRL+Z,CTRL+Y undo/redo.\n""ESCAPE to revert.\n");

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
#define BENUTZERVERWALTUNG string("`Benutzerverwaltung`")
#define MISSIONEN string("`mission`")


#define HTML_END_COLUM string("\r\n")

/* Klassen - Strukturen */

/* Klasse für die Vereinfachte X/Y Koordinaten innerhalb des Programms - bei Erweiterung sollte es auf glm::vec2 Basis aufgebaut werden*/
class Koordinaten
{

public:
	float X, Y; // Variable für die Positionierung - Für eine Vereinfachung kann es zu glm::vec2 gewandelt werden
	Koordinaten() {};
	Koordinaten(float X, float Y) : X(X), Y(Y) {};
	Koordinaten operator() (float X, float Y) { this->X = X; this->Y = Y; return *this; };
	float getX() { return X; };
	float getY() { return Y; };
};

/* Hilfsfunktionen */

#endif