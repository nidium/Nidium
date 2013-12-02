#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include "jsapi.h"
#include "NativeTypes.h"

#define MAX_FILES 8192
#define MAX_FILE_LENGTH 4096
#define MAX_LINE_LENGTH 16

typedef struct _Script {
    char *name;
    int size;
    void *data;
    _Script() : data(NULL), name(NULL), size(0) {}
} Script;

int readArgs(int argc, const char *argv[], char *inFiles[], char **outFile) 
{
    int i = 1;
    int j;

    if (argc - 1 > MAX_FILES) {
        fprintf(stderr, "More than %d files as input", MAX_FILES);
        return 1;
    }

    if (argc < 3) {
        fprintf(stderr, "Usage : js2byte input_file ... output_file");
        return 1;
    }

    *outFile = (char *)argv[argc - 1];
    argc--;

    for (j = 0; i < argc && i < MAX_FILES; i++) {
        int len = strlen(argv[i]);
        if (argv[i][len - 1] != 'j' && argv[i][len] != 's')  {
            inFiles[j] = (char *)argv[i];
        } else {
            printf("Skipping file %s (not a js file)\n", argv[i]);
            continue;
        }

        j++;
    }

    // inFiles array must be NULL terminated
    inFiles[j] = NULL;

    return 0;
}

Script encode(JSContext *cx, char *filename)
{
    Script ret;
    JSObject *gbl;

    gbl = JS_GetGlobalObject(cx);
    JS_SetOptions(cx, JSOPTION_NO_SCRIPT_RVAL);
    JS::CompileOptions options(cx);
    options.setUTF8(true)
           .setFileAndLine(filename, 1);

    js::RootedObject rgbl(cx, gbl);

    JSScript *script = JS::Compile(cx, rgbl, options, filename);
    if (!script) {
        return ret;
    }

    uint32_t len;
    void *data;

    data = JS_EncodeScript(cx, script, &len);

    printf("script %s encoded with %d size\n", filename, len);

    ret.name = filename;
    ret.size = len;
    ret.data = data;

    return ret;
}

void write(FILE *fd, const char *format, ...)
{
    va_list args; 

    va_start(args, format);
    if (!vfprintf(fd, format, args)) {
        fprintf(stderr, "Failed to write to output file.\nError is : %s\n", strerror(errno));
        exit(2);
    }
    va_end(args);
}

int run(JSContext *cx, char *inFiles[], char *outFile) 
{
    FILE *fd = fopen(outFile, "w+");
    if (!fd) {
        fprintf(stderr, "Failed to open output file %s\n", outFile);
        return 1;
    }

    // Buffer to hold an array of NativeBytecodeScript 
    // with, filename, size and a reference to bytecode
    char *structArray = (char *)malloc(MAX_FILE_LENGTH * MAX_FILES + (MAX_FILES * 512));
    int offset = 0;
    offset += sprintf(structArray, "NativeBytecodeScript __bytecodeScripts[] = {\n");

    for (int i = 0; i < MAX_FILES; i++) {
        if (inFiles[i] == NULL) break;

        Script script = encode(cx, inFiles[i]);
        if (script.data == NULL) {
            fprintf(stderr, "Failed to compile script %s\n", inFiles[i]);
            return 1;
        }
    
        /* First, write bytecode for each files */
        char name[MAX_FILE_LENGTH];
        int len = strlen(inFiles[i]);
        if (len + 2 > MAX_FILE_LENGTH) {
            fprintf(stderr, "Filename %s is too long. Max is %d", inFiles[i], MAX_FILE_LENGTH);
            return 1;
        }

        // Sanitize filename, to only contain alphanumeric character
        for (int j = 0; j < len; j++) {
            char c = inFiles[i][j];
            if (!isalpha(c)) {
                c = '_';
            }
            name[j] = c;
            if (j + 1 == len) {
                name[j + 1] = '\0';
            }
        }

        write(fd, "const uint8_t %s[] = {\n    ", name);

        uint8_t *data = (uint8_t *)script.data;
        int k = 0;
        for (int j = 0; j < script.size; j++) {
            if (k == 0) {
                write(fd, "0x%02x", data[j]);
            } else {
                write(fd, ", 0x%02x", data[j]);
            }

            k++;

            if (k % MAX_LINE_LENGTH == 0) {
                write(fd, ",\n    ");
                k = 0;
            }
        }

        write(fd, "\n};\n\n");

        /* Second, append the struct containing information to our buffer */
        offset += sprintf(structArray + offset, "%s{\"%s\", %d, %s}",
            i != 0 ? ",\n    " : "    ", 
            script.name, 
            script.size,
            name); 
    }

    sprintf(structArray + offset, "\n};\n");

    write(fd, structArray);
   
    return 0;
}

int main(int argc, const char *argv[]) 
{
    char *inFiles[MAX_FILES];
    char *outFile = NULL;
    int status;

    status = readArgs(argc, argv, inFiles, &outFile);

    if (status != 0) {
        fprintf(stderr, "No file given\n");
        return 1;
    }

    JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_USE_HELPER_THREADS);
    if (rt == NULL) {
       return 1;
    }

    JSContext *cx = JS_NewContext(rt, 8192);
    if (cx == NULL) {
       return 1;
    }

    JS_BeginRequest(cx);

    JS_SetOptions(cx, JSOPTION_NO_SCRIPT_RVAL);
    JS_SetVersion(cx, JSVERSION_LATEST);

    if (argc > 1 && argv[1][0] == 'd') {
/*
        #include "./blou"
        JSObject *gbl = JS_GetGlobalObject(cx);

        js::RootedObject rgbl(cx, gbl);
        
        NativeBytecodeScript script = __bytecodeScripts[0];

        printf("sizeof=%d size=%d\n", sizeof(framework_bar_js), script.size);
        JSScript *jscript = JS_DecodeScript(cx, (void *)script.data, sizeof(script.data), NULL, NULL);
*/
    } else {
        status = run(cx, inFiles, outFile);
    }
    JS_EndRequest(cx);

    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);

    JS_ShutDown();

    return status;
}
