// wxamFuzzer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>

HMODULE hDll_win;
HMODULE hDll_voip;
//char* inputBuf;

// Wxam -> JPEG: flag = 0
// Wxam -> Gif: flag = ?
typedef bool (__cdecl* _isWxGF)(
	unsigned char* fileBuffer,
	DWORD flag
);

struct InputStruct {
	unsigned char* inputBuf;
	uint32_t inputBuf_sz;
};

typedef bool(__fastcall* _wxam_decoder_helper_DecodeWxamToJpeg)(
	InputStruct* fileBuffer,
	int* pOut
);

unsigned char* sample_bytes;
unsigned char* shm_data;
#define MAX_SAMPLE_SIZE 1000000
#define SHM_SIZE (4 + MAX_SAMPLE_SIZE)

_isWxGF isWxGF;
_wxam_decoder_helper_DecodeWxamToJpeg wxam_decoder_helper_DecodeWxamToJpeg;


int setup_shmem(const wchar_t* name) {
	HANDLE map_file;

	map_file = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		name);            // name of mapping object

	if (map_file == NULL) {
		printf("Error accessing shared memory\n");
		return 0;
	}

	shm_data = (unsigned char*)MapViewOfFile(map_file, // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0,
		0,
		SHM_SIZE);

	if (shm_data == NULL) {
		printf("Error accessing shared memory\n");
		return 0;
	}
	// Optimization: setup shared memory as a global page we just overwrite at each fuzz iteration
	// as opposed to malloc/free all the time
	sample_bytes = (unsigned char*)calloc(1,MAX_SAMPLE_SIZE);


	return 1;
}

void init() {
	// Load target DLLs
	hDll_voip = LoadLibraryA("voipEngine.dll");
	if (hDll_voip == NULL) {
		printf("Failed to load voipEngine.dll\n");
		exit(1);
	}
	hDll_win = LoadLibraryA("WeChatWin.dll");
	if (hDll_win == NULL) {
		printf("Failed to load WeChatWin.dll, err:%x\n", GetLastError());
		exit(1);
	}
	// Create function pointers
	isWxGF = (_isWxGF)GetProcAddress(hDll_voip, "isWxGF");
	wxam_decoder_helper_DecodeWxamToJpeg = (_wxam_decoder_helper_DecodeWxamToJpeg)((unsigned int)hDll_win + 0x7D3DA0);
}

/*
void test_load_buffer() {
	// Load file from hardcoded location
	HANDLE hFile = CreateFileW(
		L"C:\\dev\\file.bin",
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("Failed to open C:\\dev\\file.bin\n");
		exit(1);
	}
	LARGE_INTEGER totalSize = {};
	BOOL res = GetFileSizeEx(hFile, &totalSize);
	if (!res) {
		printf("GetFileSizeEx failed!\n");
		exit(2);
	}
	DWORD bRet = 0;
	inputBuf = (char*)calloc(1, totalSize.QuadPart);
	res = ReadFile(hFile, inputBuf, totalSize.QuadPart, &bRet, NULL);
	if (!res) {
		printf("ReadFile failed!\n");
		exit(3);
	}
	if (bRet != totalSize.QuadPart) {
		printf("Read:%d bytes, expected:%d bytes\n", bRet, totalSize.QuadPart);
	}
	return;
}
*/


#pragma optimize( "", off )
extern "C"
__declspec(dllexport)
bool fuzz() {

	uint32_t sample_size = 0;
	sample_size = *(uint32_t*)(shm_data);
	if (sample_size > MAX_SAMPLE_SIZE) sample_size = MAX_SAMPLE_SIZE-4;
	// Update the bytes of our buffer
	memcpy(sample_bytes, shm_data + sizeof(uint32_t), sample_size);
	// Input struct is actually: 
	// 0: pointer to input buffer
	// 1: input buffer size
	// so, lets create that:
	InputStruct inputStruct;
	inputStruct.inputBuf = sample_bytes;
	inputStruct.inputBuf_sz = sample_size;


	
	// Call first function as per wxam 
	bool res = isWxGF(inputStruct.inputBuf, sample_size);
	// 1 = error, 0 = success
	if (res) {
		//printf("isWxGF failed\n");
		return false;
	}
	int pOut = 0;
	res = wxam_decoder_helper_DecodeWxamToJpeg(&inputStruct, &pOut);
	// 1 = success, 0 = error
	return res;

	
}
#pragma optimize( "", on )

int wmain(int argc, wchar_t* argv[])
{
	init();
	setup_shmem(argv[1]);
	while (true) {
		fuzz();
	}
	/*
	// 1 = success, 0 = fail
	if (res) {
		printf("Finished fuzzing successfully!\n");
	}
	else {
		printf("Fuzzing returned an error!\n");
	}
	*/
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
