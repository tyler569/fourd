#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct matrix {
    int h, w;
    double v[36];
};
#define V(m, x, y) (m).v[(y)*(m).h+(x)]

void print(struct matrix);

struct matrix mul(struct matrix a, struct matrix b) {
    if (a.h != b.w) {
        printf("Invalid matrix multiplication:\n");
        print(a);
        printf("*\n");
        print(b);
        exit(1);
    }
    struct matrix out = { a.h, b.w, {} };
    for (int col = 0; col < a.h; col++) {
        for (int row = 0; row < b.w; row++) {
            double product = 0;
            for (int dp = 0; dp < a.w; dp++) {
                product += V(a, row, dp)*V(b, dp, col);
            }
            V(out, row, col) = product;
        }
    }
    return out;
}

void print(struct matrix a) {
    for (int x = 0; x < a.h; x++) {
        printf("[");
        for (int y = 0; y < a.w; y++) {
            printf("%g ", V(a, x, y));
        }
        printf("]\n");
    }
}

int main() {
    struct matrix a = { 1, 1, {1} };
    struct matrix b = { 2, 2, {1, 2, 3, 4} };
    struct matrix c = { 4, 1, {1, 2, 3, 4} };
    struct matrix d = { 1, 4, {1, 2, 3, 4} };

    print(a);
    printf("\n");
    print(b);
    printf("\n");
    print(c);
    printf("\n");
    print(d);
    printf("\n");

    struct matrix t1 = { 2, 3, { 1, 4, 2, 5, 3, 6 } };
    struct matrix t2 = { 3, 2, { 7, 9, 11, 8, 10, 12 } };
    print(t1);
    printf("*\n");
    print(t2);
    printf("=\n");
    print(mul(t1, t2));

    struct matrix t3 = { 1, 3, { 1, 2, 3 } };
    struct matrix t4 = { 3, 1, { 4, 5, 6 } };
    print(t3);
    printf("*\n");
    print(t4);
    printf("=\n");
    print(mul(t3, t4));
    printf("---\n");
    print(mul(t4, t3));
}
