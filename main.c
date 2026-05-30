#include <stdio.h>
#include <conio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <math.h>


#pragma region Terminal Color Defines
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
#pragma region Precompiler Platform Specific Implementation

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

#endif

#pragma endregion


char buffer[40000] = {0};
int randomOffset = 0, randomDuneHeight = 0;
uint64_t startGlobalTimer = 0, endGlobalTimer = 0, timeInInitializeCanvas = 0, timeInBackgroundFill = 0, timeInBackgroundCircleEdge = 0, timeInBackgroundSun = 0
        ,
        timeInBackgroundDunes = 0,
        timeDrawingPyramid = 0, timeInfBloomTriangle = 0, timeInfOutlineTriangle = 0, timeInfBackFaceTriangle = 0, timeInfSideFaceTriangle = 0,
        timeInfFrontFaceTriangle = 0, timeInShowImage = 0, timeElapsedLastFrame = 0;

struct pixel {
    char symbol;
    unsigned char bRGB;
};

typedef struct {
    float x; // right
    float y; // up
    float z; // positive z = further from camera
} Point;

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

void adjustFrameRate(float targetFrameRate) {
    endGlobalTimer = getTimeInMicroseconds();
    timeElapsedLastFrame = endGlobalTimer - startGlobalTimer;

    useconds_t sleepTime = 1000000.0f / targetFrameRate - timeElapsedLastFrame;
    if (sleepTime <= 1000000.0f / targetFrameRate)
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

/**
 *  Calculates the signed (positive/negative) area of a triangle.
 * @param v1 Point representing the apex of the pyramid
 * @param v2 Point left of the apex of the pyramid, when facing the v1-v2-v3 triangle
 * @param v3 Point right of the apex of the pyramid, when facing the v1-v2-v3 triangle
 * @return The signed area of the v1-v2-v3 triangle. The sign indicates if the point is to the right of the v1-v2 line segment.
 */
float edgeFunction(Point v1, Point v2, Point v3) {
    return (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);
}

/**
 * Draws specified triangle on screen, triangle is only rendered if facing the camera.
 * @param canvas Screen canvas
 * @param A Apex of the triangle
 * @param B Point left of apex
 * @param C Point right of apex
 */
void drawTriangle(struct pixel canvas[41][156], Point A, Point B, Point C, unsigned char bRGB) {
    const int minX = min(A.x, min(B.x,C.x));
    const int maxX = max(A.x, max(B.x,C.x));
    const int minY = min(A.y, min(B.y,C.y));
    const int maxY = max(A.y, max(B.y,C.y));

    for (int j = minY; j < maxY; j++)
        for (int i = minX; i < maxX; i++) {
            Point tmp = {i, j, 0};
            float ABT = edgeFunction(A, B, tmp);
            float BCT = edgeFunction(B, C, tmp);
            float CAT = edgeFunction(C, A, tmp);

            if (ABT <= 0 && CAT <= 0 && BCT <= 0) {
                canvas[j][i].symbol = '-';
                canvas[j][i].bRGB = bRGB;
            }
        }
    canvas[(int) A.y][(int) A.x].symbol = '^';
}

/**
 * Draws the pyramid by calculating the current rotation, and then appropriately rendering the 4 side triangles of the pyramid.
 * @param canvas Screen canvas
 * @param pyramid The point array of the points that represent each vertex of the pyramid.
 */
void drawPyramid(struct pixel canvas[41][156], Point pyramid[5]) {
    uint64_t start = getTimeInMicroseconds();

    drawTriangle(canvas, pyramid[0], pyramid[1], pyramid[2], 0b1010); // Front Face
    drawTriangle(canvas, pyramid[0], pyramid[2], pyramid[3], 0b1011); // Right Face
    drawTriangle(canvas, pyramid[0], pyramid[3], pyramid[4], 0b1110); // Back Face
    drawTriangle(canvas, pyramid[0], pyramid[4], pyramid[1], 0b1101); // Left Face

    /* Uncomment to draw Points
     *if (pyramid[0].z > 0) {
        canvas[(int) pyramid[0].y - 1][(int) pyramid[0].x].symbol = 'A';
        canvas[(int) pyramid[0].y - 1][(int) pyramid[0].x].bRGB = 0b0101;
    } else if (pyramid[0].z == 0) {
        canvas[(int) pyramid[0].y][(int) pyramid[0].x].symbol = 'A';
        canvas[(int) pyramid[0].y][(int) pyramid[0].x].bRGB = 0b0110;
    } else {
        canvas[(int) pyramid[0].y][(int) pyramid[0].x].symbol = 'A';
        canvas[(int) pyramid[0].y][(int) pyramid[0].x].bRGB = 0b0011;
    }

    if (pyramid[1].z > 0) {
        canvas[(int) pyramid[1].y - 1][(int) pyramid[1].x].symbol = 'B';
        canvas[(int) pyramid[1].y - 1][(int) pyramid[1].x].bRGB = 0b0101;
    } else if (pyramid[1].z == 0) {
        canvas[(int) pyramid[1].y][(int) pyramid[1].x].symbol = 'B';
        canvas[(int) pyramid[1].y][(int) pyramid[1].x].bRGB = 0b0110;
    } else {
        canvas[(int) pyramid[1].y][(int) pyramid[1].x].symbol = 'B';
        canvas[(int) pyramid[1].y][(int) pyramid[1].x].bRGB = 0b0011;
    }

    if (pyramid[2].z > 0) {
        canvas[(int) pyramid[2].y - 1][(int) pyramid[2].x].symbol = 'C';
        canvas[(int) pyramid[2].y - 1][(int) pyramid[2].x].bRGB = 0b0101;
    } else if (pyramid[2].z == 0) {
        canvas[(int) pyramid[2].y][(int) pyramid[2].x].symbol = 'C';
        canvas[(int) pyramid[2].y][(int) pyramid[2].x].bRGB = 0b0110;
    } else {
        canvas[(int) pyramid[2].y][(int) pyramid[2].x].symbol = 'C';
        canvas[(int) pyramid[2].y][(int) pyramid[2].x].bRGB = 0b0011;
    }

    if (pyramid[3].z > 0) {
        canvas[(int) pyramid[3].y - 1][(int) pyramid[3].x].symbol = 'D';
        canvas[(int) pyramid[3].y - 1][(int) pyramid[3].x].bRGB = 0b0101;
    } else if (pyramid[3].z == 0) {
        canvas[(int) pyramid[3].y][(int) pyramid[3].x].symbol = 'D';
        canvas[(int) pyramid[3].y][(int) pyramid[3].x].bRGB = 0b0110;
    } else {
        canvas[(int) pyramid[3].y][(int) pyramid[3].x].symbol = 'D';
        canvas[(int) pyramid[3].y][(int) pyramid[3].x].bRGB = 0b0011;
    }

    if (pyramid[4].z > 0) {
        canvas[(int) pyramid[4].y - 1][(int) pyramid[4].x].symbol = 'E';
        canvas[(int) pyramid[4].y - 1][(int) pyramid[4].x].bRGB = 0b0101;
    } else if (pyramid[4].z == 0) {
        canvas[(int) pyramid[4].y][(int) pyramid[4].x].symbol = 'E';
        canvas[(int) pyramid[4].y][(int) pyramid[4].x].bRGB = 0b0110;
    } else {
        canvas[(int) pyramid[4].y][(int) pyramid[4].x].symbol = 'E';
        canvas[(int) pyramid[4].y][(int) pyramid[4].x].bRGB = 0b0011;
    }*/

    uint64_t end = getTimeInMicroseconds();
    timeDrawingPyramid = end - start;
}

int wave(int x) {
    // the width of the canvas is 156, and the size of the wave table is 52, to ensure proper tiling, must modulo x by width of this array.
    x %= 52;

    // size of array is 52 because 156/3 = 52.
    int waveTable[] = {
        0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, -1, -1, -1, -2, -2, -2, -2, -3, -3, -3, -3, -3, -3, -2, -2, -2, -2, -1,
        -1, -1, -1, -1, -1, -1, 0, 0
    };
    return waveTable[x];
}

void backgroundDunes(struct pixel canvas[41][156]) {
    uint64_t start = getTimeInMicroseconds();

    //It's impossible to render a dune @ j > 10 pixels with our current math.
    for (int j = 37; j > 10; j--) {
        for (int i = 0; i < 156; i++) {
            if (j >= 41 - (randomDuneHeight + wave(i + randomOffset)) && j < 38) {
                canvas[j][i].symbol = '`';
                canvas[j][i].bRGB = 0b1111;
                for (int k = j + 1; k < 38; k++) {
                    canvas[k][i].symbol = ' ';
                }
            }
        }
    }

    uint64_t end = getTimeInMicroseconds();
    timeInBackgroundDunes = end - start;
}

void backgroundSun(struct pixel canvas[41][156], int iheight) {
    uint64_t start = getTimeInMicroseconds();

    for (int j = 0; j < 38; j++) {
        //TODO change magic number 38 to preserve ground floor to variable.
        for (int i = 0; i < 156; i++) {
            float iFromOrigin = i - 78;
            float jFromOrigin = (float) j * 2.2f - (41.0f - (float) iheight) * 2.2f;
            float distFromOriginSquared = iFromOrigin * iFromOrigin + jFromOrigin * jFromOrigin;

            if (distFromOriginSquared < (float) iheight * 20) {
                canvas[j][i].symbol = 'O';
                canvas[j][i].bRGB = 0b1111;
            }
        }
    }

    uint64_t end = getTimeInMicroseconds();
    timeInBackgroundSun = end - start;
}

void backgroundCircleEdge(struct pixel canvas[41][156]) {
    uint64_t start = getTimeInMicroseconds();

#pragma region Pixel Isolation Check
    // Out of bonds check followed by is void square check
#define isTopRightVoid ((j+1 < 41 && i+2 < 156) && canvas[j+1][i+2].symbol == ' ')
#define isBotRightVoid ((j-1 >= 0 && i+2 < 156) && canvas[j-1][i+2].symbol == ' ')
#define isTopLeftVoid  ((j+1 < 41 && i-2 >= 0) && canvas[j+1][i-2].symbol == ' ')
#define isBotLeftVoid  ((j-1 >=0 && i-2 >= 0) && canvas[j-1][i-2].symbol == ' ')
    //
#define pixelIsIsolated (isTopRightVoid || isBotRightVoid || isTopLeftVoid || isBotLeftVoid)
#pragma endregion

    for (int j = 0; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if (canvas[j][i].symbol == '+' && pixelIsIsolated) {
                if (i < 78) {
                    if ((i + (j % 2) * 2) % 4 == 0)
                        canvas[j][i].symbol = '(';
                } else {
                    if ((i + (j % 2) * 2) % 4 == 0)
                        canvas[j][i].symbol = ')';
                }
            }
            canvas[j][i].bRGB = 0b1000;
        }
    }

    uint64_t end = getTimeInMicroseconds();
    timeInBackgroundCircleEdge = end - start;
}

void backgroundFill(struct pixel canvas[41][156], const int iheight) {
    //MULTIPLY X BY RATIO TO GET ACCURATE LENGTH 2.2f
    //ORIGIN IS (78,20.5)

    uint64_t start = getTimeInMicroseconds();

    for (int j = 0; j < 38; j++) {
        for (int i = 0; i < 156; i++) {
            int iFromOrigin = i - 78;
            int jFromOrigin = (float) j * 2.2f - (41.0f - (float) iheight) * 2.2f;
            int distFromOriginSquared = iFromOrigin * iFromOrigin + jFromOrigin * jFromOrigin;

            if (distFromOriginSquared > iheight * 100) {
                if ((i + (j % 2) * 2) % 4 == 0)
                    canvas[j][i].symbol = '+';
                else
                    canvas[j][i].symbol = ' ';
                canvas[j][i].bRGB = 0b1000;
            } else {
                canvas[j][i].symbol = ' ';
                canvas[j][i].bRGB = 0b1000;
            }
        }
    }
    for (int j = 38; j < 41; j++) {
        for (int i = 0; i < 156; i++) {
            if ((i + (j % 2) * 2) % 4 == 0)
                canvas[j][i].symbol = '_';
            else
                canvas[j][i].symbol = ' ';
            canvas[j][i].bRGB = 0b1000;
        }
    }

    uint64_t end = getTimeInMicroseconds();
    timeInBackgroundFill = end - start;
}

/**
 * Rotates points of an array along the y-axis.
 * @param point A pointer to the array of points.
 * @param arrSize Number of elements of array.
 * @param radianRotation The rotation to apply to the points.
 */
void yAxisPointRotation(Point *point, int arrSize, double radianRotation) {
    Point tmp = {0};
    float tmpX = 0;
    float cosTheta = cos(radianRotation), sinTheta = sin(radianRotation);

    //foreach(point : pyramid)
    Point *end = point + arrSize;
    for (; point < end; point++) {
        // Translate the prism back to origin
        tmpX = point->x - 78;

        // Rotate the prism
        tmp.x = tmpX * cosTheta + point->z * sinTheta;
        tmp.y = point->y;
        tmp.z = tmpX * -sinTheta + point->z * cosTheta;

        // Translate the prism back to its offset
        tmp.x += 78;

        point->x = tmp.x;
        point->y = tmp.y;
        point->z = tmp.z;
    }
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

void drawOutputLoop(int iheight) {
    struct pixel canvas[41][156] = {}; //156x41 = pixel position
    unsigned int frameCounter = 0;
    unsigned int totalAnimationFrames = 24;
    double yRotation = (2.0f * M_PI) / totalAnimationFrames;

    // 0-indexed so every value is -1
    Point pyramid[5] = {
        {78, 40 - iheight, 0}, // Apex of pyramid
        {78 - iheight * 1.75f, 40, -iheight * 1.75f},
        {78 + iheight * 1.75f, 40, -iheight * 1.75f},
        {78 + iheight * 1.75f, 40, +iheight * 1.75f},
        {78 - iheight * 1.75f, 40, +iheight * 1.75f}
    };

    initializeCanvas(canvas);
    srand(time(NULL));
    randomOffset = rand() % 52; // Size of wave array is 52, so we can offset by up to 51
    randomDuneHeight = 20 - (33 - iheight) / 2 + (rand() % 4);

    for (;; frameCounter++) {
        startGlobalTimer = getTimeInMicroseconds();
        if (kbhit())
            if (getch() == ' ') // Space bar
                break;
        fflush(stdin);

        backgroundFill(canvas, iheight);
        backgroundCircleEdge(canvas);
        backgroundSun(canvas, iheight);
        backgroundDunes(canvas);
        drawPyramid(canvas, pyramid);

        showImage(canvas);

        float targetFrameRate = 4.0f;
        adjustFrameRate(targetFrameRate);

        yAxisPointRotation(pyramid, 5, yRotation);
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
        if (height > 0 && height < 31 && height == iheight) {
            break;
        }

        fwrite("\n\t<Veuillez saisir un nombre entre 1 et 30 et reessayer.>\n\n", 63, 1,stdout);
        fflush(stdout);
        fflush(stdin);
    }

    fflush(stdin);
    drawOutputLoop(iheight);
    system("cls");

    printf("\x1b[97mDEBUG: ELAPSED TIME IN EACH FUNCTION :\n\t\t\t"
           "initializeCanvas = %"PRIu64" \xE6s\n\t\t\tbfill = %"PRIu64" \xE6s\n\t\t\tbackgroundCircleEdge = %"PRIu64""
           " \xE6s\n\t\t\tbackgroundSun = %"PRIu64"\n\t\t\tbackgroundDunes = %"PRIu64"\n\t\t\t" "timeDrawingPyramid = %"PRIu64" "
           "\xE6s\n\t\t\tshowImage = %"PRIu64" \xE6s\n\t\t\ttotal = %"PRIu64" \xE6s\n",
           timeInInitializeCanvas, timeInBackgroundFill, timeInBackgroundCircleEdge, timeInBackgroundSun, timeInBackgroundDunes, timeDrawingPyramid,
           timeInShowImage, timeElapsedLastFrame);
    fflush(stdout);
    system("pause");
    return 0;
}
