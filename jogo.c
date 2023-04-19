#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>

// ToDos
// Detectar colisao entre nave e missil, apagar ambos objetos e gerar explosao
// Detectar input do usuario para disparar missil
// Recarregar missil sincronamente caso nao estoque nao esteja cheio (lembrar de desenhar ele na tela)
// Apagar missel conforme ele é utilizado
// Acabar jogo ao eliminar 20 naves
// (revisar se nao falta algo no moodle)

// Feitos
// Movimento da nave
// Movimento do missil
// Timer de tempo maximo do jogo
// Variavel para alterar dificuldade (que muda a velocidade das naves)

int dificuldade = 0; // 0, 1 ou 2: muda a velocidade da nave

int total_segundos_jogo = 5;
int acabou_jogo = 0;    //false

int nave_x;
int nave_y;

int missel_x;
int missel_y;

int delay_deslocamento_nave; //default

HANDLE semaforo_goto;

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

DWORD WINAPI detecta_colisao(LPVOID lpParameter) {

    static const int hitbox_nave = 10;

    while (!acabou_jogo){
        Sleep(100);
        if (missel_x > (nave_x - hitbox_nave) &&
            missel_x < (nave_x + hitbox_nave) &&
            missel_y > (nave_y - hitbox_nave) &&
            missel_y < (nave_y + hitbox_nave)
        ){
            //ToDo sumir missel e nave e explodir

        }
    }
    ExitThread(0);
}

DWORD WINAPI timer_do_jogo(LPVOID lpParameter) {
    while (!acabou_jogo){
        if(total_segundos_jogo > 0) {
            total_segundos_jogo--;
            Sleep(1000);
        }
        else {
            acabou_jogo = 1;
        }
    }
    ExitThread(0);
}

DWORD WINAPI movimento_missel(LPVOID lpParameter) {
    while(!acabou_jogo){

        for (int y=20; y >=2; y--){
            WaitForSingleObject(semaforo_goto, INFINITE);
            bomba(40,y);
            ReleaseSemaphore(semaforo_goto, 1, NULL);

            Sleep(40);

            WaitForSingleObject(semaforo_goto, INFINITE);
            apaga_bomba(40,y);
            ReleaseSemaphore(semaforo_goto, 1, NULL);
        }
    }
    ExitThread(0);
}

DWORD WINAPI movimento_nave(LPVOID lpParameter) {
    int linha;
    while(!acabou_jogo){

        linha = 2 + (rand() % 5);
        for (int x=79; x >=0; x--){
            WaitForSingleObject(semaforo_goto, INFINITE);
            nave(x,linha);
            ReleaseSemaphore(semaforo_goto, 1, NULL);

            Sleep(delay_deslocamento_nave);

            WaitForSingleObject(semaforo_goto, INFINITE);
            apaga_nave(x,linha);
            ReleaseSemaphore(semaforo_goto, 1, NULL);
        }
    }
    ExitThread(0);
}

DWORD WINAPI interpreta_input(LPVOID lpParameter) {
    //getche();   // ToDo pegar input
}

// ToDo lembrar de matar as threads abertas caso o jogo acabe
// ToDo funcao gotoxy precisa de cuidado para ser manipulada por 2 threads se nao buga a impressao
int main(){
    int coluna = 5;
    int linha  = 3;
    int k=0;
    system("cls");

    criar_montanhas();

    switch(dificuldade){
        case 0:
            delay_deslocamento_nave = 50;
            break;
        case 1:
            delay_deslocamento_nave = 30;
            break;
        case 2:
            delay_deslocamento_nave = 10;
            break;
        default:
            delay_deslocamento_nave = 30;
            break;
    }

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

    HANDLE handle_timer_do_jogo = CreateThread(NULL, 0, timer_do_jogo, NULL, 0, NULL);
    HANDLE handle_movimento_missel = CreateThread(NULL, 0, movimento_missel, NULL, 0, NULL);
    HANDLE handle_movimento_nave = CreateThread(NULL, 0, movimento_nave, NULL, 0, NULL);

    semaforo_goto = CreateSemaphore(NULL, 1, 1, NULL);

    // se falhar a criacao de algo
    if (handle_timer_do_jogo == NULL ||
        semaforo_goto == NULL ||
        handle_movimento_missel == NULL ||
        handle_movimento_nave == NULL
        ) {
        return 420;
    }

	while(1){
        Sleep(100);
        if(acabou_jogo) {
            CloseHandle(semaforo_goto);
            return 0;
        }
    }
}

