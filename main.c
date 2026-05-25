#include <stdio.h>
#include <conio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdint.h>


#pragma region TerminalColorDefines
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

#pragma endregion
// #ifdef _WIN32
// #undef _WIN32
// #undef __WIN32__
// #endif
#pragma region PrecompilerPlatformSpecificImplementation

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>

// This usleep function is rewritten here so I can debug the code for this program,
// the weird usleep implementation in windows.h prevents the debugger from running.
int usleep(const unsigned int usecond) {
    LARGE_INTEGER start, counter, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    do {
        QueryPerformanceCounter(&counter);
    } while ((uint64_t) (counter.QuadPart - start.QuadPart) < (uint64_t) usecond * (uint64_t) freq.QuadPart / 1000000ULL);

    return 0;
}

#else // Linux/Mac?
#include <time.h>

#endif

#pragma endregion


char buffer[40000] = {0};
uint64_t startGlobalTimer, endGlobalTimer, timeInInitializeCanvas, timeInbFill, timeInbCircleEdge, timeInbSun, timeInbDunes, timeInfBloomTriangle,
        timeInfOutlineTriangle, timeInfBackFaceTriangle, timeInfSideFaceTriangle, timeInfFrontFaceTriangle, timeInShowImage, timeElapsedLastFrame = 0;

uint64_t getTimeInMicroseconds() {
    uint64_t timeInMicroseconds = 0;
#if defined(_WIN32) || defined(__WIN32__)
    LARGE_INTEGER counter, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    timeInMicroseconds = (uint64_t) (counter.QuadPart * (1000000) / freq.QuadPart);
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    timeInMicroseconds = (uint64_t) t.tv_sec * 1000000 + (uint64_t) t.tv_nsec / 1000;
#endif
    return timeInMicroseconds;
}

struct pixel {
    char symbol;
    unsigned char bRGB;
};

void adjustFrameRate(float targetFrameRate) {
    endGlobalTimer = getTimeInMicroseconds();
    timeElapsedLastFrame = endGlobalTimer - startGlobalTimer;

    useconds_t sleepTime = 1000000.0f / targetFrameRate - timeElapsedLastFrame;
    if (!(sleepTime > 1000000.0f / targetFrameRate))
        usleep(sleepTime);
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
    uint64_t start = getTimeInMicroseconds();

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

    uint64_t end = getTimeInMicroseconds();
    timeInShowImage = end - start;
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

void outlineIfBlock(struct pixel canvas[41][156], int j, int i) {
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
    float slopeLeft = (v2.y * 1.0f - v1.y) / (v2.x - v1.x + 0.001f); //the 0.0001f bias is to get rid of 0 asymptote
    float slopeRight = (v3.y * 1.0f - v2.y) / (v3.x - v2.x + 0.001f); //the 0.0001f bias is to get rid of 0 asymptote

    float offsetLeft = v1.y - slopeLeft * v1.x;
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
                case 3: outlineIfBlock(canvas, j, i);
                    break;
                case 4: bloomIfBlock(canvas, iheight, j, i);
                    break;
                default: printf("DEBUG: BAD IF BLOCK CASE. ifBlock = %i", ifBlock);
            }
        }
    }
}

void fFrontFaceTriangle(struct pixel canvas[41][156], int iheight, unsigned int frameCounter) {
    uint64_t start = getTimeInMicroseconds();

    // Vertices
    Point A = {78, 41 - iheight};
    Point B = {0, 40};
    Point C = {0, 40};
    if (frameCounter >= 18 && frameCounter < 24) {
        B.x = 78 + (iheight * 2) - ((frameCounter % 6) * (4 * iheight + 1) / 6);
        C.x = 78 + (iheight * 2);
    }
    if (frameCounter < 6) {
        B.x = 78 - (iheight * 2);
        C.x = 78 + (iheight * 2) - ((frameCounter % 6) * (4 * iheight + 1) / 6);
    }

    fillTriangle(B, A, C, canvas, iheight, 0);

    uint64_t end = getTimeInMicroseconds();
    timeInfFrontFaceTriangle = end - start;
}

void fSideFaceTriangle(struct pixel canvas[41][156], int iheight, unsigned int frameCounter) {
    uint64_t start = getTimeInMicroseconds();

    // Vertices
    Point A = {78, 41 - iheight};
    Point B = {78 - (iheight * 2), 40};
    Point C = {78 + (iheight * 2), 40};

    fillTriangle(B, A, C, canvas, iheight, 1);

    uint64_t end = getTimeInMicroseconds();
    timeInfSideFaceTriangle = end - start;
}

void fBackFaceTriangle(struct pixel canvas[41][156], int iheight) {
    uint64_t start = getTimeInMicroseconds();

    // Vertices
    Point A = {78, 41 - iheight};
    Point B = {78 - (iheight * 2), 40};
    Point C = {78 + (iheight * 2), 40};

    fillTriangle(B, A, C, canvas, iheight, 2);

    uint64_t end = getTimeInMicroseconds();
    timeInfBackFaceTriangle = end - start;
}

void fOutlineTriangle(struct pixel canvas[41][156], int iheight) {
    uint64_t start = getTimeInMicroseconds();

    // Vertices
    Point A = {78, 40 - iheight};
    Point B = {78 - (iheight * 2) - 1, 40};
    Point C = {78 + (iheight * 2) + 1, 40};

    fillTriangle(B, A, C, canvas, iheight, 3);

    uint64_t end = getTimeInMicroseconds();
    timeInfOutlineTriangle = end - start;
}

void fBloomTriangle(struct pixel canvas[41][156], int iheight) {
    uint64_t start = getTimeInMicroseconds();
    // Vertices
    Point A = {77, 39 - iheight};
    Point B = {77 - (iheight * 2) - 3, 40};
    Point C = {77 + (iheight * 2) + 3, 40};

    fillTriangle(B, A, C, canvas, iheight, 4);

    uint64_t end = getTimeInMicroseconds();
    timeInfBloomTriangle = end - start;
}

int sin(int x) {
    int sinTable[] = {
        0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 0, -1, -1, -1, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3, -3, -3, -3, -2, -2, -2,
        -2, -1, -1, -1, 0
    };
    return sinTable[(sizeof(sinTable) / 4 - x) % (sizeof(sinTable) / 4)];
}

void bDunes(struct pixel canvas[41][156], int iheight) {
    uint64_t start = getTimeInMicroseconds();

    int random; // Pulls number from uninitialized memory. I chose this because I wanted as few libraries as possible when I started C (and this project).

    // ReSharper disable CppLocalVariableMightNotBeInitialized
    int offset = random % 6;
    int baseheight = 20 - (33 - iheight) / 2 - random % 2;
    // ReSharper restore CppLocalVariableMightNotBeInitialized
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

    uint64_t end = getTimeInMicroseconds();
    timeInbDunes = end - start;
}

void bSun(struct pixel canvas[41][156], int iheight) {
    uint64_t start = getTimeInMicroseconds();

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

    uint64_t end = getTimeInMicroseconds();
    timeInbSun = end - start;
}

void bCircleEdge(struct pixel canvas[41][156]) {
    uint64_t start = getTimeInMicroseconds();

#define  hashGauche (canvas[j][i - 1].symbol == ' ' && canvas[j][i - 1].bRGB ==  0b0011)// HOW TO USE???
#define  hashDroite (canvas[j][i + 1].symbol == ' ' && canvas[j][i + 1].bRGB ==  0b0011)// HOW TO USE???
#define  hashHaut   (canvas[j-1][i].symbol == ' ' && canvas[j-1][i].bRGB ==  0b0011)// HOW TO USE???
#define  hashBas    (canvas[j+1][i].symbol == ' ' && canvas[j+1][i].bRGB ==  0b0011) // HOW TO USE???
#define  hashPos    (canvas[j][i].symbol == ' ' && canvas[j][i].bRGB ==  0b0011)// HOW TO USE???

#define CARRE(X) ((X)*(X))


    for (int j = 0; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if (i < 78) {
                if ((hashDroite && (canvas[j][i].symbol != ' ' && canvas[j][i].bRGB != 0b0011)) ||
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

    uint64_t end = getTimeInMicroseconds();
    timeInbCircleEdge = end - start;
}

void bFill(struct pixel canvas[41][156], int iheight) {
    //MULTIPLY X BY RATIO TO GET ACCURATE LENGTH 5:2
    //ORIGIN IS (78,20.5)

    uint64_t start = getTimeInMicroseconds();

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


    uint64_t end = getTimeInMicroseconds();
    timeInbFill = end - start;
}

void initializeCanvas(struct pixel canvas[41][156]) {
    uint64_t start = getTimeInMicroseconds();

    fflush(stdout);
    system("cls");
    for (int i = 0; i < 156; i++) {
        for (int j = 0; j < 41; j++) {
            canvas[j][i].symbol = ' ';
            canvas[j][i].bRGB = 0b0011;
        }
    }

    uint64_t end = getTimeInMicroseconds();
    timeInInitializeCanvas = end - start;
}

void drawOutput(int iheight) {
    struct pixel canvas[41][156] = {}; //156x41 = pixel position
    unsigned int frameCounter = 0; //Aiming for 16 total frames at 4 fps, for a total of 4 seconds of run time

    initializeCanvas(canvas);
    while (1) {
        startGlobalTimer = getTimeInMicroseconds();
        if (kbhit())
            if (getch() == ' ')
                break;
        fflush(stdin);

        bFill(canvas, iheight);
        bCircleEdge(canvas);
        bSun(canvas, iheight);
        bDunes(canvas, iheight);
        //fBloomTriangle(canvas, iheight);
        fOutlineTriangle(canvas, iheight);
        fBackFaceTriangle(canvas, iheight);
        fSideFaceTriangle(canvas, iheight, frameCounter);
        fFrontFaceTriangle(canvas, iheight, frameCounter);
        showImage(canvas);
        float targetFrameRate = 4.0f;
        adjustFrameRate(targetFrameRate);

        frameCounter++;
        if (frameCounter > 24)
            frameCounter = 0;
    }
}

int main(void) {
    setvbuf(stdout, buffer,_IOLBF, sizeof(buffer));
    // Default buffer for stdout is too small to fully contain stringBuffer later, so we increase it

    float height = 0;
    int iheight = 0;


    while (1) {
        fwrite("Veuillez saisir la hauteur de la pyramide : \n", 46, 1,stdout);
        fflush(stdout);
        scanf("%f", &height);

        iheight = (int) height;
        if (height > 5 && height < 40 && height == iheight) {
            break;
        }

        fwrite("\n\t<Veuillez saisir un nombre entre 6 et 39 et reessayer.>\n\n", 63, 1,stdout);
        fflush(stdout);
        fflush(stdin);
    }

    fflush(stdin);
    drawOutput(iheight);
    system("cls");

    printf("\x1b[97mDEBUG: ELAPSED TIME IN EACH FUNCTION :\n\t\t\t"
           "initializeCanvas = %"PRIu64" \xE6s\n\t\t\tbfill = %"PRIu64" \xE6s\n\t\t\tbCircleEdge = %"PRIu64" \xE6s\n\t\t\tbSun = %"PRIu64"\n\t\t\tbDunes = %"
           PRIu64"\n\t\t\t"
           "fBloomTriangle = %"PRIu64" \xE6s\n\t\t\tfOutlineTriangle = %"PRIu64" \xE6s\n\t\t\tfBackFaceTriangle = %"PRIu64" \xE6s\n\t\t\tfSideFaceTriangle = %"
           PRIu64" \xE6s\n\t\t\t"
           "fFrontFaceTriangle = %"PRIu64" \xE6s\n\t\t\tshowImage = %"PRIu64" \xE6s\n\t\t\ttotal = % "PRIu64" \xE6s\n",
           timeInInitializeCanvas, timeInbFill, timeInbCircleEdge, timeInbSun, timeInbDunes, timeInfBloomTriangle,
           timeInfOutlineTriangle, timeInfBackFaceTriangle, timeInfSideFaceTriangle, timeInfFrontFaceTriangle,
           timeInShowImage, timeElapsedLastFrame);
    fflush(stdout);
    system("pause");
    return 0;
}
