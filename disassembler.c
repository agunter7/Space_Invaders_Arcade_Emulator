#include <stdio.h>

int main(int argc, char **argv)
{
    // Open Space Invaders ROM file
    FILE *invadersFile = fopen("invaders.h", "rb");  // binary file read-only

    if(invadersFile == NULL){
        printf("Failed to open Space Invaders ROM.");
        return -1;
    }

    int start = ftell(invadersFile);
    fseek(invadersFile, 0, SEEK_END);
    int end = ftell(invadersFile);
    printf("%d\n", start);
    printf("%d\n", end);

    return 0;
}