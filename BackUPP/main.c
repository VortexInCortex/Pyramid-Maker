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

#define B_BRIGHTBLACK "\x1b[90m"
#define B_BRIGHTRED "\x1b[91m"
#define B_BRIGHTGREEN "\x1b[92m"
#define B_BRIGHTYELLOW "\x1b[93m"
#define B_BRIGHTBLUE "\x1b[4m"
#define B_BRIGHTMAGENTA "\x1b[5m"
#define B_BRIGHTCYAN "\x1b[6m"
#define B_BRIGHTWHITE "\x1b[7m"

void printSpaces(int spaceAmount) {
    for (int j = 1; j < spaceAmount; j++) {
        printf(" ");
    }
}

void printStars(int starAmount) {
    for (int j = 1; j <= starAmount; j++) {
        printf("*");
    }
}

void outputPyramid(int iheight) {
    system("cls");
    for (int j = 0; j < iheight; j++) {
        printSpaces((iheight - j) * 2);

        if (j != 0) {
            printf("%s",F_BRIGHTRED);
            printStars(1);
            printf("%s", F_WHITE);
        }

        if (j > 0 && j < (iheight + 8) / (8 + iheight / 16))
            printf("%s", F_BRIGHTYELLOW);

        printStars(j * 2 - 1);

        if (j == 0)
            printf("%s", F_BRIGHTRED);
        printStars(1);
        printf("%s", F_BRIGHTBLACK);

        if (j > 0 && j < (iheight + 8) / (8 + iheight / 16))
            printf("%s", F_YELLOW);

        printStars(j * 2 - 1);

        if (j != 0) {
            printf("%s", F_RED);
            printStars(1);
        }

        printf("\n");
    }
}

struct pixel {
    char symbol;
    unsigned char bRGB;
    //using last 4 bits, which indicate, in order, brightness, redness, greeness, blueness
};

void showImage(struct pixel canvas[40][156]) {
    printf("%s",F_BRIGHTBLACK);
    // for (int j = 0; j < 40; j++) {
    //     for (int i = 0; i < 156; i++) {
    //         printf("%c", canvas[i][j].symbol);
    //     }
    // }
    for (int i = 0; i < 156; i++) {
        fwrite(canvas[i], 40, 1, stdout);
        printf("\n");
    }
}

void firstPass(struct pixel canvas[40][156]) {
    //MULTIPLY X BY RATIO TO GET ACCURATE LENGTH 4:3
    //ORIGIN IS (78,20)
    for (int i = 0; i < 156; i++) {
        for (int j = 0; j < 40; j++) {
            int iFromOrigin = i - 78;
            int jFromOrigin = j * (5 / 2) - 20 * (5 / 2);
            int distFromOriginSquared = iFromOrigin * iFromOrigin + jFromOrigin * jFromOrigin;

            if (distFromOriginSquared > 2000) {
                if (j % 2 == 1)
                    canvas[i][j].symbol = '.';
                    // else if ((i % 2 == 0 && i < 78 && j > 20) || (i % 2 == 0 && i > 78 && j < 20))
                    //     canvas[i][j].symbol = '\\';
                    // else if ((i % 2 == 0 && i < 78 && j < 20) || (i % 2 == 0 && i > 78 && j > 20))
                    //     canvas[i][j].symbol = '/';
                else if (i % 2 == 0)
                    canvas[i][j].symbol = '~';
                else
                    canvas[i][j].symbol = '.';

                canvas[i][j].bRGB = 0b00001000;
            } else
                canvas[i][j].symbol = '.';
        }
    }
}

void initializeCanvas(struct pixel canvas[40][156]) {
    for (int i = 0; i < 156; i++) {
        for (int j = 0; j < 40; j++) {
            canvas[i][j].symbol = '=';
            canvas[i][j].bRGB = 0;
        }
    }
}

void drawOutput(int iheight) {
    struct pixel canvas[40][156]; //156x40 = pixel position

    system("cls");
    initializeCanvas(canvas);
    firstPass(canvas);
    showImage(canvas);
}

int main(void) {
    // char s[] = "Je suis une chaine de caracteres !";
    // printf("%s", s);
    // fwrite(s, strlen(s), 1, stdout);
    //
    // system("pause");
    // return 0;

    float height = -1;
    int iheight = 0;

    while (1) {
        printf("Veuillez saisir la height de la pyramide : \n");
        scanf("%f", &height);

        iheight = (int) height;
        if (height > 1 && height == iheight) {
            break;
        }

        printf("\n\t<Veuillez saisir un nombre valide et reessayer.>\n\n");
        fflush(stdin);
        height = -1;
    }

    outputPyramid(iheight);
    // CANVAS IS 156x40 PIXELS -> MAX PYRAMID BASE = 90 -> 2 lines at bottom and top reserved for tv-effect + 4 side to side
    // AND COVER THE EDGES OF IN MINIMALIST SANDY DUNES RANDOMLY GENERATED + Cactus if possible
    // + DARK MAGENTA AND RED BACKGROUD OF CLOUDS/SUN THAT TURNS IN PARALLAX WITH TURNING OF SKY?

    system("pause");

    drawOutput(iheight);
    system("pause");
    return 0;
}
