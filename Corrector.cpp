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
#include <stdlib.h>


#define countof(x) (sizeof(x) / sizeof((x)[0]))

int alphaRank[256];

void buildAlphabetRanking(const unsigned char* alphabet)
{
    // default: unknown chars go to end
    for (int i = 0; i < 256; i++)
        alphaRank[i] = 999;

    // assign order
    for (int i = 0; alphabet[i] != '\0'; i++)
        alphaRank[ alphabet[i] ] = i;
}

int cmpstr_custom(const void* a, const void* b)
{
    const unsigned char* s1 = (const unsigned char*)a;
    const unsigned char* s2 = (const unsigned char*)b;

    while (*s1 && *s2)
    {
        int r1 = alphaRank[*s1];
        int r2 = alphaRank[*s2];

        if (r1 != r2)
            return r1 - r2;

        s1++;
        s2++;
    }

    return (int)*s1 - (int)*s2;
}


bool yaExiste(char lista[][TAMTOKEN], int n, const char* pal)
{
    for (int i = 0; i < n; i++)
        if (strcmp(lista[i], pal) == 0)
            return true;
    return false;
}


bool esLetraEsp(char c)
{
	return  (c >= 'a' && c <= 'z') ||
		c == '�' ||
		c == '�' || c == '�' || c == '�' ||
		c == '�' || c == '�';
}

bool esDig(char c)
{
	return (c >= '0' && c <= '9');
}
void limpiarPalabra(char* p)
{
    int j = 0;

    for (int i = 0; p[i] != '\0'; i++)
    {
        unsigned char c = p[i];

        if (c >= 'A' && c <= 'Z')
            c = c - 'A' + 'a';

        bool letra = esLetraEsp(c);
        bool dig = esDig(c);

        // "/" posicion 1 y 0
        if (c == '/') {
            if (j < 2) p[j++] = c;
            continue;
        }

        // $ digito
        if (j == 0 && c == '$') { p[j++] = c; continue; }
        if (j == 1 && p[0] == '$' && dig) { p[j++] = c; continue; }
        if (p[0] == '$') continue;

        // # digitos
        if (j == 0 && c == '#') { p[j++] = c; continue; }
        if (j > 0 && p[0] == '#' && dig) { p[j++] = c; continue; }
        if (p[0] == '#') continue;

        // & solo
        if (j == 0 && c == '&') { p[j++] = c; continue; }
        if (j > 0 && p[0] == '&' && letra) { p[j++] = c; continue; }
        if (p[0] == '&') continue;

        // simbolos
        if (j == 0 && (c == '-' || c == '\'' || c == '"' || c == '!' || c == '+')) {
            p[j++] = c;
            continue;
        }

        // default
        if (letra || dig) {
            p[j++] = c;
            continue;
        }
    }

    p[j] = '\0';

    // final
    if (j >= 2 && p[0] == '/' && p[1] == '/')
    {
        int len = j - 2;
        if (len == 0 || len > 10) { p[0] = '\0'; return; }

        for (int k = 2; k < j; k++)
            if (!esLetraEsp(p[k])) {
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
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (strcmp(dic[i], dic[j]) > 0)
            {
                char tmp[TAMTOKEN];
                strcpy_s(tmp, TAMTOKEN, dic[i]);
                strcpy_s(dic[i], TAMTOKEN, dic[j]);
                strcpy_s(dic[j], TAMTOKEN, tmp);

                int t = est[i];
                est[i] = est[j];
                est[j] = t;
            }
}

int cmpstr(const void* a, const void* b)
{
    const char (*pa)[TAMTOKEN] = (const char (*)[TAMTOKEN])a;
    const char (*pb)[TAMTOKEN] = (const char (*)[TAMTOKEN])b;
    return strcmp(*pa, *pb);
}


void Diccionario(char* szNombre, char dic[][TAMTOKEN],
    int est[], int& n)
{
    FILE* fp;
    if (fopen_s(&fp, szNombre, "r") != 0) {
        printf("Archivo inv�lido\n");
        n = 0;
        return;
    }

    n = 0;
    char buffer[TAMTOKEN];

    while (fscanf_s(fp, "%s", buffer, (unsigned)countof(buffer)) != EOF)
    {
        limpiarPalabra(buffer);
        if (!buffer[0]) continue;

        int pos = -1;

        for (int i = 0; i < n; i++)
            if (strcmp(dic[i], buffer) == 0) { pos = i; break; }

        if (pos != -1)
            est[pos]++;
        else {
            strcpy_s(dic[n], TAMTOKEN, buffer);
            est[n] = 1;
            n++;
            if (n >= NUMPALABRAS) break;
        }
    }

    fclose(fp);
    ordenarDiccionario(dic, est, n);
}

void ListaCandidatas(
    char sug[][TAMTOKEN], int nSug,
    char dic[][TAMTOKEN], int est[],
    int nDic,
    char out[][TAMTOKEN], int peso[], int& nOut)
{
    nOut = 0;

    for (int i = 0; i < nSug; i++)
        for (int j = 0; j < nDic; j++)
            if (!strcmp(sug[i], dic[j]))
            {
                strcpy(out[nOut], dic[j]);
                peso[nOut] = est[j];
                nOut++;
            }

    // Peso
    for (int i = 0; i < nOut - 1; i++)
        for (int j = 0; j < nOut - i - 1; j++)
            if (peso[j] < peso[j + 1])
            {
                int tp = peso[j];
                peso[j] = peso[j + 1];
                peso[j + 1] = tp;

                char tmp[TAMTOKEN];
                strcpy(tmp, out[j]);
                strcpy(out[j], out[j + 1]);
                strcpy(out[j + 1], tmp);
            }
}
void ClonaPalabras(char* s, char out[][TAMTOKEN], int& nOut)
{
    int L = strlen(s);

    const unsigned char alphabet[] =
        "abcdefghijklmn"
        "\xA4"            // �
        "opqrstuvwxyz"
        "\xA0"            // �
        "\x82"            // �
        "\xA1"            // �
        "\xA2"            // �
        "\xA3";           // �

    int A = strlen((const char*)alphabet);

    // Build ranking table ONCE per call
    buildAlphabetRanking(alphabet);

    memset(out, 0, NUMPALABRAS * TAMTOKEN);
    nOut = 0;

    // Original word
    strcpy(out[nOut++], s);
    if (nOut >= NUMPALABRAS) return;

    // REMOVALS
    for (int pos = 0; pos < L; pos++)
    {
        char t[TAMTOKEN] = {0};
        int k = 0;

        for (int i = 0; i < L; i++)
            if (i != pos)
                t[k++] = s[i];

        strcpy(out[nOut++], t);
        if (nOut >= NUMPALABRAS) return;
    }

    // SWAPS
    for (int i = 0; i < L - 1; i++)
    {
        char t[TAMTOKEN] = {0};
        strcpy(t, s);

        char temp = t[i];
        t[i] = t[i + 1];
        t[i + 1] = temp;

        strcpy(out[nOut++], t);
        if (nOut >= NUMPALABRAS) return;
    }

    // REPLACEMENTS
    for (int pos = 0; pos < L; pos++)
        for (int a = 0; a < A; a++)
        {
            char t[TAMTOKEN];
            strcpy(t, s);
            t[pos] = alphabet[a];

            strcpy(out[nOut++], t);
            if (nOut >= NUMPALABRAS) return;
        }

    // INSERTIONS
    for (int a = 0; a < A; a++)
        for (int pos = 0; pos <= L; pos++)
        {
            char t[TAMTOKEN] = {0};
            int k = 0;

            // left part
            for (int i = 0; i < pos; i++)
                t[k++] = s[i];

            // insert
            t[k++] = alphabet[a];

            // right part
            for (int i = pos; i < L; i++)
                t[k++] = s[i];

            strcpy(out[nOut++], t);
            if (nOut >= NUMPALABRAS) return;
        }

    // ONLY ONE SORT - custom
    qsort(out, nOut, sizeof out[0], cmpstr_custom);
}






