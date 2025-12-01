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

        // "/" allowed only in positions 0 and 1
        if (c == '/') {
            if (j < 2) p[j++] = c;
            continue;
        }

        // $ + 1 digit
        if (j == 0 && c == '$') { p[j++] = c; continue; }
        if (j == 1 && p[0] == '$' && dig) { p[j++] = c; continue; }
        if (p[0] == '$') continue;

        // # + digits
        if (j == 0 && c == '#') { p[j++] = c; continue; }
        if (j > 0 && p[0] == '#' && dig) { p[j++] = c; continue; }
        if (p[0] == '#') continue;

        // & alone or with letters
        if (j == 0 && c == '&') { p[j++] = c; continue; }
        if (j > 0 && p[0] == '&' && letra) { p[j++] = c; continue; }
        if (p[0] == '&') continue;

        // allowed standalone symbols
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

    // final rule for //xxxx
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

    // sort by weight (bubble)
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
    const char alphabet[] = "abcdefghijklmn�opqrstuvwxyz�����";
    int A = strlen(alphabet);

    // insertions
    for (int a = 0; a < A; a++)
        for (int pos = 0; pos <= L; pos++)
        {
            char t[TAMTOKEN];
            strncpy(t, s, pos);
            t[pos] = alphabet[a];
            strcpy(t + pos + 1, s + pos);

            // ? NEW: only save if not duplicate
            if (!yaExiste(out, nOut, t))
            {
                strcpy(out[nOut++], t);
                if (nOut >= NUMPALABRAS) return;
            }
        }

    // replacements
    for (int pos = 0; pos < L; pos++)
        for (int a = 0; a < A; a++)
        {
            char t[TAMTOKEN];
            strcpy(t, s);
            t[pos] = alphabet[a];

            if (!yaExiste(out, nOut, t))
            {
                strcpy(out[nOut++], t);
                if (nOut >= NUMPALABRAS) return;
            }
        }

    // swaps
    for (int i = 0; i < L - 1; i++)
    {
        char t[TAMTOKEN];
        strcpy(t, s);
        char tmp = t[i]; t[i] = t[i + 1]; t[i + 1] = tmp;

        if (!yaExiste(out, nOut, t))
        {
            strcpy(out[nOut++], t);
            if (nOut >= NUMPALABRAS) return;
        }
    }

    // removals
    for (int pos = 0; pos < L; pos++)
    {
        char t[TAMTOKEN];
        strncpy(t, s, pos);
        strcpy(t + pos, s + pos + 1);

        if (!yaExiste(out, nOut, t))
        {
            strcpy(out[nOut++], t);
            if (nOut >= NUMPALABRAS) return;
        }
    }
}




