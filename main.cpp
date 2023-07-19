#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>


//game field
const int fld_width = 20;
const int fld_height = 30;
const int scr_width = fld_width * 2;
const int scr_height = fld_height;

typedef char TScreenMap [scr_height][scr_width];
typedef char TFieldMap[fld_height][fld_width];

// field and figure symbols
const char c_fig = -37;
const char c_field = -80;
const char c_figDwn = -78;

//shape
const int shp_width = 4;
const int shp_height = 4;
typedef char TShape[shp_height][shp_width];

//array of shapes of figure
char *shpArr[] = {
        (char*) ".....**..**.....", // box
        (char*) "....****........", // line
        (char*) "....***..*......", // T
        (char*) ".....***.*......", // left L
        (char*) ".....**.**......", // right horror
        (char*) "........*......."}; //dot
        const int shpArrCnt = sizeof(shpArr) / sizeof(shpArr[0]);


//set cursor position
void SetCurPos (int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//Screen class
class TScreen {
    void SetEnd ()
    {
        scr[scr_height-1][scr_width-1] = '\0';
    }

public:
    TScreenMap scr;
    TScreen() { Clear();}

    void Clear()
    {
        memset (scr, '.', sizeof(scr));
    }

    void Show()
    {
        SetCurPos(0,0);
        SetEnd();
        std::cout << scr[0];
    }
};


class TField {
public:
    TFieldMap fld;
    TField() { Clear();}
    void Clear() { memset(fld, c_field, sizeof(fld)); }
    void Put (TScreenMap &scr);
    void Burning(); //delete line if filled
};

void TField::Put(TScreenMap &scr)
{
    for (int i = 0; i < fld_width; i++)
        for (int j = 0; j < fld_height; j++)
            scr[j][i*2] = scr[j][i*2 +1] = fld[j][i];
}

void TField::Burning()
{
    for (int j = fld_height - 1; j >=0; j--)
    {
        static bool fillLine;
        fillLine = true;
        for (int i = 0; i < fld_width; i++)
            if (fld[j][i] != c_figDwn)
                fillLine = false;
        if (fillLine)
        {
            for (int y = j; y >= 1; y--)
                memcpy(fld[y], fld[y-1], sizeof(fld[y]));
            return;
        }
    }
}

class TFigure {
    int x,y;
    TShape vid;
    char turn;
    COORD coord[shp_width * shp_height];
    int coordCnt;
    TField *field = 0;

public:
    TFigure() { memset(this,0, sizeof(*this));}
    void FieldSet (TField *_field) { field = _field; }
    void Shape (const char* _vid) {memcpy (vid, _vid, sizeof(vid));}
    void Pos(int _x, int _y) { x= _x; y = _y; CalcCoord();}
    char TurnGet() { return turn; }
    void TurnSet (char _turn);
    void Put (TScreenMap &scr);
    void Put (TFieldMap &fld);
    bool Move (int dx, int dy);
    int Check();

private:
    void CalcCoord();
};


class TGame {
    TScreen screen;
    TField field;
    TFigure figure;

public:
    TGame();
    void PlayerControl();
    void Move();
    void Show();
};

TGame::TGame()
{
    figure.FieldSet(&field);
    figure.Shape(shpArr[rand() % shpArrCnt]);
    //figure.Shape(shpArr[0]);
    figure.Pos(fld_width / 2 - shp_width / 2, 0);
}

void TGame::Show()
{
    screen.Clear();
    field.Put(screen.scr);
    figure.Put(screen.scr);
    screen.Show();
}

void TGame::PlayerControl()
{
    static int trn = 0;
    if(GetKeyState('W') < 0) trn += 1;
    if (trn == 1) figure.TurnSet(figure.TurnGet() + 1), trn++;
    if(GetKeyState('W') >= 0) trn = 0;

    if (GetKeyState('S') < 0) figure.Move (0,1);
    if (GetKeyState('A') < 0) figure.Move (-1,0);
    if (GetKeyState('D') < 0) figure.Move (1,0);
}

void TGame::Move()
{
    static int tick = 0;
    tick++;
    if (tick >= 5)
    {
        if (!figure.Move(0,1)) //new figure
        {
            figure.Put(field.fld);
            figure.Shape(shpArr[rand() % shpArrCnt]);
         //   figure.Shape(shpArr[4]);
            figure.Pos(fld_width/2 - shp_width/2,0);
            if (figure.Check() > 0) //New game if filled till top
                field.Clear();
        }
        field.Burning();
        tick = 0;
    }
}

void TFigure::Put(TScreenMap &scr)
{
    for (int i = 0; i < coordCnt; i++)
        scr[coord[i].Y][coord[i].X*2] = scr[coord[i].Y][coord[i].X*2+1] = c_fig;
}

bool TFigure::Move(int dx, int dy) {
    int oldX = x, oldY = y;
    Pos(x + dx, y + dy);
    int chk = Check();
    if (chk >= 1)
    {
       Pos(oldX, oldY);
       if (chk == 2)
       return false; //we need to land the figure and create the new one
    }
    return true;
}

int TFigure::Check()
{
    CalcCoord();
    for (int i = 0; i < coordCnt; i++) // first return figure back
        if (coord[i].X < 0 || coord[i].X >= fld_width)
            return 1;
    for (int i = 0; i < coordCnt; i++) //if figure on field and fall then landing
        if (coord[i].Y >= fld_height || field -> fld[coord[i].Y][coord[i].X] == c_figDwn)
            return 2;
    return 0;
}

void TFigure::CalcCoord()
{
    int xx, yy;
    coordCnt = 0;
    for (int i = 0; i < shp_width; i++)
        for (int j = 0; j < shp_height; j++)
            if (vid[j][i] == '*')
            {
                //xx = x+i, yy = y+j;
                if (turn == 0) xx = x + i, yy = y + j;
                if (turn == 1) xx = x + (shp_height - j - 1), yy = y + i;
                if (turn == 2) xx = x + (shp_width - i - 1), yy = y + (shp_height - j - 1);
                if (turn == 3) xx = x + j, yy = y + (shp_height - i - 1) + (shp_width - shp_height);
                coord[coordCnt] = (COORD){(short) xx, (short) yy};
                coordCnt++;
            }
}

void TFigure::Put(TFieldMap &fld)
{
    for (int i = 0; i < coordCnt; i++)
        fld[coord[i].Y][coord[i].X] = c_figDwn;
}

void TFigure::TurnSet(char _turn)
{
    int oldTurn = turn;
    turn = (_turn > 3 ? 0 : (_turn < 0 ? 3 : _turn));
    int chk = Check();
    if (chk == 0) return;
    if (chk == 1)
    {
        int xx = x;
        int k = (x > (fld_width / 2) ? -1 : +1);
        for (int i = 1; i < 3; i++)
        {
            x +=k;
            if (Check() == 0) return;
        }
        x = xx;
    }
    turn = oldTurn;
    CalcCoord();
}

int main ()
{
    ShowCursor(FALSE);
    char command [1000];
    sprintf(command, "mode con cols=%d lines=%d", scr_width, scr_height);
    system(command);

    srand(time(nullptr));

    TGame game;
    while (1)
    {
        game.PlayerControl();
        game.Move();
        game.Show();
        if (GetKeyState(VK_ESCAPE) < 0) break;
        Sleep (50);
    }
    return 0;
}
