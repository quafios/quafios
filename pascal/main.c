#include <stdio.h>
#include <gui.h>

int all[2][1001][300]; /* 300 digit, 1001 nums, two arrays */
int (*arr)[300] = all[0];
int (*tmp)[300] = all[1];
int count;
pixbuf_t *pascal;

void init(int *num) {

    int i;
    num[0] = 1;
    for (i = 1; i < 300; i++)
        num[i] = 0;

}

void sum(int *num1, int *num2, int *output) {

    int i, carry = 0;
    for (i = 0; i < 300; i++) {
        int sum = carry + num1[i] + num2[i];
        output[i] = sum%10;
        carry = sum/10;
    }

}


void generate(int k) {

    /* generate kth level */
    if (k == 0) {
        /* 0th level */
        init(arr[0]); /* set first element to 1 */
        count = 1;
    } else {
        int i = 0;
        int (*swap)[300];
        generate(k-1);
        init(tmp[i++]); /* set first element to 1 */
        for (; i < count; i++)
            sum(arr[i], arr[i-1], tmp[i]);
        init(tmp[i]); /* set last element to 1 */
        count++;
        swap = tmp;
        tmp = arr;
        arr = swap;
    }

}

void print() {

    int i, j;
    for (i = 0; i < count; i++) {
        int f = 0;
        for (j = 299; j >= 0; j--) {
            if (arr[i][j] || f) {
                printf("%d", arr[i][j]);
                f = 1;
            }
        }
        if (!f)
            printf("%d", 0);
        printf("\n");
    }

}

int main() {

    int i, j;
    window_t *win;

    generate(1000);
    pascal = pixbuf_alloc(300, 1001);
    for (i = 0; i < count; i++) {
        for (j = 299; j >= 0; j--) {
            int g = 255-((tmp[i][j]*255)/9);
            int color = 0xFF000000 | (g<<16) | (g<<8) | g;
            pixbuf_set_pixel(pascal, 299-j, i, color);
        }
    }

    win = window_alloc("Pascal Triangle",  /* title */
                       300,                /* width */
                       500,                /* height */
                       -2,                 /* x (center) */
                       -2,                 /* y (center) */
                       0xFFC0C0C0,         /* bg color */
                       "/usr/share/icons/launcher16.png" /* iconfile */);

    pixbuf_paint(pascal, win->pixbuf, 0, 0);

    window_flush(win, 0, 0, win->pixbuf->width, win->pixbuf->height);

    gui_loop();

    return 0;

}
