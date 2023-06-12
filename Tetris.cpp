// Tetris.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Tetris.h"
#include <windows.h>
#include <vector>
#include <ctime>
#include <string>
#include <thread>
#include <fstream>

using namespace std;

#define IDM_NEWGAME 1001
#define IDM_EXIT 1002
#define IDM_ABOUT_GAME 1003
#define IDM_PAUSE 1004
#define IDM_RESUME 1005
#define IDM_OPEN_RECORDS 1006

const int FieldWidth = 8;
const int FieldHeight = 16;
const int CellSize = 35;
const int WindowWidth = FieldWidth * CellSize + 250;
const int WindowHeight = FieldHeight * CellSize + 35 + 50;
bool initialized = false; 
int score = 0;
int scoreTemp = 0;

int level = 1;
int levelTemp = 0;

int numberOfDestroyedLines = 0;
int numberOfDestroyedLinesTemp = 0;

int timerStart = 1000;
bool gameRunning = true;
bool gamePaused = false;

bool collisionDetection = false;
bool gameOverMessageBoxShown = false;

string scoreString_temp = "SCORE: " + to_string(score);
string levelString_temp = "LEVEL: " + to_string(level);
string numberOfDestroyedLinesString_temp = "NUMBER OF DESTROYED LINES: " + to_string(numberOfDestroyedLines);

// Преобразуйте значение переменной score в строку
wstring scoreString = to_wstring(score);
wstring levelString = to_wstring(level);
wstring numberOfDestroyedLinesString = to_wstring(numberOfDestroyedLines);

// Преобразуйте строку в массив символов (const wchar_t*)
const wchar_t* scoreText = scoreString.c_str();
const wchar_t* levelText = levelString.c_str();
const wchar_t* numberOfDestroyedLinesText = numberOfDestroyedLinesString.c_str();

int rotatedPiece[4][4];
int colorStore[8][3]
{ 
    {3, 255, 70},
    {245, 245, 7},
    {0, 252, 252},
    {205, 2, 250},
    {17, 5, 247},
    {252, 3, 148},
    {135, 9, 224},
    {121, 255, 3}
};
int color = 0;
int colorNext = 0;

int randomNextPiece;
int randomNextColor;

HWND hwnd;
HWND hTextBox;
HWND hTextBox2;
HWND hTextBox3;

HDC hdc;
std::vector<std::vector<bool>> field(FieldHeight, std::vector<bool>(FieldWidth));
int currentPiece[4][4]; 
int pieceX, pieceY;
int nextPiece[4][4]; 
int nextPieceX, nextPieceY;

void DrawPiece();
void NewGame();
void EndGame();
void DrawNextPiece();
void FixPiece();
void DrawGrid();
void DrawField();
bool CanMove(int newX, int newY);
void RemoveLines();
void RotatePiece();
void Update();
void GeneratePiece();
void GenerateNextPiece();
void MovePiece(int newX, int newY);
void GameThread(HWND hwnd);
void LoadAndDisplayPNGIcon(HWND hWnd, LPCWSTR filePath);
void PauseGame();
void ResumeGame();
void SaveResultsToFile();
void OpenRecords();

std::wstring stringToWstring(const std::string& str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}

void OpenRecords()
{
    system("results.txt");
}


void LoadAndDisplayPNGIcon(HWND hWnd, LPCWSTR filePath)
{
    HANDLE hIcon = LoadImage(NULL, filePath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (hIcon)
    {
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }
}

void SaveResultsToFile() {
    std::ofstream outputFile("results.txt", std::ios::app); 
    if (!outputFile.is_open()) {
        return;
    }

    // Получение текущего времени
    SYSTEMTIME time;
    GetLocalTime(&time);
    std::string timestamp = std::to_string(time.wYear) + "-" + std::to_string(time.wMonth) + "-" + std::to_string(time.wDay) +
        " " + std::to_string(time.wHour) + ":" + std::to_string(time.wMinute) + ":" +
        std::to_string(time.wSecond);

    // Дописывание результатов в файл
    outputFile << "Timestamp: " << timestamp << std::endl;
    outputFile << "SCORE: " << score << std::endl;
    outputFile << "LEVEL: " << level << std::endl;
    outputFile << "NUMBER OF DESTROYED LINES: " << numberOfDestroyedLines << std::endl;

    outputFile.close();
}

void GameThread(HWND hwnd)
{
    LoadAndDisplayPNGIcon(hwnd, L"LOGTETRIS.ico");

    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, IDM_NEWGAME, L"&New Game");
    AppendMenu(hSubMenu, MF_STRING, IDM_ABOUT_GAME, L"&About Game");
    AppendMenu(hSubMenu, MF_STRING, IDM_EXIT, L"&Exit");
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, L"&Menu");
    AppendMenu(hSubMenu, MF_STRING, IDM_OPEN_RECORDS, L"&Open Records");

    SetMenu(hwnd, hMenu);

    
    hdc = GetDC(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Update(); 

        while (gamePaused) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
}

void PauseGame()
{
    gamePaused = true;
}
void ResumeGame()
{
    gamePaused = false;
}

void DrawGrid() {
    
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
    SelectObject(hdc, hPen);

    for (int i = 0; i <= FieldHeight; i++)
    {
        MoveToEx(hdc, 0, i * CellSize, NULL);
        LineTo(hdc, FieldWidth * CellSize, i * CellSize);
    }

    for (int i = 0; i <= FieldWidth; i++)
    {
        MoveToEx(hdc, i * CellSize, 0, NULL);
        LineTo(hdc, i * CellSize, FieldHeight * CellSize);
    }
}

void GeneratePiece() {
    srand(static_cast<unsigned int>(time(0)));
    int randomPiece;
    int randomColor;

    if (field.empty())
    {
        randomPiece = rand() % 7 + 1; 
        randomColor = rand() % 7 + 1; 
    }
    else
    {
        randomPiece = randomNextPiece;
        randomColor = randomNextColor;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            currentPiece[i][j] = false;
        }
    }

    switch (randomPiece) {
    case 0: // O piece
        currentPiece[0][1] = true;
        currentPiece[0][2] = true;
        currentPiece[1][1] = true;
        currentPiece[1][2] = true;
        break;
    case 1: // J piece
        currentPiece[0][2] = true;
        currentPiece[1][2] = true;
        currentPiece[2][2] = true;
        currentPiece[2][1] = true;
        break;
    case 2: // L piece
        currentPiece[0][1] = true;
        currentPiece[1][1] = true;
        currentPiece[2][1] = true;
        currentPiece[2][2] = true;
        break;
    case 3: // I piece
        currentPiece[0][1] = true;
        currentPiece[1][1] = true;
        currentPiece[2][1] = true;
        currentPiece[3][1] = true;
        break;
    case 4: // S piece
        currentPiece[0][1] = true;
        currentPiece[1][1] = true;
        currentPiece[1][2] = true;
        currentPiece[2][2] = true;
        break;
    case 5: // T piece
        currentPiece[1][1] = true;
        currentPiece[1][2] = true;
        currentPiece[0][2] = true;
        currentPiece[1][3] = true;
        break;
    case 6: // Z piece
        currentPiece[0][2] = true;
        currentPiece[1][2] = true;
        currentPiece[1][1] = true;
        currentPiece[2][1] = true;
        break;
    case 7: // C piece
        currentPiece[0][1] = true;
        currentPiece[1][1] = true;
        currentPiece[1][2] = true;
        currentPiece[1][3] = true;
        currentPiece[0][3] = true;
        break;
    }

    switch (randomColor) {
    case 0:
        color = 0;
        break;
    case 1:
        color = 1;
        break;
    case 2:
        color = 2;
        break;
    case 3:
        color = 3;
        break;
    case 4:
        color = 4;
        break;
    case 5:
        color = 5;
        break;
    case 6:
        color = 6;
        break;
    case 7:
        color = 7;
        break;
    }

    pieceX = FieldWidth / 2 - 2;
    pieceY = -4;
}

void GenerateNextPiece() {
    // Generate a new random piece
    srand(static_cast<unsigned int>(time(0)));
    randomNextPiece = ((rand() % 8) + 1); 

    if (randomNextPiece > 7)
    {
        randomNextPiece -= 1;
    }

    randomNextColor = ((rand() % 8) + 1); 

    if (randomNextColor > 7)
    {
        randomNextColor -= 1;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            nextPiece[i][j] = false;
        }
    }

    switch (randomNextPiece) {
    case 0: // O piece
        nextPiece[0][1] = true;
        nextPiece[0][2] = true;
        nextPiece[1][1] = true;
        nextPiece[1][2] = true;
        break;
    case 1: // J piece
        nextPiece[0][2] = true;
        nextPiece[1][2] = true;
        nextPiece[2][2] = true;
        nextPiece[2][1] = true;
        break;
    case 2: // L piece
        nextPiece[0][1] = true;
        nextPiece[1][1] = true;
        nextPiece[2][1] = true;
        nextPiece[2][2] = true;
        break;
    case 3: // I piece
        nextPiece[0][1] = true;
        nextPiece[1][1] = true;
        nextPiece[2][1] = true;
        nextPiece[3][1] = true;
        break;
    case 4: // S piece
        nextPiece[0][1] = true;
        nextPiece[1][1] = true;
        nextPiece[1][2] = true;
        nextPiece[2][2] = true;
        break;
    case 5: // T piece
        nextPiece[1][1] = true;
        nextPiece[1][2] = true;
        nextPiece[0][2] = true;
        nextPiece[1][3] = true;
        break;
    case 6: // Z piece
        nextPiece[0][2] = true;
        nextPiece[1][2] = true;
        nextPiece[1][1] = true;
        nextPiece[2][1] = true;
        break;
    case 7: // C piece
        nextPiece[0][1] = true;
        nextPiece[1][1] = true;
        nextPiece[1][2] = true;
        nextPiece[1][3] = true;
        nextPiece[0][3] = true;
        break;
    }

    switch (randomNextColor) {
    case 0:
        colorNext = 0;
        break;
    case 1:
        colorNext = 1;
        break;
    case 2:
        colorNext = 2;
        break;
    case 3:
        colorNext = 3;
        break;
    case 4:
        colorNext = 4;
        break;
    case 5:
        colorNext = 5;
        break;
    case 6:
        colorNext = 6;
        break;
    case 7:
        colorNext = 7;
        break;
    }

    nextPieceX = 100;
    nextPieceY = 5;

    DrawNextPiece(); 
}

void DrawPiece() {
    RECT rect;
    HBRUSH brush;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece[y][x]) {
                rect.left = (pieceX + x) * CellSize;
                rect.top = (pieceY + y) * CellSize;
                rect.right = rect.left + CellSize;
                rect.bottom = rect.top + CellSize;

                brush = CreateSolidBrush(RGB(colorStore[color][0], colorStore[color][1], colorStore[color][2]));
                FillRect(hdc, &rect, brush);
                DeleteObject(brush);
            }
        }
    }
}

void DrawNextPiece() { 
    RECT rect;
    HBRUSH brush;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (nextPiece[i][j]) {
                rect.left = 220 + nextPieceX + j * CellSize;
                rect.top = 200 + nextPieceY + i * CellSize;

                rect.right = rect.left + CellSize;
                rect.bottom = rect.top + CellSize;
            
                brush = CreateSolidBrush(RGB(colorStore[colorNext][0], colorStore[colorNext][1], colorStore[colorNext][2]));
                FillRect(hdc, &rect, brush);
                DeleteObject(brush);
            }
        }
    }
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    SelectObject(hdc, hPen);

    for (int i = 0; i <= 4; i++)
    {
        MoveToEx(hdc, 320, 170 + i * CellSize, NULL);
        LineTo(hdc, 320 + 4 * CellSize, 170 + i * CellSize);
    }

    for (int i = 0; i <= 4; i++)
    {
        MoveToEx(hdc, 320 + i * CellSize, 170, NULL);
        LineTo(hdc, 320 + i * CellSize, 170 + 4 * CellSize);
    }
}


void FixPiece() {
    collisionDetection = false;
    for (int y = 0; y < 4; y++) {
        if (collisionDetection) break;
        for (int x = 0; x < 4; x++)
        {
            if (currentPiece[y][x])
            {
                if ((pieceY + y) < 16 && (pieceY + y) > 0)
                {
                    field[pieceY + y][pieceX + x] = true;
                }
                else if ((pieceY + y) <= 0)
                {
                    SaveResultsToFile();

                    for (int y = 0; y < FieldHeight; y++) {
                        for (int x = 0; x < FieldWidth; x++) {
                            field[y][x] = false;
                        }
                    }

                    if (!gameOverMessageBoxShown) { 
                        int result = MessageBoxA(hwnd, "Ви програли. Щоб почати нову гру, натисніть кнопку \"ОК\". \n Якщо ви хочете вийти з гри, натисніть \"СКАСУВАТИ\".", "Ви програли.", MB_OKCANCEL | MB_SYSTEMMODAL);

                        gameOverMessageBoxShown = true; 
                        collisionDetection = true;

                        if (result == IDOK)
                        {
                            NewGame();
                        }
                        else if (result == IDCANCEL)    
                        {
                            EndGame();
                        }
                    }

                    break;
                }
            }
        }
    }

    collisionDetection = false;

    RemoveLines();
    GeneratePiece(); 
    GenerateNextPiece();

    pieceX = FieldWidth / 2 - 2;
    pieceY = -4;

}

void DrawField() {
    RECT rect = { 0 };
    HBRUSH brush = NULL;

    brush = CreateSolidBrush(RGB(242, 242, 242));
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    for (int y = 0; y < FieldHeight; y++) {
        for (int x = 0; x < FieldWidth; x++) {
            if (field[y][x]) {
                RECT rect;
                rect.left = x * CellSize;
                rect.top = y * CellSize;
                rect.right = rect.left + CellSize;
                rect.bottom = rect.top + CellSize;

                brush = CreateSolidBrush(RGB(252, 2, 2)); // цвет фигуры после остановки

                FillRect(hdc, &rect, brush);

                DeleteObject(brush);
            }
        }
    }

    DrawNextPiece();
}

bool CanMove(int newX, int newY) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece[y][x]) {
                int fieldX = newX + x;
                int fieldY = newY + y;

                if (fieldX < 0 || fieldX >= FieldWidth || fieldY >= FieldHeight || (fieldY >= 0 && field[fieldY][fieldX]))
                    return false;
            }
        }
    }

    return true;
}

void RemoveLines() {
    int counter = 0;

    for (int y = FieldHeight - 1; y >= 0; y--) {
        bool fullLine = true;
        for (int x = 0; x < FieldWidth; x++) {
            if (!field[y][x]) {
                fullLine = false;
                break;
            }
        }
        if (fullLine) {
            for (int j = y; j > 0; j--) {
                for (int x = 0; x < FieldWidth; x++) {
                    field[j][x] = field[j - 1][x];
                }
            }
            for (int x = 0; x < FieldWidth; x++) {
                field[0][x] = false;
            }

            score += 10;
            numberOfDestroyedLines++;

            if (numberOfDestroyedLines % 5 == 0)
            {
                level++;
            }

            scoreString_temp = "SCORE: " + to_string(score);
            scoreString = to_wstring(score);

            levelString_temp = "LEVEL: " + to_string(level);
            levelString = to_wstring(level);
            
            numberOfDestroyedLinesString_temp = "NUMBER OF DESTROYED LINES: " + to_string(numberOfDestroyedLines);
            numberOfDestroyedLinesString = to_wstring(level);
        }

    }
}

void RotatePiece() {
    int rotatedPiece[4][4];

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            rotatedPiece[x][3 - y] = currentPiece[y][x];
        }
    }

    if (CanMove(pieceX, pieceY)) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                currentPiece[y][x] = rotatedPiece[y][x];
            }
        }
    }
    else
    {
        return;
    }
}

void Update()
{
    if (gamePaused) {
        Sleep(100);
        return;
    }

    if (CanMove(pieceX, pieceY + 1) && collisionDetection == false)
    {
        MovePiece(pieceX, pieceY + 1);
    }
    else
    {
        if (pieceX < 0 || pieceX + 4 > FieldWidth || pieceY + 1 < 0 || pieceY + 1 + 4 > FieldHeight || collisionDetection == true) {
            FixPiece();
            RemoveLines();
            GeneratePiece();
            GenerateNextPiece();
            return; 
        }
    }
}

void NewGame() 
{
    score = 0;
    level = 1;
    numberOfDestroyedLines = 0;

    scoreString_temp = "SCORE: " + to_string(score);
    scoreString = to_wstring(score);

    levelString_temp = "LEVEL: " + to_string(level);
    levelString = to_wstring(level);

    numberOfDestroyedLinesString_temp = "NUMBER OF DESTROYED LINES: " + to_string(numberOfDestroyedLines);
    numberOfDestroyedLinesString = to_wstring(numberOfDestroyedLines);

    for (int y = 0; y < FieldHeight; y++) {
        for (int x = 0; x < FieldWidth; x++) {
            field[y][x] = false;
        }
    }

    GeneratePiece();
    GenerateNextPiece();
    pieceX = FieldWidth / 2 - 2;
    pieceY = -4;

    gameOverMessageBoxShown = false;
}

void EndGame()
{   
    gameRunning = false;
    DestroyWindow(hwnd);
}

void MovePiece(int newX, int newY)
{
    pieceX = newX;
    pieceY = newY;

    if (!gameRunning)
        return;

    // Отрисовка поля и фигуры с новыми координатами
    DrawField();
    DrawPiece();
    DrawGrid();
    DrawNextPiece();

    // Обновление содержимого окна
    InvalidateRect(hwnd, NULL, TRUE);

    if (newX < 0 || newX + 4 > FieldWidth || newY < 0 || newY + 4 > FieldHeight) {
        return; // Не перемещать фигуру, если новые координаты недопустимы
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_NEWGAME:
            ResumeGame();
            NewGame();
            break;
        case IDM_ABOUT_GAME:
            MessageBox(hwnd, L"Тетрис - це популярна комп'ютерна гра, в якій гравець повинен керувати падаючими блоками (тетроміно) і розташовувати їх у різних комбінаціях, намагаючись заповнити горизонтальні рядки. Коли рядок заповнюється повністю, він зникає, а гравець отримує очки. Головною метою гри є тривале виживання і набір якомога більшої кількості очок. \n\nст. гр. КІУКІ-21-10 Гасанова Лоліта.", L"Про Гру", MB_OK | MB_ICONINFORMATION);
            PauseGame(); 
            break;
        case IDM_EXIT:
            EndGame();
            break;
        case IDM_PAUSE:
            PauseGame();
            break;
        case IDM_OPEN_RECORDS:
            OpenRecords();
            break;
        case IDM_RESUME:
            ResumeGame();
            break;
        }
        break;
    case WM_CREATE:
        if (!initialized) {
            hwnd = hwnd;
            hdc = GetDC(hwnd);
            
            if (gameRunning)
            {
                if (!collisionDetection)
                {
                    GeneratePiece();
                    GenerateNextPiece();
                }

                Update();

                SetTimer(hwnd, 1, timerStart, NULL); 

                thread gameThread(GameThread, hwnd);
                gameThread.detach();

                hTextBox = CreateWindowW(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY, 300, 50, 200, 20, hwnd, NULL, NULL, NULL);
                hTextBox2 = CreateWindowW(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY, 300, 80, 200, 20, hwnd, NULL, NULL, NULL);
                hTextBox3 = CreateWindowW(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY, 300, 110, 200, 40, hwnd, NULL, NULL, NULL);

                SetWindowTextW(hTextBox, L"SCORE: 0");
                SetWindowTextW(hTextBox2, L"LEVEL: 0");
                SetWindowTextW(hTextBox3, L"NUMBER OF DESTROYED LINES: 0");
                DrawNextPiece();
            }
            initialized = true;
        }
        break;

    case WM_PAINT:
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        
        if (gameRunning)
        {
            DrawField();
            DrawPiece();
            DrawGrid();
            DrawNextPiece();
        }

        EndPaint(hwnd, &ps);
        break;
    case WM_TIMER:
        if (CanMove(pieceX, pieceY + 1) && collisionDetection == false)
        {
            MovePiece(pieceX, pieceY + 1);
        }
        else
        {
            FixPiece();
        }

        if (score == scoreTemp + 50)
        {
            scoreTemp = score;
            
            if (timerStart > 150)
            {
                timerStart /= 1.5;
            }

            KillTimer(hwnd, 1);
            SetTimer(hwnd, 1, timerStart, NULL); 
        }

        scoreString = stringToWstring(scoreString_temp);
        scoreText = scoreString.c_str();

        levelString = stringToWstring(levelString_temp);
        levelText = levelString.c_str();

        numberOfDestroyedLinesString = stringToWstring(numberOfDestroyedLinesString_temp);
        numberOfDestroyedLinesText = numberOfDestroyedLinesString.c_str();

        SetWindowTextW(hTextBox, scoreText);
        SetWindowTextW(hTextBox2, levelText);
        SetWindowTextW(hTextBox3, numberOfDestroyedLinesText);

        if (gameRunning)
        {
            DrawGrid();
            DrawField();
            DrawPiece();
            DrawNextPiece();
        }

        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT:
            if (CanMove(pieceX - 1, pieceY))
            {
                MovePiece(pieceX - 1, pieceY);
            }
            break;
        case VK_RIGHT:
            if (CanMove(pieceX + 1, pieceY))
            {
                MovePiece(pieceX + 1, pieceY);
            }
            break;
        case VK_DOWN:
            if (CanMove(pieceX, pieceY + 1))
            {
                MovePiece(pieceX, pieceY + 1);
            }
            break;
        case VK_UP:
            if (CanMove(pieceX - 1, pieceY) && CanMove(pieceX + 1, pieceY))
            {
                RotatePiece();
            }
            break;
        }
        break;

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        ReleaseDC(hwnd, hdc);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"TetrisTemp";
    RegisterClass(&wc);

    hwnd = CreateWindow(wc.lpszClassName, L"Tetris", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight, NULL, NULL, hInstance, NULL);

    std::thread gameThread(GameThread, hwnd);
    gameThread.detach();

    if (hwnd == NULL)
        return 0;

    ShowWindow(hwnd, nCmdShow);

    wc.lpfnWndProc = WndProc;

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
