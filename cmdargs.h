
#ifndef CMDARGS_H
#define CMDARGS_H
#include <stdio.h>

typedef struct struct_CommandSpec CommandSpec;
typedef struct struct_ParsedArguments ParsedArguments;
typedef int (*CommandExecutor)(ParsedArguments*);

typedef enum {
    TYPE_FLOAT,
    TYPE_INTEGER,
    TYPE_FILE,
    TYPE_STRING,
    TYPE_BOOLEAN
} OptionsType;

typedef enum {
    FLAG_LONG_NAME_CASE_ISENSITIVE = 1,
    FLAG_REPEAT = 2,
    FLAG_FILE_READ = 4,
    FLAG_FILE_WRITE = 8
} CommandOptionsFlags;

void *ParsedArguments_get(ParsedArguments *args, char *key);
int ParsedArguments_count(ParsedArguments *args, char *key);
#define ParsedArguments_getInteger(args, key) (*(int*)ParsedArguments_get(args, key))
#define ParsedArguments_getBoolean(args, key) (*(int*)ParsedArguments_get(args, key))
#define ParsedArguments_getFloat(args, key) (*(float*)ParsedArguments_get(args, key))
#define ParsedArguments_getString(args, key) ((char*)ParsedArguments_get(args, key))
#define ParsedArguments_getFile(args, key) ((FILE*)ParsedArguments_get(args, key))

void *defaultValueFloat(float value);
void *defaultValueInteger(int value);
void *defaultValueString(char *value);

void CommandSpec_showHelp(CommandSpec *spec);

CommandSpec *CommandSpec_createRootCommand(char *description);
CommandSpec *CommandSpec_createSubCommand(CommandSpec *base, char* name, CommandExecutor executor, char *description);
void CommandSpec_addOption(CommandSpec *dst, char *longName, char* shortName, char *accessKey, OptionsType type, CommandOptionsFlags flags, void* defaultValue, char *decsription);
void CommandSpec_addArgument(CommandSpec *dst, char *name, char *accessKey, OptionsType type, CommandOptionsFlags flags, char *description);
int CommandSpec_exec(CommandSpec *spec, int argc, char ** argv);

#endif