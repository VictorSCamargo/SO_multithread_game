#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>

typedef struct {
    int x;
    int y;
    int is_alive;
    int was_hit;
} NaveModel;

typedef struct {
    int x;
    int y;
} MissilModel;

// defines sao jeitos de definir constantes em nivel de compilacao (compilador interpreta)
#define NUM_DE_NAVES 20
#define TEMPO_MAXIMO_JOGO 60
#define COOLDOWN_RESPAWN_NAVE 1000
#define MAX_MUNICOES 6
#define DELAY_PRODUCAO_MISSIL 2000

NaveModel naves[NUM_DE_NAVES]; //array de naves (maximo de naves vivas simultaneamente)
COORD coord_explosao;

// Nao critico: editado apenas em uma thread
int total_segundos_jogo = TEMPO_MAXIMO_JOGO; //thread vai decrementar esse valor

// Soh eh mudado para 1 e acaba jogo
int acabou_jogo = 0; // quando deixar de ser zero as threads irao ser finalizadas

// Definida ao escolher dificuldade
int delay_base_deslocamento_nave;

int municoes_disponiveis = 6;
int y_ultimo_missel = 21;

int naves_destruidas = 0;

HANDLE semaforo_goto;
HANDLE semaforo_coord_explosao;
HANDLE semaforo_naves_destruidas;

HANDLE semaforo_missil_disparado;
HANDLE mutex_municoes_disponiveis;
HANDLE handle_produtor;

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

DWORD WINAPI ProdutorMisseis(LPVOID lpParam) {
    while (!acabou_jogo) {
        if (municoes_disponiveis < MAX_MUNICOES) {
            WaitForSingleObject(mutex_municoes_disponiveis, INFINITE);
            municoes_disponiveis++;
            y_ultimo_missel--;
            WaitForSingleObject(semaforo_goto, INFINITE);
            bomba_horizontal(43, y_ultimo_missel);
            ReleaseSemaphore(semaforo_goto, 1, NULL);
            ReleaseMutex(mutex_municoes_disponiveis);
        }
        Sleep(DELAY_PRODUCAO_MISSIL); // Delay entre a produção de mísseis
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

DWORD WINAPI desenha_explosao(LPVOID lpParameter) {

    COORD coord;
    coord.X = coord_explosao.X;
    coord.Y = coord_explosao.Y;

    ReleaseSemaphore(semaforo_coord_explosao, 1, NULL);

    WaitForSingleObject(semaforo_goto, INFINITE);
	gotoxy(coord.X, coord.Y);
	printf("*");
    ReleaseSemaphore(semaforo_goto, 1, NULL);

	Sleep(50);

    WaitForSingleObject(semaforo_goto, INFINITE);
	printf(" ");
	gotoxy(coord.X, coord.Y-1);
	printf("O");
	gotoxy(coord.X-1, coord.Y);
	printf("O O");
	gotoxy(coord.X, coord.Y+1);
	printf("O");
    ReleaseSemaphore(semaforo_goto, 1, NULL);

	Sleep(50);

    WaitForSingleObject(semaforo_goto, INFINITE);
	gotoxy(coord.X, coord.Y-1);
	printf(" ");
	gotoxy(coord.X-1, coord.Y);
	printf("   ");
	gotoxy(coord.X, coord.Y+1);
	printf(" ");
	gotoxy(coord.X, coord.Y-2);
	printf("o");
	gotoxy(coord.X-2, coord.Y);
	printf("o   o");
	gotoxy(coord.X, coord.Y+2);
	printf("o");
    ReleaseSemaphore(semaforo_goto, 1, NULL);

	Sleep(50);

    WaitForSingleObject(semaforo_goto, INFINITE);
	gotoxy(coord.X, coord.Y-2);
	printf(" ");
	gotoxy(coord.X-2, coord.Y);
	printf("     ");
	gotoxy(coord.X, coord.Y+2);
	printf(" ");
    ReleaseSemaphore(semaforo_goto, 1, NULL);

    ExitThread(0);
}

DWORD WINAPI movimento_missil(LPVOID lpParameter) {
    static const int x_inicial_missil = 40;
    static const int y_inicial_missil = 20;

    static const int hitbox_nave_x = 4;
    static const int hitbox_nave_y = 2;

    MissilModel missil;
    missil.x = x_inicial_missil;
    missil.y = y_inicial_missil;

    int missil_destruido = 0;

    for (missil.y = y_inicial_missil; missil.y >=2; missil.y--){
        if(acabou_jogo) {
            break;
        }

        // verifica se missil esta colidindo com alguma nave
        for(int i = 0; i < NUM_DE_NAVES; i++){
            // se posicao de missil e nave (com hitbox maior) estiverem batendo
            if ( naves[i].is_alive &&
                (missil.x >= (naves[i].x - hitbox_nave_x)) &&
                (missil.x <= (naves[i].x + hitbox_nave_x)) &&
                (missil.y >= (naves[i].y - hitbox_nave_y)) &&
                (missil.y <= (naves[i].y + hitbox_nave_y))
            ){
                //marca para nave se destruir na sua thread
                naves[i].was_hit = 1;

                WaitForSingleObject(semaforo_naves_destruidas, INFINITE);
                naves_destruidas += 1;
                ReleaseSemaphore(semaforo_naves_destruidas, 1, NULL);

                WaitForSingleObject(semaforo_coord_explosao, INFINITE); // deve ser liberado na thread de desenhar explosao
                coord_explosao.X = missil.x;
                coord_explosao.Y = missil.y;

                // tenta criar thread. Se falhar printa.
                if(!CreateThread(NULL, 0, desenha_explosao, &coord_explosao, 0, NULL)) {
                    WaitForSingleObject(semaforo_goto, INFINITE);
                    gotoxy(0, 0);
                    printf("Nao criou explosao");
                    ReleaseSemaphore(semaforo_goto, 1, NULL);
                }

                missil_destruido = 1; //marca para sair do loop em seguida

                if (naves_destruidas >= NUM_DE_NAVES){
                    acabou_jogo = 1;
                }

                // sai do loop
                break;
            }
        }

        if(missil_destruido){
            break; // sai do loop
        }

        WaitForSingleObject(semaforo_goto, INFINITE);
        bomba(missil.x, missil.y);
        ReleaseSemaphore(semaforo_goto, 1, NULL);

        Sleep(40);

        WaitForSingleObject(semaforo_goto, INFINITE);
        apaga_bomba(missil.x, missil.y);
        ReleaseSemaphore(semaforo_goto, 1, NULL);
    }
    ExitThread(0);
}

DWORD WINAPI movimento_nave(LPVOID lpParameter) {

    // interpreta dados do ponteiro recebido como do tipo NaveModel
    NaveModel* nave_model = (NaveModel*)lpParameter;

    // deixa a velocidade da nave levemente aleatoria baseado na dificuldade
    int velocidade_nave = delay_base_deslocamento_nave - 3 + (rand() % 7);

    // enquanto nave nao chegou no fim da tela
    while (nave_model->x >= 0){

        // encerra precoce se nave foi morta por missil ou acabou jogo
        if(nave_model->was_hit || acabou_jogo) {
            nave_model->is_alive = 0; // marca como morta para poder ser spawnada novamente
            break;
        }

        WaitForSingleObject(semaforo_goto, INFINITE);
        nave(nave_model->x,nave_model->y);
        ReleaseSemaphore(semaforo_goto, 1, NULL);

        Sleep(velocidade_nave);

        WaitForSingleObject(semaforo_goto, INFINITE);
        apaga_nave(nave_model->x, nave_model->y);
        ReleaseSemaphore(semaforo_goto, 1, NULL);

        nave_model->x--;
    }

    ExitThread(0);
}

DWORD WINAPI spawner_nave(LPVOID lpParameter) {

    static const int x_inicial_nave = 99;

    // inicia naves com valor zerado
    for(int i = 0; i < NUM_DE_NAVES; i++) {
        naves[i].x = 0;
        naves[i].y = 0;
        naves[i].is_alive = 0;
        naves[i].was_hit = 0;
    }

    while(!acabou_jogo){
        // cooldown pra respawnar nave
        Sleep(COOLDOWN_RESPAWN_NAVE + (rand() % 1000));

        // busca por alguma nave morta para spawnar
        for(int i = 0; i < NUM_DE_NAVES; i++) {
            if(!naves[i].is_alive){

                // inicia dados da nave que vai spawnar
                naves[i].x = x_inicial_nave;
                naves[i].y = 2 + (rand() % 5);
                naves[i].is_alive = 1;
                naves[i].was_hit = 0;

                // tenta criar thread da nave e passa endereco da nave. Se falhar printa.
                if(!CreateThread(NULL, 0, movimento_nave, &naves[i], 0, NULL)) {
                    WaitForSingleObject(semaforo_goto, INFINITE);
                    gotoxy(0, 0);
                    printf("Nao criou nave");
                    ReleaseSemaphore(semaforo_goto, 1, NULL);
                }

                break; //sai do for e entra em cooldown de respawn dnv
            }
        }
    }
    ExitThread(0);
}

DWORD WINAPI interpreta_input(LPVOID lpParameter) {
    char ch;

    while(!acabou_jogo) {
        ch = getch(); // pega caractere sem imprimir na tela

        // se apertou espaco
        if(ch == ' '){
            if (municoes_disponiveis > 0) {
                WaitForSingleObject(mutex_municoes_disponiveis, INFINITE);

                WaitForSingleObject(semaforo_goto, INFINITE);
                apaga_bomba_horizontal(43,y_ultimo_missel);
                ReleaseSemaphore(semaforo_goto, 1, NULL);

                municoes_disponiveis--;
                y_ultimo_missel++;
                ReleaseMutex(mutex_municoes_disponiveis);

                // tenta criar thread do missil. Se falhar printa.
                if(!CreateThread(NULL, 0, movimento_missil, NULL, 0, NULL)) {
                    WaitForSingleObject(semaforo_goto, INFINITE);
                    gotoxy(0, 0);
                    printf("Nao criou missil");
                    ReleaseSemaphore(semaforo_goto, 1, NULL);
                }
            }
        }
        Sleep(300); // cooldown minimo entre disparos
    }
    ExitThread(0);
}


int main(){
    system("cls");

    printf("Selecione a dificuldade [0]facil [1]medio [2]dificil: ");
    char ch = getche();

    switch(ch){
        case '0':
            delay_base_deslocamento_nave = 36;
            break;
        case '1':
            delay_base_deslocamento_nave = 24;
            break;
        case '2':
            delay_base_deslocamento_nave = 12;
            break;
        default:
            printf("\nValor nao reconhecido. Setando dificuldade 'medio'...");
            Sleep(2000);
            delay_base_deslocamento_nave = 24;
            break;
    }
    system("cls"); //clear screen

  	// Providing a seed value for srand function
    srand(time(NULL));

    // cria desenhos iniciais da tela
    criar_montanhas();
    canhao(40, 23);
    bomba_horizontal(43, 26);
    bomba_horizontal(43, 25);
    bomba_horizontal(43, 24);
    bomba_horizontal(43, 23);
    bomba_horizontal(43, 22);
    bomba_horizontal(43, 21);

    // cria threads
    HANDLE handle_timer_do_jogo = CreateThread(NULL, 0, timer_do_jogo, NULL, 0, NULL);
    HANDLE handle_interpreta_input = CreateThread(NULL, 0, interpreta_input, NULL, 0, NULL);
    HANDLE handle_spawner_nave = CreateThread(NULL, 0, spawner_nave, NULL, 0, NULL);
    HANDLE handle_produtor = CreateThread(NULL, 0, ProdutorMisseis, NULL, 0, NULL);

    // cria mecanismos de sinalizacao
    semaforo_goto = CreateSemaphore(NULL, 1, 1, NULL);
    semaforo_coord_explosao = CreateSemaphore(NULL, 1, 1, NULL);
    semaforo_naves_destruidas = CreateSemaphore(NULL, 1, 1, NULL);
    semaforo_missil_disparado = CreateSemaphore(NULL, 1, 1, NULL);
    mutex_municoes_disponiveis = CreateMutex(NULL, FALSE, NULL);

    // se falhar a criacao de algo
    if (handle_timer_do_jogo == NULL ||
        handle_spawner_nave == NULL ||
        handle_interpreta_input == NULL ||
        semaforo_goto == NULL ||
        semaforo_coord_explosao == NULL ||
        semaforo_naves_destruidas == NULL ||
        semaforo_missil_disparado == NULL ||
        mutex_municoes_disponiveis == NULL ||
        handle_produtor == NULL
        ) {
        acabou_jogo = 1;
        return -1;
    }

	while(1){
        // espera processos acabarem, exceto o que trata input pois ele fica bloqueado por causa da espera de input
        WaitForSingleObject(handle_timer_do_jogo, INFINITE);
        WaitForSingleObject(handle_spawner_nave, INFINITE);
        WaitForSingleObject(handle_produtor, INFINITE);

        if(acabou_jogo) {
            // fecha os handles dos dispositivos de sincronizacao
            CloseHandle(semaforo_coord_explosao);
            CloseHandle(mutex_municoes_disponiveis);
            CloseHandle(semaforo_missil_disparado);
            CloseHandle(semaforo_naves_destruidas);

            // fecha os handles das threads
            CloseHandle(handle_produtor);
            CloseHandle(handle_timer_do_jogo);
            CloseHandle(handle_spawner_nave);
            CloseHandle(handle_interpreta_input);

            // imprimir jogo finalizado
            WaitForSingleObject(semaforo_goto, INFINITE);
            system("cls");
            gotoxy(0, 0);
            printf("Jogo finalizado!");
            ReleaseSemaphore(semaforo_goto, 1, NULL);

            Sleep(3000);

            // finalizar handle final
            CloseHandle(semaforo_goto);

            return 0;
        }
    }
}

