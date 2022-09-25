CC = cl

ASM_IN = src/asm/rop.asm
ASM_OUT = bin/rop.obj
HEADERS = headers
BIN_FOLDER = bin
BIN_NAME = Cronos.exe

all: clean compile_asm compile clean_obj

compile_asm:
	nasm -f win64 $(ASM_IN) -o bin/rop.obj

compile:
	$(CC) src/*.c $(ASM_OUT) /I $(HEADERS) /Fo:$(BIN_FOLDER)\ /Fe: $(BIN_FOLDER)\$(BIN_NAME)

clean_obj:
	del /Q $(BIN_FOLDER)\*.obj

clean:
	del /Q $(BIN_FOLDER)\*
