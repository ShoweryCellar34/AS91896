// System Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Library Headers


// Project Headers
#include <projectData.h>

int main(int argc, char* argv[]) {
    // Output project info and argument count
    printf("Project Name:    %s\nProject Version: %s\nArg Count:       %i\n", PROJECT_NAME, PROJECT_VERSION, argc);

    // Output arguments
    for(int i = 0; i < argc; i++) {
        printf("Arg %i:           %s\n", i, argv[i]);
    }

    // Define resource file
    const char* resourceFilePath = "resources/resource.txt";

    // Open resource file
    FILE* filePointer = fopen(resourceFilePath, "r");
    if(!filePointer) {
        fprintf(stderr, "Failed to open %s: %s\n", resourceFilePath, strerror(errno));
        return EXIT_FAILURE;
    }
    printf("Opened %s\n-----------------------------\n", resourceFilePath);

    // Define buffer to store resource file contents
    char stringBuffer[128];

    // Clear error code and read resource file line by line
    errno = 0;
    while(fgets(stringBuffer, sizeof(stringBuffer), filePointer)) {
        printf("%s", stringBuffer);
    }

    // Check for reading errors
    if(errno) {
        fprintf(stderr, "Failed to read %s: %s\n", resourceFilePath, strerror(errno));
        fclose(filePointer);
        return EXIT_FAILURE;
    }

    // Close resource file
    fclose(filePointer);
    printf("-----------------------------\nClosed %s\n", resourceFilePath);

    // Exit with success
    return EXIT_SUCCESS;
}
