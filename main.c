#include <math.h>
#include <stdio.h>
#include <unistd.h>

#define F_BLACK "\x1b[30m"
#define F_RED "\x1b[31m"
#define F_GREEN "\x1b[32m"
#define F_YELLOW "\x1b[33m"
#define F_BLUE "\x1b[34m"
#define F_MAGENTA "\x1b[35m"
#define F_CYAN "\x1b[36m"
#define F_WHITE "\x1b[37m"

#define F_BRIGHTBLACK "\x1b[90m"
#define F_BRIGHTRED "\x1b[91m"
#define F_BRIGHTGREEN "\x1b[92m"
#define F_BRIGHTYELLOW "\x1b[93m"
#define F_BRIGHTBLUE "\x1b[94m"
#define F_BRIGHTMAGENTA "\x1b[95m"
#define F_BRIGHTCYAN "\x1b[96m"
#define F_BRIGHTWHITE "\x1b[97m"

#define F_NORMAL "\x1b[m"

#define B_BLACK "\x1b[40m"
#define B_RED "\x1b[41m"
#define B_GREEN "\x1b[42m"
#define B_YELLOW "\x1b[43m"
#define B_BLUE "\x1b[44m"
#define B_MAGENTA "\x1b[45m"
#define B_CYAN "\x1b[46m"
#define B_WHITE "\x1b[47m"

#define B_BRIGHTBLACK "\x1b[100m"
#define B_BRIGHTRED "\x1b[101m"
#define B_BRIGHTGREEN "\x1b[102m"
#define B_BRIGHTYELLOW "\x1b[103m"
#define B_BRIGHTBLUE "\x1b[104m"
#define B_BRIGHTMAGENTA "\x1b[105m"
#define B_BRIGHTCYAN "\x1b[106m"
#define B_BRIGHTWHITE "\x1b[107m"

struct pixel {
    char symbol;
    unsigned char bRGB;
    //using last 4 bits, which indicate, in order, brightness, redness, greeness, blueness
};

void colorizePixel(char stringBuffer[], unsigned char bRGB, int i) {
    for (int k = 0; k < 5; k++) {
        stringBuffer[i * 6 - 6 + k] = "\x1b[96m"[k];
    }

    unsigned char brightness = bRGB >> 3;
    stringBuffer[i * 6 - 3] = brightness ? 9 : 3;

    unsigned char rgb = (bRGB & 0b00000111);
    switch (rgb) {
        case 0: stringBuffer[i * 6 - 2] = 0; break;
        case 2: stringBuffer[i * 6 - 2] = 2; break;
        case 5: stringBuffer[i * 6 - 2] = 5; break;
        case 1: stringBuffer[i * 6 - 2] = 4; break;
        case 3: stringBuffer[i * 6 - 2] = 6; break;
        case 4: stringBuffer[i * 6 - 2] = 1; break;
        case 6: stringBuffer[i * 6 - 2] = 3; break;
        default: printf("DEBUG: (bRGB & 0b00000111) = %d\n", rgb); break;
    }
}


void showImage(struct pixel canvas[40][156]) {

    for (int j = 0; j < 40; j++) {
        char stringBuffer[936] = {0};
        for (int i = 1; i <= 156; i++) {
            stringBuffer[i * 6 - 1] = canvas[j][i - 1].symbol;
            colorizePixel(stringBuffer, canvas[j][i - 1].bRGB, i);
        }
        fwrite(stringBuffer, 936, 1, stdout);
    }
}


void circleEdge(struct pixel canvas[40][156]) {
    for (int j = 0; j < 40; j++) {
        for (int i = 0; i < 156; i++) {
            int iFromOrigin = i - 78;
            int jFromOrigin = j * (5 / 2) - 20 * (5 / 2);
            int distFromOriginSquared = iFromOrigin * iFromOrigin + jFromOrigin * jFromOrigin;

            if (distFromOriginSquared < 2035 && distFromOriginSquared > 1950) {
                canvas[j][i].symbol = '^';
                canvas[j][i].bRGB = 0b00001110;
            }
            else
                canvas[j][i].symbol = '.';
            canvas[j][i].bRGB = 0b00001110;
        }
    }
}

void backgroundFill(struct pixel canvas[40][156]) {
    //MULTIPLY X BY RATIO TO GET ACCURATE LENGTH 5:2
    //ORIGIN IS (78,20)
    for (int j = 0; j < 40; j++) {
        for (int i = 0; i < 156; i++) {
            int iFromOrigin = i - 78;
            int jFromOrigin = j * (5 / 2) - 20 * (5 / 2);
            int distFromOriginSquared = iFromOrigin * iFromOrigin + jFromOrigin * jFromOrigin;

            if (distFromOriginSquared > 2000) {
                if (i % 2 == 0)
                    canvas[j][i].symbol = '^';
                else
                    canvas[j][i].symbol = '-';

                canvas[j][i].bRGB = 0b00001110;
            }
        }
    }
}

void initializeCanvas(struct pixel canvas[40][156]) {
    for (int i = 0; i < 156; i++) {
        for (int j = 0; j < 40; j++) {
            canvas[j][i].symbol = '+';
            canvas[j][i].bRGB = 0b00001110;
        }
    }
}

void drawOutput(int iheight) {
    struct pixel canvas[40][156]; //156x40 = pixel position

    system("cls");
    initializeCanvas(canvas);
    backgroundFill(canvas);
    circleEdge(canvas);
    showImage(canvas);
}

int main(void) {
    float height = -1;
    int iheight = 0;

    while (1) {
        printf("Veuillez saisir la hauteur de la pyramide : \n");
        scanf("%f", &height);

        iheight = (int) height;
        if (height > 1 && height == iheight) {
            break;
        }

        printf("\n\t<Veuillez saisir un nombre valide et reessayer.>\n\n");
        fflush(stdin);
        height = -1;
    }

    drawOutput(iheight);
    system("pause");
    return 0;
}
