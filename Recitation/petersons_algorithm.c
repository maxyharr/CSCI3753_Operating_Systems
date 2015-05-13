#define FALSE 0;
#define TRUE 1;
#define N 2; // Number of process

int turn;
int interested[N];

void enter_region(int process) {
	int other;
	other = 1 - process;
	interested[process] = TRUE;
	turn = process;

	while (turn == process && interested[other] == TRUE);

	// do stuff

	void leave_region(int process); // process: who is leaving

	interested[process] = FALSE;
}