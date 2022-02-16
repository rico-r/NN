
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //used for test if given path is directory
#include <sys/stat.h> //used for test if given path is readable

#include "cmdargs.h"

typedef struct struct_CommandOptionsSpec CommandOptionsSpec;

struct struct_ParsedArguments {
    int count;
    char **key;
    void **value;
};

struct struct_CommandOptionsSpec {
    int flags;
    char *shortName;
    char *longName;
    char *accessKey;
    char *description;
    OptionsType type;
    void *defaultValue;
};

struct struct_CommandSpec {
    char *name;
    char *description;
    CommandExecutor executor;
    int optionsSize;
    int argumentSize;
    int subCommandSize;
    CommandOptionsSpec *options;
    CommandOptionsSpec *arguments;
    CommandSpec *subCommand;
};

void *defaultValueFloat(float value){
    float *result = (float*) malloc(sizeof(float));
    *result = value;
    return (void*) result;
}

void *defaultValueInteger(int value){
    int *result = (int*) malloc(sizeof(int));
    *result = value;
    return (void*) result;
}

void *defaultValueString(char *value){
    return strdup(value);
}

void ParsedArguments_add(ParsedArguments *args, char *key, void* value){
    int count = args->count;
    args->key = realloc(args->key, (count+1)*sizeof(char*));
    args->value = realloc(args->value, (count+1)*sizeof(void*));
    args->key[count] = key;
    args->value[count] = value;
    args->count++;
}

int ParsedArguments_set(ParsedArguments *args, char *key, void* value){
    for(int i=0; i<args->count; i++){
        if(!strcmp(args->key[i], key)){
            args->value[i] = value;
            return 1;
        }
    }
    ParsedArguments_add(args, key, value);
    return 0;
}

void *ParsedArguments_get(ParsedArguments *args, char *key){
    for(int i=0; i<args->count; i++){
        if(!strcmp(args->key[i], key)){
            args->key[i] = "";
            return args->value[i];
        }
    }
    return 0;
}

int ParsedArguments_count(ParsedArguments *args, char *key){
    int count = 0;;
    for(int i=0; i<args->count; i++){
        if(!strcmp(args->key[i], key)){
            count++;
        }
    }
    return count;
}

void CommandSpec_set(CommandSpec *dst, char *name, CommandExecutor executor, char *description){
    dst->name = name;
    dst->description = description;
    dst->executor = executor;
    dst->optionsSize = 0;
    dst->argumentSize = 0;
    dst->subCommandSize = 0;
    dst->arguments = (CommandOptionsSpec*) malloc(0);
    dst->options = (CommandOptionsSpec*) malloc(0);
    dst->subCommand = (CommandSpec*) malloc(0);
}

void pad(FILE *dst, int count){
    for(int i=0; i<count; i++){
        fputc(' ', dst);
    }
}

void CommandSpec_showHelp_(CommandSpec *spec, int depth){
    int argsWidth = 0;
    if(spec->name!=0){
        pad(stdout, depth * 2);
        printf("> %s", spec->name);
        if(spec->optionsSize>0){
            printf(" <options>");
        }
        for(int i=0; i<spec->argumentSize; i++){
            CommandOptionsSpec *opt = &spec->arguments[i];
            printf(" <%s>", opt->longName);
            int width = strlen(opt->longName);
            if(width>argsWidth){
                argsWidth = width;
            }
            if(opt->flags&FLAG_REPEAT){
                printf("...");
            }
        }
        printf("\n");
        pad(stdout, depth * 2);
        if(spec->description){
            printf("# %s\n", spec->description);
        }
    }
    if(spec->optionsSize>0){
        // printf("<options>:\n");
        for(int i=0; i<spec->optionsSize; i++){
            CommandOptionsSpec *opt = &spec->options[i];
            pad(stdout, depth * 2 + 2);
            printf("  -%s --%s # %s\n", opt->shortName, opt->longName, opt->description);
        }
    }
    for(int i=0; i<spec->argumentSize; i++){
        CommandOptionsSpec *opt = &spec->arguments[i];
        pad(stdout, depth * 2 + 2);
        printf("<%s>", opt->longName);
        pad(stdout, 1 + argsWidth - strlen(opt->longName));
        printf("%s\n", opt->description);
    }
    for(int i=0; i<spec->subCommandSize; i++){
        CommandSpec_showHelp_(&spec->subCommand[i], depth + 1);
    }
}

void CommandSpec_showHelp(CommandSpec *spec){
    CommandSpec_showHelp_(spec, 0);
}

CommandSpec *CommandSpec_createRootCommand(char *description){
    CommandSpec *result = (CommandSpec*) calloc(1, sizeof(CommandSpec));
    CommandSpec_set(result, (char*) 0, (CommandExecutor) 0, description);
    return result;
}

CommandSpec *CommandSpec_createSubCommand(CommandSpec *base, char* name, CommandExecutor executor, char *description){
    int count = base->subCommandSize++;
    base->subCommand = (CommandSpec*) realloc(base->subCommand, (count+1)*sizeof(CommandSpec));
    CommandSpec_set(&base->subCommand[count], name, executor, description);
    return &base->subCommand[count];
}

void CommandSpec_addOption(CommandSpec *dst, char *longName, char* shortName, char *accessKey, OptionsType type, CommandOptionsFlags flags, void* defaultValue, char *decsription){
    int count = dst->optionsSize++;
    dst->options = (CommandOptionsSpec*) realloc(dst->options, (count+1)*sizeof(CommandOptionsSpec));
    CommandOptionsSpec *opt = &dst->options[count];
    opt->longName = longName;
    opt->shortName = shortName;
    opt->description = decsription;
    opt->accessKey = accessKey;
    opt->type = type;
    opt->flags = flags;
    opt->defaultValue = defaultValue;
}

void CommandSpec_addArgument(CommandSpec *dst, char *name, char *accessKey, OptionsType type, CommandOptionsFlags flags, char *description){
    int count = dst->argumentSize++;
    dst->arguments = (CommandOptionsSpec*) realloc(dst->arguments, (count+1)*sizeof(CommandOptionsSpec));
    CommandOptionsSpec *opt = &dst->arguments[count];
    opt->longName = name;
    opt->shortName = (char*) 0;
    opt->accessKey = accessKey;
    opt->type = type;
    opt->flags = flags;
    opt->description = description;
    opt->defaultValue = (void*) 0;
}

int checkFile(const char* path, int read){
	char* msg;
	if(!access(path, F_OK)){
		struct stat st;
		stat(path, &st);
		if(S_ISDIR(st.st_mode)){
			msg="is directory";
			goto err;
		}
		if(read){
			if(access(path, R_OK)){
				msg="file unreadable";
				goto err;
			}
		}else{
			if(access(path, W_OK)){
				msg="file unwritable";
				goto err;
			}
		}
	}else{
		if(read){
			msg="no such file or directory";
			goto err;
		}
	}
	return 0;
	
	err:
	fprintf(stderr, "error: \"%s\" %s\n", path, msg);
	return 1;
}

int CommandOptionsSpec_parse(CommandOptionsSpec *opt, ParsedArguments *dst, int *index, int argc, char **argv){
    switch(opt->type){
        case TYPE_STRING:{
            if(*index>=argc){
                goto need_more_arg;
            }
            ParsedArguments_add(dst, opt->accessKey, strdup(argv[*index]));
            *index += 1;
            break;
        }
        case TYPE_FILE:{
            if(*index>=argc){
                goto need_more_arg;
            }
            int mode = 0;
            if(opt->flags&FLAG_FILE_READ){
                if(checkFile(argv[*index], 1)){
                    return 0;
                }
                mode = 1;
            }
            if(opt->flags&FLAG_FILE_WRITE){
                if(checkFile(argv[*index], 0)){
                    return 0;
                }
                mode += 2;
            }
            char *fmode = mode == 1 ? "rb" : ( mode == 2 ? "wb" : "rwb" );
            printf("%s %s\n", argv[*index], fmode);
            FILE *value = fopen(argv[*index], fmode);
            if(opt->flags&FLAG_REPEAT){
                ParsedArguments_add(dst, opt->accessKey, value);
            }else{
                ParsedArguments_set(dst, opt->accessKey, value);
            }
            *index += 1;
            break;
        }
        case TYPE_FLOAT:{
            if(*index>=argc){
                goto need_more_arg;
            }
            float *value = (float*) malloc(sizeof(float));
            sscanf(argv[*index], "%f", value);
            if(opt->flags&FLAG_REPEAT){
                ParsedArguments_add(dst, opt->accessKey, value);
            }else{
                ParsedArguments_set(dst, opt->accessKey, value);
            }
            *index += 1;
            break;
        }
        case TYPE_INTEGER:{
            if(*index>=argc){
                goto need_more_arg;
            }
            int *value = (int*) malloc(sizeof(int));
            sscanf(argv[*index], "%d", value);
            if(opt->flags&FLAG_REPEAT){
                ParsedArguments_add(dst, opt->accessKey, value);
            }else{
                ParsedArguments_set(dst, opt->accessKey, value);
            }
            *index += 1;
            break;
        }
        case TYPE_BOOLEAN:{
            int *value = (int*) ParsedArguments_get(dst, opt->accessKey);
            *value ^= 1;
            break;
        }
    }
    return 1;
    need_more_arg:
    fprintf(stderr, "need more argument for <%s>\n", opt->longName);
    return 0;
}

int CommandSpec_parseOptions(CommandSpec *spec, ParsedArguments *parsed, int skip, int *index, int argc, char **argv){
    for(int i=0; i<spec->optionsSize; i++){
        CommandOptionsSpec *opt = &spec->options[i];
        char *name;
        if(skip==1){
            name = opt->shortName;
        }else{
            name = opt->longName;
        }
        if(name==0){
            continue;
        }
        if(!strcmp(&argv[*index][skip], name)){
            *index += 1;
            return CommandOptionsSpec_parse(opt, parsed, index, argc, argv);
        }
    }
    fprintf(stderr, "Unknown option %s\n", argv[*index]);
    return 0;
}

CommandExecutor CommandSpec_exec_(int *index, CommandSpec *cmd, ParsedArguments *parsedArgs, int argc, char ** argv){
    for(int i=0; i<cmd->optionsSize; i++){
        CommandOptionsSpec *opt = &cmd->options[i];
        if(opt->defaultValue){
            ParsedArguments_add(parsedArgs, opt->accessKey, opt->defaultValue);
        }
    }
    printf(">%s\n", cmd->name);
    match:
    while(*index<argc){
        if(argv[*index][0]=='-'){
            if(argv[*index][1]=='-'){
                if(argv[*index][1]==0){
                    break;
                }
                if(!strcmp(argv[*index], "--help")){
                    goto show_help;
                }
                if(CommandSpec_parseOptions(cmd, parsedArgs, 2, index, argc, argv)){
                    goto match;
                }
            }else{
                if(CommandSpec_parseOptions(cmd, parsedArgs, 1, index, argc, argv)){
                    goto match;
                }
            }
            goto show_help;
        }
        break;
    }

    for(int i=0; i<cmd->argumentSize; i++){
        CommandOptionsSpec *arg = &cmd->arguments[i];
        if(CommandOptionsSpec_parse(arg, parsedArgs, index, argc, argv)){
            if((arg->flags&FLAG_REPEAT)!=0 && *index < argc){
                i--;
            }
        }else{
            goto show_help;
        }
    }

    CommandExecutor result = (CommandExecutor) 0;
    for(int i=0; i<cmd->subCommandSize; i++){
        CommandSpec *sub = &cmd->subCommand[i];
        if(!strcmp(sub->name, argv[*index])){
            *index += 1;
            result = CommandSpec_exec_(index, sub, parsedArgs, argc, argv);
            break;
        }
    }
    if(result==0){
        return cmd->executor;
    }
    return result;
    show_help:
    CommandSpec_showHelp(cmd);
    return 0;
}

int CommandSpec_exec(CommandSpec *spec, int argc, char ** argv){
    int index = 1;
    ParsedArguments parsedArgs = {0, malloc(0), malloc(0)};
    CommandExecutor cmd = CommandSpec_exec_(&index, spec, &parsedArgs, argc, argv);
    if(cmd){
        return cmd(&parsedArgs);
    }
    return 1;
}
