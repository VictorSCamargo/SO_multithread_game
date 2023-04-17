#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>


int x;

int total_segundos_jogo = 5;

int nave_x;
int nave_y;

int missel_x;
int missel_y;

//Função gotoxy
void gotoxy(int x, int y)
{
  COORD coord;
  coord.X = x;
  coord.Y = y;
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void bomba_horizontal(int x, int y){
	gotoxy(x, y);
	printf("<");
	gotoxy(x+1, y);
	printf("___");
}
void apaga_bomba_horizontal(int x, int y){
	gotoxy(x, y);
	printf(" ");
	gotoxy(x+1, y);
	printf("   ");
}
void bomba(int x, int y){
	gotoxy(x, y);
	printf("/\\");
	gotoxy(x, y+1);
	printf("||");
	gotoxy(x-1, y+2);
	printf("/||\\");
}
void apaga_bomba(int x, int y){
	gotoxy(x, y);
	printf("  ");
	gotoxy(x, y+1);
	printf("  ");
	gotoxy(x-1, y+2);
	printf("    ");
}
void explode_bomba(int x, int y){
	gotoxy(x, y);
	printf("*");
	Sleep(50);
	printf(" ");
	gotoxy(x, y-1);
	printf("O");
	gotoxy(x-1, y);
	printf("O O");
	gotoxy(x, y+1);
	printf("O");
	Sleep(50);
	gotoxy(x, y-1);
	printf(" ");
	gotoxy(x-1, y);
	printf("   ");
	gotoxy(x, y+1);
	printf(" ");


	gotoxy(x, y-2);
	printf("o");
	gotoxy(x-2, y);
	printf("o   o");
	gotoxy(x, y+2);
	printf("o");
	Sleep(50);
	gotoxy(x, y-2);
	printf(" ");
	gotoxy(x-2, y);
	printf("     ");
	gotoxy(x, y+2);
	printf(" ");
}
void nave(int x, int y){
	gotoxy(x, y);
	printf("/");
	gotoxy(x+1, y-1);
	printf("___");
	gotoxy(x+4, y);
	printf("\\");
	gotoxy(x, y+1);
	printf("v-V-v");
	gotoxy(x, y+1);
	printf("V-v-V");
}
void apaga_nave(int x, int y){
	gotoxy(x, y);
	printf(" ");
	gotoxy(x+1, y-1);
	printf("   ");
	gotoxy(x+4, y);
	printf(" ");
	gotoxy(x, y+1);
	printf("     ");
	gotoxy(x, y+1);
	printf("     ");
}
void canhao(int x, int y){
	gotoxy(x, y);
	printf("+-+");
	gotoxy(x, y+1);
	printf("| |");
	gotoxy(x, y+2);
	printf("| |");
	gotoxy(x, y+3);
	printf("+-+");
	}

void detecta_colisao() {

    static const int hitbox_nave = 10;

    if (missel_x > (nave_x - hitbox_nave) &&
        missel_x < (nave_x + hitbox_nave) &&
        missel_y > (nave_y - hitbox_nave) &&
        missel_y < (nave_y + hitbox_nave)
        ){
            //ToDo sumir missel e nave e explodir
        }
}

DWORD WINAPI timer_do_jogo(LPVOID lpParameter) {
    while (total_segundos_jogo > 0){
        total_segundos_jogo--;
        Sleep(1000);
    }
    ExitThread(0);
}

// pode estar em outra thread mas acho que nao precisa
void criar_montanhas() {
    static char montanhas[] = {
        10, 17, 14, 18, 16, 19, 12, 18, 13, 24,
        10, 17, 14, 18, 16, 19, 12, 18, 13, 24,
        10, 17, 14, 18, 16, 19, 12, 18, 13, 24,
        10, 17, 14, 18, 16, 19, 12, 18, 13, 24
    };

    int k = 0;

    for (int i = 0; i<18; ){
        //printf("aqui");
        for (int j=montanhas[i]; j<=montanhas[i+1]; j++){
            gotoxy(k, j);//printf("%i %i\n", k, j);//
            printf("\\");
            k++;
        }

        for (int j=montanhas[i]; j>=montanhas[i+1]; j--){
            gotoxy(k, j);//printf("%i %i\n", k, j);//
            printf("/");
            k++;
        }
        i++;
    }
}

// ToDo lembrar de matar as threads abertas caso o jogo acabe
int main(){
    int coluna = 5;
    int linha  = 3;
    int k=0;
    system("cls");

    HANDLE handle_timer_do_jogo = CreateThread(NULL, 0, timer_do_jogo, NULL, 0, NULL);

    if (handle_timer_do_jogo == NULL) { // se falhar na criacao da thread
        return 420;
    }

    criar_montanhas();

  	// Providing a seed value
    srand(time(NULL));
    k=21;

    canhao(40, 23);
    bomba_horizontal(43, 26);
    bomba_horizontal(43, 25);
    bomba_horizontal(43, 24);
    bomba_horizontal(43, 23);
    bomba_horizontal(43, 22);
    bomba_horizontal(43, 21);
	while(1){
        if(total_segundos_jogo <= 0) {
            return 0;
        }

        linha = 2 + (rand() % 5);
        for (int x=79; x >=0; x--){
            nave(x,linha);	Sleep(40);
            apaga_nave(x,linha);
        }
        apaga_bomba_horizontal(43, k);
        for (int y=20; y >=5; y--){
            bomba(40,y);	Sleep(40);
            apaga_bomba(40,y);
        }
        explode_bomba(40, 7);
        k++;
        if (k==26){
            bomba_horizontal(43, 26);
        }
    }
    getche();
}

