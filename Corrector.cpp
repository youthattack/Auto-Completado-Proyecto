/*****************************************************************************************************************
	UNIVERSIDAD NACIONAL AUTONOMA DE MEXICO
	FACULTAD DE ESTUDIOS SUPERIORES -ARAGON-

	Computadoras y programacion. 
	(c) Ponga su nombre y numero de cuenta aqui.
	
	Quiso decir: Programa principal de la aplicacion de la distancia de Levenstein.
	
******************************************************************************************************************/
#include "stdafx.h"
#include <string.h>
#include "corrector.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

void limpiarPalabra(char* p)
{
    int j = 0;
    unsigned char c;

    for (int i = 0; p[i] != '\0'; i++)
    {
        c = (unsigned char)p[i];

        // Convert to lowercase
        if (c >= 'A' && c <= 'Z')
            c += ('a' - 'A');

        // Spanish letters
        bool esLetra =
            (c >= 'a' && c <= 'z') ||
            c == 0xF1 || // ñ
            c == 0xE1 || c == 0xE9 || c == 0xED || c == 0xF3 || c == 0xFA; // áéíóú

        bool esDigito = (c >= '0' && c <= '9');

        //----------------------------------------------------------------------
        // RULE: allow "/" only in positions 0 and 1 (for //xxx)
        //----------------------------------------------------------------------
        if (c == '/' && j < 2) {
            p[j++] = c;
            continue;
        }
        if (c == '/')
            continue; // any slash after pos 2 is illegal

        //----------------------------------------------------------------------
        // RULE: $ + ONE digit   (accept $5, reject $5000)
        //----------------------------------------------------------------------
        if (j == 0 && c == '$') {
            p[j++] = c;
            continue;
        }
        if (j == 1 && p[0] == '$' && esDigito) {
            p[j++] = c;
            continue;
        }
        if (p[0] == '$')
            continue; // reject anything after $digit

        //----------------------------------------------------------------------
        // RULE: # + digits only
        //----------------------------------------------------------------------
        if (j == 0 && c == '#') {
            p[j++] = c;
            continue;
        }
        if (j > 0 && p[0] == '#' && esDigito) {
            p[j++] = c;
            continue;
        }
        if (p[0] == '#')
            continue;

        //----------------------------------------------------------------------
        // RULE: & alone, or & + letters (like &c)
        //----------------------------------------------------------------------
        if (j == 0 && c == '&') {
            p[j++] = c;
            continue;
        }
        if (j > 0 && p[0] == '&' && esLetra) {
            p[j++] = c;
            continue;
        }
        if (p[0] == '&')
            continue;

        //----------------------------------------------------------------------
        // RULE: Allowed standalone symbols
        //       '-', "'", '"', '!', '&', '+'
        //----------------------------------------------------------------------
        if (j == 0 && (c == '-' || c == '\'' || c == '"' || c == '!' || c == '+')) {
            p[j++] = c;
            continue;
        }

        //----------------------------------------------------------------------
        // RULE: default — accept normal letters and digits
        //----------------------------------------------------------------------
        if (esLetra || esDigito) {
            p[j++] = c;
            continue;
        }

        // Anything else → reject char
    }

    p[j] = '\0';

    //======================================================================
    // FINAL URL RULE (REPLACES YOUR OLD BLOCK)
    //
    // ACCEPT ONLY EXACT tokens:
    //   //www
    //   //pglaf
    //   //gutenberg
    //   //gallica
    //
    // ANY OTHER TOKEN STARTING WITH "//" IS REJECTED.
    //======================================================================
    if (j >= 2 && p[0] == '/' && p[1] == '/')
    {
        const char* ok[] = {
            "//www",
            "//pglaf",
            "//gutenberg",
            "//gallica"
        };
        int allowed = 0;

        for (int k = 0; k < 4; k++) {
            if (strcmp(p, ok[k]) == 0) {
                allowed = 1;
                break;
            }
        }

        if (!allowed) {
            p[0] = '\0';
            return;
        }
    }
}

int existePalabra(char dic[][TAMTOKEN], int n, const char* p)
{
    for (int i = 0; i < n; i++)
        if (!strcmp(dic[i], p))
            return 1;
    return 0;
}

void ordenarDiccionario(char dic[][TAMTOKEN], int est[], int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (strcmp(dic[i], dic[j]) > 0) {

                char tmp[TAMTOKEN];
                strcpy_s(tmp, TAMTOKEN, dic[i]);
                strcpy_s(dic[i], TAMTOKEN, dic[j]);
                strcpy_s(dic[j], TAMTOKEN, tmp);

                int t = est[i];
                est[i] = est[j];
                est[j] = t;
            }
        }
    }
}

void Diccionario(char* szNombre, char dic[][TAMTOKEN],
    int est[], int& n)
{
    FILE* fp;
    if (fopen_s(&fp, szNombre, "r") != 0) {
        printf("Archivo inválido\n");
        n = 0;
        return;
    }

    n = 0;
    char buffer[TAMTOKEN];

    while (fscanf_s(fp, "%s", buffer, (unsigned)countof(buffer)) != EOF)
    {
        limpiarPalabra(buffer);

        if (buffer[0] == '\0')
            continue;

        int pos = -1;

        // BUSCAR si la palabra ya existe en el diccionario
        for (int i = 0; i < n; i++) {
            if (strcmp(dic[i], buffer) == 0) {
                pos = i;
                break;
            }
        }

        if (pos != -1) {
            // EXISTE → incrementar frecuencia
            est[pos]++;
        }
        else {
            // NO EXISTE → agregar al diccionario
            strcpy_s(dic[n], TAMTOKEN, buffer);
            est[n] = 1;      // ← MUY IMPORTANTE
            n++;

            if (n >= NUMPALABRAS)
                break;
        }
    }

    fclose(fp);

    // ordenar al final
    ordenarDiccionario(dic, est, n);
}

/*****************************************************************************************************************
	ListaCandidatas: Esta funcion recupera desde el diccionario las palabras validas y su peso
	Regresa las palabras ordenadas por su peso
	char	szPalabrasSugeridas[][TAMTOKEN],	//Lista de palabras clonadas
	int		iNumSugeridas,						//Lista de palabras clonadas
	char	szPalabras[][TAMTOKEN],				//Lista de palabras del diccionario
	int		iEstadisticas[],					//Lista de las frecuencias de las palabras
	int		iNumElementos,						//Numero de elementos en el diccionario
	char	szListaFinal[][TAMTOKEN],			//Lista final de palabras a sugerir
	int		iPeso[],							//Peso de las palabras en la lista final
	int &	iNumLista)							//Numero de elementos en la szListaFinal
******************************************************************************************************************/
void	ListaCandidatas		(
	char	szPalabrasSugeridas[][TAMTOKEN],	//Lista de palabras clonadas
	int		iNumSugeridas,						//Lista de palabras clonadas
	char	szPalabras[][TAMTOKEN],				//Lista de palabras del diccionario
	int		iEstadisticas[],					//Lista de las frecuencias de las palabras
	int		iNumElementos,						//Numero de elementos en el diccionario
	char	szListaFinal[][TAMTOKEN],			//Lista final de palabras a sugerir
	int		iPeso[],							//Peso de las palabras en la lista final
	int &	iNumLista)							//Numero de elementos en la szListaFinal
{

	//Sustituya estas lineas por su c�digo
	strcpy_s(szListaFinal[0], szPalabrasSugeridas[ 0] ); //la palabra candidata
	iPeso[0] = iEstadisticas[0];			// el peso de la palabra candidata
	
	iNumLista = 1;							//Una sola palabra candidata
}

/*****************************************************************************************************************
	ClonaPalabras: toma una palabra y obtiene todas las combinaciones y permutaciones requeridas por el metodo
	char *	szPalabraLeida,						// Palabra a clonar
	char	szPalabrasSugeridas[][TAMTOKEN], 	//Lista de palabras clonadas
	int &	iNumSugeridas)						//Numero de elementos en la lista
******************************************************************************************************************/
void	ClonaPalabras(
	char *	szPalabraLeida,						// Palabra a clonar
	char	szPalabrasSugeridas[][TAMTOKEN], 	//Lista de palabras clonadas
	int &	iNumSugeridas)						//Numero de elementos en la lista
{
	//Sustituya estas lineas por su c�digo
	strcpy_s(szPalabrasSugeridas[0], szPalabraLeida); //lo que sea que se capture, es sugerencia
	iNumSugeridas = 1;							//Una sola palabra sugerida
}
