#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <stdlib.h>
#include <time.h>

const char* seq = "PPPHHPPHHPPPPPHHHHHHHPPHHPPPPHHPPHPP";
const double p1 = 0.8, p2 = 0.5;
size_t lenseq = 0;
int* connections;

typedef struct {
    int x, y;
} vec;

typedef struct {
    vec dir, pos;
    int feasible;
} check;

const vec LEFT = {0, -1};
const vec UP = {-1, 0};
const vec RIGHT = {0, 1};
const vec DOWN = {1, 0};
const char ACTIONS[3] = {'s', 'r', 'l'};

int min(int a, int b) {
    if (a < b)
        return a;
    return b;
}

int vecEqual(vec vec1, vec vec2) {
    if (vec1.x == vec2.x && vec1.y == vec2.y)
        return 1;
    return 0;
}

vec vecAdd(vec vec1, vec vec2) {
    vec newvec;
    newvec.x = vec1.x + vec2.x;
    newvec.y = vec1.y + vec2.y;
    return newvec;
}

int countConnections(char* conformation, int limit) {
    int conn = 0;
    for (int i = 0; i < limit; i++)
        if (conformation[i] == 'H' && conformation[i+1] == 'H')
            conn++;
    return conn;
}

vec intendedNextDirection(vec dir, char step) {
    if (step == 'r') {
        if (vecEqual(dir, LEFT))
            return UP;
        if (vecEqual(dir, UP))
            return RIGHT;
        if (vecEqual(dir, RIGHT))
            return DOWN;
        return LEFT;
    }
    if (step == 'l') {
        if (vecEqual(dir, LEFT))
            return DOWN;
        if (vecEqual(dir, UP))
            return LEFT;
        if (vecEqual(dir, RIGHT))
            return UP;
        return RIGHT;
    }
    return dir;
}

check checkStep(vec pos, char step, vec dir, int c, char** ma) {
    vec newDir = intendedNextDirection(dir, step);
    vec tempNewPosition = vecAdd(pos, newDir);
    if (ma[tempNewPosition.x][tempNewPosition.y] != '*')
        return (check){newDir, pos, 0};
    return (check){newDir, tempNewPosition, 1};
}

void showLattice(int n, char** ma) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%c", ma[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int emin = INT_MAX;

double nextRand() {
    return (double)rand()/(double)RAND_MAX;
}

int* u, *z, *zcont;
char** bestLattice;

void saveLattice(char** ma, int n) {
    for (int i = 0; i < n; i++)
        memcpy(bestLattice[i], ma[i], sizeof(char) * n);
}


void bb(int olde, char prev, int i, vec pos, vec dir, int n, char** ma) {
    for (int j = 0; j < 3; j++) {
        char action = ACTIONS[j];
        check resCheck = checkStep(pos, action, dir, n, ma);
        if (resCheck.feasible) {
            ma[resCheck.pos.x][resCheck.pos.y] = seq[i];
            int newe = olde;
            if (seq[i] == 'H') {
                if (ma[resCheck.pos.x - 1][resCheck.pos.y] == 'H')
                    newe--;
                if (ma[resCheck.pos.x][resCheck.pos.y + 1] == 'H')
                    newe--;
                if (ma[resCheck.pos.x + 1][resCheck.pos.y] == 'H')
                    newe--;
                if (ma[resCheck.pos.x][resCheck.pos.y - 1] == 'H')
                    newe--;
                if (prev == 'H')
                    newe++;
            }
            ma[resCheck.pos.x][resCheck.pos.y] = '*';
            if (newe < u[i])
                u[i] = newe;
            if (i == lenseq - 1) {
                ma[resCheck.pos.x][resCheck.pos.y] = seq[i];
                if (newe < emin) {
                    emin = newe;
                    saveLattice(ma, n);
                }
                ma[resCheck.pos.x][resCheck.pos.y] = '*';
                return;
            } else {
                if (seq[i] == 'H') {
                    if (newe <= u[i]) {
                        ma[resCheck.pos.x][resCheck.pos.y] = seq[i];
                        bb(newe, seq[i], i + 1, resCheck.pos, resCheck.dir, n, ma);
                        z[i] +=  newe/(zcont[i] + 1);
                        zcont[i] += 1;
                        ma[resCheck.pos.x][resCheck.pos.y] = '*';
                    }
                    else if (newe > u[i]) {
                        double r = nextRand();
                        if (r > p1) {
                            ma[resCheck.pos.x][resCheck.pos.y] = seq[i];
                            bb(newe, seq[i], i + 1, resCheck.pos, resCheck.dir, n, ma);
                            z[i] +=  newe/(zcont[i] + 1);
                            zcont[i] += 1;
                            ma[resCheck.pos.x][resCheck.pos.y] = '*';
                        }
                    }
                    else if (newe >= u[i] && newe <= z[i]) {
                        double r = nextRand();
                        if (r > p2) {
                            ma[resCheck.pos.x][resCheck.pos.y] = seq[i];
                            bb(newe, seq[i], i + 1, resCheck.pos, resCheck.dir, n, ma);
                            z[i] +=  newe/(zcont[i] + 1);
                            zcont[i] += 1;
                            ma[resCheck.pos.x][resCheck.pos.y] = '*';
                        }
                    }
                } else {
                    ma[resCheck.pos.x][resCheck.pos.y] = seq[i];
                    bb(newe, seq[i], i + 1, resCheck.pos, resCheck.dir, n, ma);
                    z[i] +=  newe/(zcont[i] + 1);
                    zcont[i] += 1;
                    ma[resCheck.pos.x][resCheck.pos.y] = '*';
                }
            }
        }
    }
}


int main() {
    srand(time(NULL) + getpid());
    lenseq = strlen(seq);
    size_t n = lenseq * 2 + 1;
    char** ma = malloc(sizeof(char*) * n);
    bestLattice = malloc(sizeof(char*) * n);
    for (int i = 0; i < n; i++) {
        ma[i] = malloc(sizeof(char) * n);
        bestLattice[i] = malloc(sizeof(char) * n);
        for (int j = 0; j < n; j++)
            ma[i][j] = '*';
    }
    u = malloc(sizeof(int) * lenseq);
    z = malloc(sizeof(int) * lenseq);
    zcont = malloc(sizeof(int) * lenseq);

    memset(u, 0, sizeof(int) * lenseq);
    memset(z, 0, sizeof(int) * lenseq);
    memset(zcont, 0, sizeof(int) * lenseq);

    connections = malloc(sizeof(int) * lenseq);
    for (int i = 0; i < lenseq; i++)
        connections[i] = countConnections(seq, i);
    ma[lenseq][lenseq] = seq[0];
    ma[lenseq][lenseq - 1] = seq[1];
    int energy = 0;
    if (seq[0] == 'H' && seq[1] == 'H')
        energy = -1;
    energy += connections[1];
    bb(energy, seq[1], 2, (vec){lenseq, lenseq - 1}, LEFT, n, ma);
    printf("min: %d\n", emin);
    showLattice(n, bestLattice);
    return 0;
}