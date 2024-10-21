#include <stdio.h>
#include <unistd.h>
#include <conio.h>
#include <math.h>
#include <windows.h>

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
double el_initializeCanvas, el_bFill, el_bCircleEdge, el_bSun, el_fBloomTriangle, el_fOutlineTriangle, el_fForwardFaceTriangle, el_showImage, el_colorizePixel,
        el_total,
        el_timeBetweenFrames = 0;
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
    startTimer();

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

    stopTimer();
    el_colorizePixel = getElapsedTimeInMicroseconds();
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
    write(1, "\x1b[2J\x1b[H", 7); // 1 being stdout
    write(1, stringBuffer, 38376); // 1 being stdout

    stopTimer();
    el_showImage = getElapsedTimeInMicroseconds();
}

typedef struct {
    int x;
    int y;
} Point;

float triangleArea(Point p1, Point p2, Point p3) {
    return 0.5 * abs(p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y));
}

void fForwardFaceTriangle(struct pixel canvas[41][156], int iheight) {
    startTimer();

    // Vertices
    Point A = {78, 41 - iheight};
    Point B = {78 - (iheight * 2), 41};
    Point C = {78 + (iheight * 2), 41};

    // Calculate total area of triangle specified by vertices
    float areaOrig = triangleArea(A, B, C);
    for (int j = 0; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if (j > (A.y - 1 - (10 / iheight)) && i > B.x && i < C.x) {
                Point P = {i, j}; // Current pixel's coordinate

                float area1 = triangleArea(P, B, C);
                float area2 = triangleArea(A, P, C);
                float area3 = triangleArea(A, B, P);

                // Is point inside triangle?
                if (area1 + area2 + area3 <= areaOrig) {
                    if (j > (41 - iheight + iheight / 10)) {
                        canvas[j][i].symbol = '='; // = - _ WHICH?
                        canvas[j][i].bRGB = 0b0111;
                    } else {
                        canvas[j][i].symbol = 'U';
                        canvas[j][i].bRGB = 0b1110;
                    }
                }
            }
        }
    }


    stopTimer();
    el_fForwardFaceTriangle = getElapsedTimeInMicroseconds();
}

void fOutlineTriangle(struct pixel canvas[41][156], int iheight) {
    startTimer();

    // Vertices
    Point A = {78, 40 - iheight};
    Point B = {78 - (iheight * 2) - 1, 40};
    Point C = {78 + (iheight * 2) + 1, 40};

    // Calculate total area of triangle specified by vertices
    float areaOrig = triangleArea(A, B, C);
    for (int j = 0; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if (j > (A.y - 1) && i > B.x && i < C.x) {
                Point P = {i, j}; // Current pixel's coordinate

                float area1 = triangleArea(P, B, C);
                float area2 = triangleArea(A, P, C);
                float area3 = triangleArea(A, B, P);

                // Is point inside triangle?
                if (area1 + area2 + area3 <= areaOrig) {
                    canvas[j][i].symbol = '*';
                    canvas[j][i].bRGB = 0b1110;
                }
            }
        }
    }


    stopTimer();
    el_fOutlineTriangle = getElapsedTimeInMicroseconds();
}

void fBloomTriangle(struct pixel canvas[41][156], int iheight) {
    startTimer();

    // Vertices
    Point A = {77, 39 - iheight};
    Point B = {77 - (iheight * 2) - 3, 41};
    Point C = {77 + (iheight * 2) + 3, 41};

    // Calculate total area of triangle specified by vertices
    float areaOrig = triangleArea(A, B, C);
    for (int j = 0; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if (j > (A.y - 1) && i > B.x && i < C.x) {
                Point P = {i, j}; // Current pixel's coordinate

                float area1, area2, area3 = 0;
                if (i < 78) {
                    area1 = triangleArea(P, B, C);
                    area2 = triangleArea(A, P, C);
                    area3 = triangleArea(A, B, P);
                } else {
                    A.x = 78;
                    B.x = 78 - (iheight * 2) - 3;
                    C.x = 78 + (iheight * 2) + 3;

                    area1 = triangleArea(P, B, C);
                    area2 = triangleArea(A, P, C);
                    area3 = triangleArea(A, B, P);
                }

                // Is point inside triangle?
                if (area1 + area2 + area3 <= areaOrig) {
                    if (j == 39 - iheight && i > 77) {
                        canvas[j][i].symbol = '|';
                        canvas[j][i].bRGB = 0b1110;
                    } else if (j < (43 - iheight) && j != (39 - iheight))
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
            }
        }
    }


    stopTimer();
    el_fBloomTriangle = getElapsedTimeInMicroseconds();
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

    for (int j = 0; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if (i < 78) {
                if ((canvas[j][i + 1].symbol == '#' && canvas[j][i].symbol != '#') ||
                    (canvas[j + 1][i].symbol == '#' && canvas[j - 1][i].symbol != '(' && canvas[j - 1][i].symbol != '#' && canvas[j][i - 1].symbol != '#' &&
                     canvas[j][i].symbol != '#')) {
                    canvas[j][i].symbol = '(';
                    canvas[j][i].bRGB = 0b1110;
                }
            } else if ((canvas[j][i].symbol != '#' && canvas[j][i - 1].symbol == '#') ||
                       (canvas[j + 1][i].symbol == '#' && canvas[j - 1][i].symbol != ')' && canvas[j - 1][i].symbol != '#' && canvas[j][i + 1].symbol != '#' &&
                        canvas[j][i].symbol != '#')) {
                canvas[j][i].symbol = ')';
                canvas[j][i].bRGB = 0b1110;
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

    for (int j = 0; j < 41; j++) {
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

    stopTimer();
    el_bFill = getElapsedTimeInMicroseconds();
}

void initializeCanvas(struct pixel canvas[41][156]) {
    startTimer();

    for (int i = 0; i < 156; i++) {
        for (int j = 0; j < 41; j++) {
            canvas[j][i].symbol = '#';
            canvas[j][i].bRGB = 0b1100;
        }
    }

    stopTimer();
    el_initializeCanvas = getElapsedTimeInMicroseconds();
}

void drawOutput(int iheight) {
    struct pixel canvas[41][156] = {}; //156x41 = pixel position
    unsigned char frameCounter = 0; //Aiming for 16 total frames at 4 fps, for a total of 4 seconds of run time
    float targetFrameRate = 3.0f;

    initializeCanvas(canvas);
    startTimerTotal();
    while (1) {
        if (kbhit())
            if (getch() == ' ')
                break;
        fflush(stdin);

        switch (frameCounter) /*NOLINT(*-multiway-paths-covered)*/ {
            case 0:
                bFill(canvas, iheight);
                bCircleEdge(canvas);
                bSun(canvas, iheight);
                fBloomTriangle(canvas, iheight);
                fOutlineTriangle(canvas, iheight);
                fForwardFaceTriangle(canvas, iheight);
                adjustFrameRate(targetFrameRate);
                showImage(canvas);
                break;
            default: printf("FRAME COUNTER ERROR\nDEBUG: frameCounter = %i\n", frameCounter);
                fflush(stdout);
                break;
        }
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
           "initializeCanvas = %.3lf ms\n\t\t\tbfill = %.3lf ms\n\t\t\tbCircleEdge = %.3lf ms\n\t\t\tbSun = %.3lf\n\t\t\tfBloomTriangle = %.3lf ms\n\t\t\t"
           "fOutlineTriangle = %.3lf ms\n\t\t\tfForwardFaceTriangle = %.3lf ms\n\t\t\tshowImage = %.3lf ms\n\t\t\tcolorizePixel = %.3lf ms\n\t\t\ttotal = %.3lf ms\n",
           el_initializeCanvas / 1000, el_bFill / 1000, el_bCircleEdge / 1000, el_bSun / 1000, el_fBloomTriangle / 1000, el_fOutlineTriangle / 1000,
           el_fForwardFaceTriangle / 1000, el_showImage / 1000, el_colorizePixel / 1000, el_total);
    fflush(stdout);
    system("pause");
    return 0;
}
