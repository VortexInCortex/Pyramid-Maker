#include <stdio.h>
#include <conio.h>
#include <windows.h>
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

char buffer[40000] = {0};
double el_initializeCanvas, el_bFill, el_bCircleEdge, el_bSun, el_bDunes, el_fBloomTriangle, el_fOutlineTriangle, el_fBackFaceTriangle, el_fSideFaceTriangle,
        el_fFrontFaceTriangle, el_showImage, el_total, el_timeBetweenFrames = 0;
LARGE_INTEGER start, end, freq, startTotal, endTotal;

void startTimer() {
    QueryPerformanceFrequency(&freq); // Get the frequency of the high-resolution timer
    QueryPerformanceCounter(&start); // Get the current counter value (start time)
}

void stopTimer() {
    QueryPerformanceCounter(&end); // Get the current counter value (end time)
}

double getElapsedTimeInMicroseconds() {
    return (double) (end.QuadPart - start.QuadPart) * 1000000.0 / freq.QuadPart;
}

void startTimerTotal() {
    QueryPerformanceFrequency(&freq); // Get the frequency of the high-resolution timer
    QueryPerformanceCounter(&startTotal); // Get the current counter value (start time)
}

void stopTimerTotal() {
    QueryPerformanceCounter(&endTotal); // Get the current counter value (end time)
}

double getElapsedTimeInMicrosecondsTotal() {
    return (double) (endTotal.QuadPart - startTotal.QuadPart) * 1000000.0 / freq.QuadPart;
}

struct pixel {
    char symbol;
    unsigned char bRGB;
};

void adjustFrameRate(float targetFrameRate) {
    stopTimerTotal();
    el_timeBetweenFrames = getElapsedTimeInMicrosecondsTotal();
    usleep(1000000.0f / targetFrameRate - el_timeBetweenFrames);
    startTimerTotal();
}

void colorizePixel(char stringBuffer[], unsigned char bRGB, int j, int i) {
    for (int k = 1; k <= 5; k++) {
        stringBuffer[(j * 936) + (i * 6 - 6 + k - 1)] = "\x1b[96m"[k - 1];
    }

    unsigned char brightness = bRGB >> 3;
    stringBuffer[(j * 936) + (i * 6 - 4)] = brightness ? '9' : '3';

    unsigned char rgb = (bRGB & 0b00000111);
    switch (rgb) {
        case 0: stringBuffer[(j * 936) + (i * 6 - 3)] = '0';
            break;
        case 1: stringBuffer[(j * 936) + (i * 6 - 3)] = '4';
            break;
        case 2: stringBuffer[(j * 936) + (i * 6 - 3)] = '2';
            break;
        case 3: stringBuffer[(j * 936) + (i * 6 - 3)] = '6';
            break;
        case 4: stringBuffer[(j * 936) + (i * 6 - 3)] = '1';
            break;
        case 5: stringBuffer[(j * 936) + (i * 6 - 3)] = '5';
            break;
        case 6: stringBuffer[(j * 936) + (i * 6 - 3)] = '3';
            break;
        case 7: stringBuffer[(j * 936) + (i * 6 - 3)] = '7';
            break;
        default: printf("DEBUG: (bRGB & 0b00000111) = %d\n", rgb);
            fflush(stdout);
            break;
    }
}

void showImage(struct pixel canvas[41][156]) {
    startTimer();

    char stringBuffer[38376] = {0};
    for (int j = 0; j < 41; j++) {
        for (int i = 1; i <= 156; i++) {
            stringBuffer[(j * 936) + (i * 6 - 1)] = canvas[j][i - 1].symbol;
            colorizePixel(stringBuffer, canvas[j][i - 1].bRGB, j, i);
        }
    }
    write(1, "\x1b[H", 7); // 1 being stdout
    fwrite(stringBuffer, 38376, 1, stdout);
    fflush(stdout);

    stopTimer();
    el_showImage = getElapsedTimeInMicroseconds();
}

typedef struct {
    int x;
    int y;
} Point;

void frontFaceIfBlock(struct pixel canvas[41][156], int iheight, int j, int i) {
    if (j > (41 - iheight + iheight / 10)) {
        if (canvas[j][i - 3].symbol == '>' || canvas[j][i - 4].symbol == '>'
            || canvas[j][i + 3].symbol == '<' || canvas[j][i + 4].symbol == '<') {
            canvas[j][i].symbol = 'I';
            canvas[j][i].bRGB = 0b0111;
        } else if ((i + (j % 3) * 3) % 9 == 0) {
            canvas[j][i].symbol = 'I';
            canvas[j][i].bRGB = 0b0111;
        } else {
            canvas[j][i].symbol = '_';
            canvas[j][i].bRGB = 0b0111;
        }
    } else if (j == 41 - iheight) {
        canvas[j][i].symbol = '^';
        canvas[j][i].bRGB = 0b1110;
    } else {
        if (canvas[j][i - 3].symbol == '>' || (canvas[j][i - 3].symbol == '\\' /*&& canvas[j][i - 3].bRGB == '0b1110'// WHY THIS CHECK*/) ||
            canvas[j][i + 3].symbol == '<' || (canvas[j][i + 3].symbol == '/' /*&& canvas[j][i + 3].bRGB == '0b1110'//FAIL?*/)) {
            if (i < 78)
                canvas[j][i].symbol = '/';
            else
                canvas[j][i].symbol = '\\';
            canvas[j][i].bRGB = 0b1110;
        } else {
            canvas[j][i].symbol = '#';
            canvas[j][i].bRGB = 0b1110;
        }
    }
}

void sideFaceIfBlock(struct pixel canvas[41][156], int iheight, int j, int i) {
    if (j > (41 - iheight + iheight / 10)) {
        if (canvas[j][i - 3].symbol == '>' || canvas[j][i - 4].symbol == '>'
            || canvas[j][i + 3].symbol == '<' || canvas[j][i + 4].symbol == '<') {
            canvas[j][i].symbol = 'I';
            canvas[j][i].bRGB = 0b0101;
        } else if ((i + (j % 3) * 3) % 9 == 0) {
            canvas[j][i].symbol = 'I';
            canvas[j][i].bRGB = 0b0101;
        } else {
            canvas[j][i].symbol = '_';
            canvas[j][i].bRGB = 0b0101;
        }
    } else if (j == 41 - iheight) {
        canvas[j][i].symbol = '^';
        canvas[j][i].bRGB = 0b1101;
    } else {
        if (canvas[j][i - 3].symbol == '>' || (canvas[j][i - 3].symbol == '\\' /*&& canvas[j][i - 3].bRGB == '0b1110'// WHY THIS CHECK*/) ||
            canvas[j][i + 3].symbol == '<' || (canvas[j][i + 3].symbol == '/' /*&& canvas[j][i + 3].bRGB == '0b1110'//FAIL?*/)) {
            if (i < 78)
                canvas[j][i].symbol = '/';
            else
                canvas[j][i].symbol = '\\';
            canvas[j][i].bRGB = 0b1101;
        } else {
            canvas[j][i].symbol = '#';
            canvas[j][i].bRGB = 0b1101;
        }
    }
}

void backFaceIfBlock(struct pixel canvas[41][156], int iheight, int j, int i) {
    if (j > (41 - iheight + iheight / 10)) {
        if (canvas[j][i - 3].symbol == '>' || canvas[j][i - 4].symbol == '>'
            || canvas[j][i + 3].symbol == '<' || canvas[j][i + 4].symbol == '<') {
            canvas[j][i].symbol = 'I';
            canvas[j][i].bRGB = 0b0001;
        } else if ((i + (j % 3) * 3) % 9 == 0) {
            canvas[j][i].symbol = 'I';
            canvas[j][i].bRGB = 0b0001;
        } else {
            canvas[j][i].symbol = '_';
            canvas[j][i].bRGB = 0b0001;
        }
    } else if (j == 41 - iheight) {
        canvas[j][i].symbol = '^';
        canvas[j][i].bRGB = 0b1001;
    } else {
        if (canvas[j][i - 3].symbol == '>' || (canvas[j][i - 3].symbol == '\\' /*&& canvas[j][i - 3].bRGB == '0b1110'// WHY THIS CHECK*/) ||
            canvas[j][i + 3].symbol == '<' || (canvas[j][i + 3].symbol == '/' /*&& canvas[j][i + 3].bRGB == '0b1110'//FAIL?*/)) {
            if (i < 78)
                canvas[j][i].symbol = '/';
            else
                canvas[j][i].symbol = '\\';
            canvas[j][i].bRGB = 0b1001;
        } else {
            canvas[j][i].symbol = '#';
            canvas[j][i].bRGB = 0b1001;
        }
    }
}

void outlineIfBlock(struct pixel canvas[41][156], int iheight, int j, int i) {
    canvas[j][i].symbol = '*';
    canvas[j][i].bRGB = 0b1110;
}

void bloomIfBlock(struct pixel canvas[41][156], int iheight, int j, int i) {
    if (j == 39 - iheight && i > 77) {
        canvas[j][i].symbol = '|';
        canvas[j][i].bRGB = 0b1110;
    } else if (j < (42 - iheight + iheight / 10) && j != (39 - iheight))
        if (i < 78) {
            canvas[j][i].symbol = '\\';
            canvas[j][i].bRGB = 0b1110;
        } else {
            canvas[j][i].symbol = '/';
            canvas[j][i].bRGB = 0b1110;
        }
    else if (i < 78 && j != (39 - iheight)) {
        canvas[j][i].symbol = '>';
        canvas[j][i].bRGB = 0b1110;
    } else if (j != (39 - iheight)) {
        canvas[j][i].symbol = '<';
        canvas[j][i].bRGB = 0b1110;
    }
}

void fillTriangle(Point v1, Point v2, Point v3, struct pixel canvas[41][156], int iheight, int ifBlock) {
    //Scanline algorithm using y=mx+b
    float slopeLeft = (v2.y - v1.y) / (v2.x - v1.x);
    float slopeRight = (v3.y - v2.y) / (v3.x - v2.x);

    float offsetLeft = v1.y;
    float offsetRight = v2.y - slopeRight * v2.x;

    for (int j = v2.y; j < v1.y; j++) {
        int xLeft = (j - offsetLeft) / slopeLeft;
        int xRight = (j - offsetRight) / slopeRight;

        for (int i = xLeft; i < xRight; i++) {
            switch (ifBlock) {
                case 0: frontFaceIfBlock(canvas, iheight, j, i);
                    break;
                case 1: sideFaceIfBlock(canvas, iheight, j, i);
                    break;
                case 2: backFaceIfBlock(canvas, iheight, j, i);
                    break;
                case 3: outlineIfBlock(canvas, iheight, j, i);
                    break;
                case 4: bloomIfBlock(canvas, iheight, j, i);
                    break;
                default: printf("DEBUG: BAD IF BLOCK CASE. ifBlock = %i", ifBlock);
            }
        }
    }
}

void fFrontFaceTriangle(struct pixel canvas[41][156], int iheight, unsigned int frameCounter) {
    startTimer();

    // Vertices
    Point A = {78, 41 - iheight};
    Point B = {78 - (iheight * 2), 41};
    Point C = {78 + (iheight * 2) - (frameCounter * 8 / (41 - iheight)), 41};

    fillTriangle(B, A, C, canvas, iheight, 0);

    stopTimer();
    el_fFrontFaceTriangle = getElapsedTimeInMicroseconds();
}

void fSideFaceTriangle(struct pixel canvas[41][156], int iheight, unsigned int frameCounter) {
    startTimer();

    // Vertices
    Point A = {78, 41 - iheight};
    Point B = {78 - (iheight * 2), 41};
    Point C = {78 + (iheight * 2), 41};

    fillTriangle(B, A, C, canvas, iheight, 1);

    stopTimer();
    el_fSideFaceTriangle = getElapsedTimeInMicroseconds();
}

void fBackFaceTriangle(struct pixel canvas[41][156], int iheight) {
    startTimer();

    // Vertices
    Point A = {78, 41 - iheight};
    Point B = {78 - (iheight * 2), 41};
    Point C = {78 + (iheight * 2), 41};

    fillTriangle(B, A, C, canvas, iheight, 2);

    stopTimer();
    el_fBackFaceTriangle = getElapsedTimeInMicroseconds();
}

void fOutlineTriangle(struct pixel canvas[41][156], int iheight) {
    startTimer();

    // Vertices
    Point A = {78, 40 - iheight};
    Point B = {78 - (iheight * 2) - 1, 40};
    Point C = {78 + (iheight * 2) + 1, 40};

    fillTriangle(B, A, C, canvas, iheight, 3);

    stopTimer();
    el_fOutlineTriangle = getElapsedTimeInMicroseconds();
}

void fBloomTriangle(struct pixel canvas[41][156], int iheight) {
    startTimer();

    // Vertices
    Point A = {77, 39 - iheight};
    Point B = {77 - (iheight * 2) - 3, 41};
    Point C = {77 + (iheight * 2) + 3, 41};

    fillTriangle(B, A, C, canvas, iheight, 4);

    stopTimer();
    el_fBloomTriangle = getElapsedTimeInMicroseconds();
}

int sin(int x) {
    int sinTable[] = {
        0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 0, -1, -1, -1, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3, -3, -3, -3, -2, -2, -2,
        -2, -1, -1, -1, 0
    };
    return sinTable[(sizeof(sinTable) / 4 - x) % (sizeof(sinTable) / 4)];
}

void bDunes(struct pixel canvas[41][156], int iheight) {
    startTimer();

    int randomArray[16];
    int rand = randomArray[0] + randomArray[1] + randomArray[2] + randomArray[3] + randomArray[4] + randomArray[5] + randomArray[6] + randomArray[7] +
               randomArray[8] + randomArray[9] + randomArray[10] + randomArray[11] + randomArray[12] + randomArray[13] + randomArray[14] + randomArray[15];

    int offset = rand % 6;
    int baseheight = 20 - (33 - iheight) / 2 - rand % 2;
    for (int j = 38 + (39 - iheight) / 12; j > 10; j--) {
        for (int i = 0; i < 156; i++) {
            if (j >= 41 - (baseheight + sin(i))) {
                canvas[j][i + offset].symbol = '`';
                canvas[j][i + offset].bRGB = 0b1111;
                for (int k = j + 1; k < 41; k++) {
                    canvas[k][i + offset].symbol = ' '; //SIMPLER DUNES IF I USE THIS CODE
                }
            }
        }
    }

    stopTimer();
    el_bDunes = getElapsedTimeInMicroseconds();
}

void bSun(struct pixel canvas[41][156], int iheight) {
    startTimer();

    for (int j = 0; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            int iFromOrigin = i - 78;
            int jFromOrigin = j * (5 / 2) - (41 - iheight) * (5 / 2);
            int distFromOriginSquared = iFromOrigin * iFromOrigin + jFromOrigin * jFromOrigin;

            if (distFromOriginSquared < iheight * 20) {
                canvas[j][i].symbol = 'O';
                canvas[j][i].bRGB = 0b1111;
            }
        }
    }

    stopTimer();
    el_bSun = getElapsedTimeInMicroseconds();
}

void bCircleEdge(struct pixel canvas[41][156]) {
    startTimer();

#define  hashGauche = (canvas[j][i - 1].symbol == ' ' && canvas[j][i - 1].bRGB ==  0b0011)// HOW TO USE???
#define  hashDroite = (canvas[j][i + 1].symbol == ' ' && canvas[j][i + 1].bRGB ==  0b0011)// HOW TO USE???
#define  hashHaut   = (canvas[j-1][i].symbol == ' ' && canvas[j-1][i].bRGB ==  0b0011)// HOW TO USE???
#define  hashBas    = (canvas[j+1][i].symbol == ' ' && canvas[j+1][i].bRGB ==  0b0011) // HOW TO USE???
#define  hashPos    = (canvas[j][i].symbol == ' ' && canvas[j][i].bRGB ==  0b0011)// HOW TO USE???

    for (int j = 0; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if (i < 78) {
                if (((canvas[j][i + 1].symbol == ' ' && canvas[j][i + 1].bRGB == 0b0011) && (canvas[j][i].symbol != ' ' && canvas[j][i].bRGB != 0b0011)) ||
                    ((canvas[j + 1][i].symbol == ' ' && canvas[j + 1][i].bRGB == 0b0011) && canvas[j - 1][i].symbol != '('
                     && (canvas[j - 1][i].symbol != ' ' && canvas[j - 1][i].bRGB != 0b0011) // BIG BUG, CHECKING FOR SYMBOL ALWAYS BREAKS IF STATEMENT
                     && (canvas[j][i - 1].symbol != ' ' && canvas[j][i - 1].bRGB != 0b0011)
                     && (canvas[j][i].symbol != ' ' && canvas[j][i].bRGB != 0b0011))) {
                    canvas[j][i].symbol = '(';
                    canvas[j][i].bRGB = 0b1000;
                }
            } else if (((canvas[j][i].symbol != ' ' && canvas[j][i].bRGB != 0b0011) && (canvas[j][i - 1].symbol == ' ' && canvas[j][i - 1].bRGB == 0b0011)) ||
                       ((canvas[j + 1][i].symbol == ' ' && canvas[j + 1][i].bRGB == 0b0011) && canvas[j - 1][i].symbol != ')'
                        && (canvas[j - 1][i].symbol != ' ' && canvas[j - 1][i].bRGB != 0b0011)
                        && (canvas[j][i + 1].symbol != ' ' && canvas[j][i + 1].bRGB != 0b0011)
                        && (canvas[j][i].symbol != ' ' && canvas[j][i].bRGB != 0b0011))) {
                canvas[j][i].symbol = ')';
                canvas[j][i].bRGB = 0b1000;
            }
        }
    }

    stopTimer();
    el_bCircleEdge = getElapsedTimeInMicroseconds();
}

void bFill(struct pixel canvas[41][156], int iheight) {
    //MULTIPLY X BY RATIO TO GET ACCURATE LENGTH 5:2
    //ORIGIN IS (78,20.5)

    startTimer();

    for (int j = 0; j < 38; j++) {
        for (int i = 0; i < 156; i++) {
            int iFromOrigin = i - 78;
            int jFromOrigin = j * (5 / 2) - (41 - iheight) * (5 / 2);
            int distFromOriginSquared = iFromOrigin * iFromOrigin + jFromOrigin * jFromOrigin;

            if (distFromOriginSquared > iheight * 100) {
                if ((i + (j % 2) * 2) % 4 == 0)
                    canvas[j][i].symbol = '+';
                else
                    canvas[j][i].symbol = ' ';

                canvas[j][i].bRGB = 0b1000;
            }
        }
    }
    for (int j = 38; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if ((i + (j % 3) * 2) % 4 == 0)
                canvas[j][i].symbol = '_';
            else
                canvas[j][i].symbol = ' ';
            canvas[j][i].bRGB = 0b1000;
        }
    }


    stopTimer();
    el_bFill = getElapsedTimeInMicroseconds();
}

void initializeCanvas(struct pixel canvas[41][156]) {
    startTimer();

    for (int i = 0; i < 156; i++) {
        for (int j = 0; j < 41; j++) {
            canvas[j][i].symbol = ' ';
            canvas[j][i].bRGB = 0b0011;
        }
    }

    stopTimer();
    el_initializeCanvas = getElapsedTimeInMicroseconds();
}

void drawOutput(int iheight) {
    struct pixel canvas[41][156] = {}; //156x41 = pixel position
    unsigned int frameCounter = 0; //Aiming for 16 total frames at 4 fps, for a total of 4 seconds of run time

    initializeCanvas(canvas);
    startTimerTotal();
    while (1) {
        if (kbhit())
            if (getch() == ' ')
                break;
        fflush(stdin);

        bFill(canvas, iheight);
        bCircleEdge(canvas);
        bSun(canvas, iheight);
        bDunes(canvas, iheight);
        fBloomTriangle(canvas, iheight);
        fOutlineTriangle(canvas, iheight);
        fBackFaceTriangle(canvas, iheight);
        fSideFaceTriangle(canvas, iheight, frameCounter);
        fFrontFaceTriangle(canvas, iheight, frameCounter);
        float targetFrameRate = 3.0f;
        adjustFrameRate(targetFrameRate);
        showImage(canvas);
        frameCounter++;
    }
}

int main(void) {
    setvbuf(stdout, buffer,_IOLBF, sizeof(buffer));
    // Default buffer for stdout is too small to fully contain stringBuffer later, so we increase it

    float height = 0;
    int iheight = 0;


    while (1) {
        fwrite("Veuillez saisir la hauteur de la pyramide : \n", 45, 1,stdout);
        fflush(stdout);
        scanf("%f", &height);

        iheight = (int) height;
        if (height > 5 && height < 40 && height == iheight) {
            break;
        }

        fwrite("\n\t<Veuillez saisir un nombre valide et reessayer.>\n\n", 52, 1,stdout);
        fflush(stdout);
        fflush(stdin);
    }

    fflush(stdin);
    drawOutput(iheight);
    system("cls");

    el_total = (el_bFill + el_bCircleEdge + el_fBloomTriangle + el_showImage) / 1000.0f;
    printf("\x1b[97mDEBUG: ELAPSED TIME IN EACH FUNCTION :\n\t\t\t"
           "initializeCanvas = %.3lf ms\n\t\t\tbfill = %.3lf ms\n\t\t\tbCircleEdge = %.3lf ms\n\t\t\tbSun = %.3lf\n\t\t\tbDunes = %.3lf\n\t\t\t"
           "fBloomTriangle = %.3lf ms\n\t\t\tfOutlineTriangle = %.3lf ms\n\t\t\tfBackFaceTriangle = %.3lf ms\n\t\t\tfSideFaceTriangle = %.3lf ms\n\t\t\t"
           "fFrontFaceTriangle = %.3lf ms\n\t\t\tshowImage = %.3lf ms\n\t\t\ttotal = %.3lf ms\n",
           el_initializeCanvas / 1000, el_bFill / 1000, el_bCircleEdge / 1000, el_bSun / 1000, el_bDunes / 1000, el_fBloomTriangle / 1000,
           el_fOutlineTriangle / 1000, el_fBackFaceTriangle / 1000, el_fSideFaceTriangle / 1000, el_fFrontFaceTriangle / 1000, el_showImage / 1000, el_total);
    fflush(stdout);
    system("pause");
    return 0;
}
