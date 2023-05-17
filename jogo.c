#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>

// ToDos
// Recarregar missil sincronamente caso estoque nao esteja cheio (lembrar de desenhar ele na tela)
// Apagar missil conforme ele eh utilizado
// Acabar jogo ao eliminar 20 naves
// Impedir disparo do missil se n tiver municao
// (revisar se nao falta algo no moodle)

// Feitos
// Detectar input do usuario para disparar missil
// Movimento da nave
// Movimento do missil
// Timer de tempo maximo do jogo
// Variavel para alterar dificuldade (que muda a velocidade das naves)
// Detectar colisao entre nave e missil, apagar ambos objetos e gerar explosao


int cooldown_respawn_nave_minimo = 2000;

int dificuldade = 0; // 0, 1 ou 2: muda a velocidade da nave

int total_segundos_jogo = 60;
int acabou_jogo = 0;    //false

int nave_x;
int nave_y;

int missil_x = 40; // fixo
int missil_y;

int delay_deslocamento_nave;

// para sinalizar para a thread do missil e nave destruirem ele
int destruir_missil = 0;
int destruir_nave = 0;

int disparar_missil = 0;

HANDLE semaforo_goto;
HANDLE semaforo_missil_disparado;

//Fun��o gotoxy
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

    static const int hitbox_nave_x = 4;
    static const int hitbox_nave_y = 2;

    while (!acabou_jogo){
        Sleep(10); // delay para nao checar tanto

        // se posicao de missil e nave (com hitbox maior) estiverem batendo
        if ((missil_x >= (nave_x - hitbox_nave_x)) &&
            (missil_x <= (nave_x + hitbox_nave_x)) &&
            (missil_y >= (nave_y - hitbox_nave_y)) &&
            (missil_y <= (nave_y + hitbox_nave_y))
        ){
            int posicao_explosao_x = missil_x;
            int posicao_explosao_y = missil_y;

            destruir_missil = 1;
            destruir_nave = 1;

            WaitForSingleObject(semaforo_goto, INFINITE);
            explode_bomba(posicao_explosao_x, posicao_explosao_y);
            ReleaseSemaphore(semaforo_goto, 1, NULL);

            Sleep(cooldown_respawn_nave_minimo);
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

DWORD WINAPI movimento_missil(LPVOID lpParameter) {
    static const int y_inicial_missil = 20;

    while(!acabou_jogo){

        while(disparar_missil == 0);

        for (missil_y = y_inicial_missil; missil_y >=2; missil_y--){
            if(destruir_missil) {
                destruir_missil = 0;
                break;
            }

            WaitForSingleObject(semaforo_goto, INFINITE);
            bomba(missil_x,missil_y);
            ReleaseSemaphore(semaforo_goto, 1, NULL);

            Sleep(40);

            WaitForSingleObject(semaforo_goto, INFINITE);
            apaga_bomba(missil_x,missil_y);
            ReleaseSemaphore(semaforo_goto, 1, NULL);
        }
        missil_y = y_inicial_missil;
        disparar_missil = 0;
    }
    ExitThread(0);
}

DWORD WINAPI movimento_nave(LPVOID lpParameter) {
    static const int x_inicial_nave = 79;

    while(!acabou_jogo){

        Sleep(cooldown_respawn_nave_minimo + (rand() % 2000));

        nave_y = 2 + (rand() % 5);
        for (nave_x = x_inicial_nave; nave_x >=0; nave_x--){
            if(destruir_nave) {
                destruir_nave = 0;
                break;
            }
            
            WaitForSingleObject(semaforo_goto, INFINITE);
            nave(nave_x,nave_y);
            ReleaseSemaphore(semaforo_goto, 1, NULL);

            Sleep(delay_deslocamento_nave);

            WaitForSingleObject(semaforo_goto, INFINITE);
            apaga_nave(nave_x,nave_y);
            ReleaseSemaphore(semaforo_goto, 1, NULL);
        }
        nave_x = x_inicial_nave;
    }
    ExitThread(0);
}

DWORD WINAPI interpreta_input(LPVOID lpParameter) {
    while(!acabou_jogo) {
        getche();
        if(disparar_missil == 0){
            disparar_missil = 1;
        }
        Sleep(1000);
    }
    ExitThread(0);
}

// ToDo lembrar de matar as threads abertas caso o jogo acabe
// ToDo funcao gotoxy precisa de cuidado para ser manipulada por 2 threads se nao buga a impressao
int main(){
    int coluna = 5;
    int linha  = 3;
    int k=0;
    system("cls");

    // ToDo tirar ao final
    //printf("Selecione a dificuldade [0]facil [1]medio [2]dificil: ");
    //scanf("%d", &dificuldade);

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
            printf("Valor nao reconhecido. Setando dificuldade 'medio'...");
            Sleep(2500);
            delay_deslocamento_nave = 30;
            break;
    }
    system("cls"); //clear screen

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

    HANDLE handle_timer_do_jogo = CreateThread(NULL, 0, timer_do_jogo, NULL, 0, NULL);
    HANDLE handle_movimento_missil = CreateThread(NULL, 0, movimento_missil, NULL, 0, NULL);
    HANDLE handle_movimento_nave = CreateThread(NULL, 0, movimento_nave, NULL, 0, NULL);
    HANDLE handle_detecta_colisao = CreateThread(NULL, 0, detecta_colisao, NULL, 0, NULL);
    HANDLE handle_interpreta_input = CreateThread(NULL, 0, interpreta_input, NULL, 0, NULL);

    semaforo_goto = CreateSemaphore(NULL, 1, 1, NULL);
    semaforo_missil_disparado = CreateSemaphore(NULL, 1, 1, NULL);

    // se falhar a criacao de algo
    if (handle_timer_do_jogo == NULL ||
        semaforo_goto == NULL ||
        handle_movimento_missil == NULL ||
        handle_movimento_nave == NULL ||
        handle_detecta_colisao == NULL ||
        handle_interpreta_input == NULL
        ) {
        return -420;
    }

	while(1){
        Sleep(100);
        if(acabou_jogo) {
            CloseHandle(semaforo_goto);
            return 0;
        }
    }
}

