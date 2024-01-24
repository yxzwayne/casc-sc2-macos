#include "CascLib.h"
#include <cstdio>
#include <iostream>

typedef unsigned long DWORD;
typedef DWORD *PDWORD;


// Function to extract a file from the CASC storage
static int ExtractFile(HANDLE hStorage, const char* szStorageFile, const char* szFileName) {
    HANDLE hFile = NULL;          // Storage file handle
    HANDLE handle = NULL;         // Disk file handle
    DWORD dwErrCode = ERROR_SUCCESS; // Result value

    // Open a file in the storage
    if (!CascOpenFile(hStorage, szStorageFile, 0, 0, &hFile)) {
        return errno;
    }

    // Create the target file
    handle = fopen(szFileName, "wb");
    if (!handle) {
        perror("Error opening file");
        CascCloseFile(hFile);
        return errno;
    }

    // Read the data from the file
    char szBuffer[0x10000];
    DWORD dwBytesRead = 1;
    while (dwBytesRead != 0) {
        if (!CascReadFile(hFile, szBuffer, sizeof(szBuffer), &dwBytesRead)) {
            perror("Error reading file");
            fclose(handle); // Use 'fclose'
            CascCloseFile(hFile);
            return errno; // Use 'errno' for error code
        }
        if (dwBytesRead > 0) {
            if (fwrite(szBuffer, 1, dwBytesRead, handle) != dwBytesRead) {
                perror("Error writing to file");
                fclose(handle); // Use 'fclose'
                CascCloseFile(hFile);
                return errno; // Use 'errno' for error code
            }
        }
    }

    // Cleanup
    fclose(handle); // Use 'fclose' instead of 'CloseHandle'
    CascCloseFile(hFile);
    return 0; // Return success
}

int main() {
    HANDLE hStorage = NULL;    // Open storage handle
    HANDLE hFind = NULL;       // File search handle
    CASC_FIND_DATA findData;   // Data structure for file searching
    const char* storagePath = "/Applications/StarCraft II/SC2Data/data";
    const char* outputDir = "~/sc2_extracted_audio/"; // Directory to save extracted files

    // Open the StarCraft II storage
    if (!CascOpenStorage(storagePath, 0, &hStorage)) {
        std::cerr << "Error opening storage: " << GetLastError() << std::endl;
        return 1;
    }

    // Search for audio files
    hFind = CascFindFirstFile(hStorage, "*.mp3", &findData, NULL);
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Error searching files: " << GetLastError() << std::endl;
        CascCloseStorage(hStorage);
        return 1;
    }

    do {
        char outputPath[1024];
        snprintf(outputPath, sizeof(outputPath), "%s%s", outputDir, findData.cFileName);

        if (ExtractFile(hStorage, findData.szFileName, outputPath) != ERROR_SUCCESS) {
            std::cerr << "Error extracting file: " << findData.szFileName << std::endl;
        }
    } while (CascFindNextFile(hFind, &findData));

    CascFindClose(hFind);
    CascCloseStorage(hStorage);

    return 0;
}
