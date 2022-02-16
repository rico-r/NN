
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h> //used for test if given path is directory
#include <sys/stat.h> //used for test if given path is readable
#ifndef _WIN32
#include <termios.h>
#endif

#include "vec.h"
#include "mat.h"
#include "layer.h"
#include "nn.h"
#include "dataset.h"
#include "cmdargs.h"

#define needMin(x) if(ac<x){fprintf(stderr,"\e[33mneed more argument\e[0m\n");return 1;}

int run=1;
void mhandler(int sig){
	if(run){
		run=0;
	}
}
int checkFile(const char* path, int read);

#define MAIN_C
#include "nn_cmd.c"
#include "ae_cmd.c"
#include "ds_cmd.c"

#include "cmdargs.h"

int main(int ac, char**args){
	layer_init();
	srand(time(0));
	CommandSpec *root = CommandSpec_createRootCommand(0);

	CommandSpec *new = CommandSpec_createSubCommand(root, "new", 0, "create new neural network");
	CommandSpec_addArgument(new, "Destination", "dst", TYPE_FILE, FLAG_FILE_WRITE, "path to save the new created neural network");

	CommandSpec *new_simple = CommandSpec_createSubCommand(new, "simple", nnNEW, "create fully connected feed-forward naural network with sigmoid activation");
	CommandSpec_addArgument(new_simple, "Layer", "layer", TYPE_INTEGER, FLAG_REPEAT, "number of neuron in the layer.");

	CommandSpec *out = CommandSpec_createSubCommand(root, "out", nnOut, "get neuralnetwork output");
	CommandSpec_addArgument(out, "NeuralNetwork", "nn", TYPE_FILE, FLAG_FILE_READ, "neural network");
	CommandSpec_addArgument(out, "Dataset", "ds", TYPE_FILE, FLAG_FILE_READ, "dataset");
	CommandSpec_addArgument(out, "Index 1", "index1", TYPE_INTEGER, 0, "index 1");
	CommandSpec_addArgument(out, "Index 2", "index2", TYPE_INTEGER, 0, "index 2");

	CommandSpec *nn = CommandSpec_createSubCommand(root, "nn", 0, 0);

	CommandSpec *nn_train = CommandSpec_createSubCommand(nn, "train", nnTRAIN, "train neuralnetwork");
	CommandSpec_addOption(nn_train, "cycle", "c", "cycle", TYPE_INTEGER, 0, defaultValueInteger(200), "number of training cycle");
	CommandSpec_addOption(nn_train, "subcycle", "sc", "subcycle", TYPE_INTEGER, 0, defaultValueInteger(50), "number of dataset used for trining per cycle");
	CommandSpec_addOption(nn_train, "learnrate", "lr", "learnrate", TYPE_FLOAT, 0, defaultValueFloat(0.5f), "adjust learning rate");
	CommandSpec_addOption(nn_train, "no-group", "n", "no-group", TYPE_BOOLEAN, 0, defaultValueInteger(0), "don't group the dataset");
	CommandSpec_addOption(nn_train, "output", "o", "output", TYPE_STRING, 0, 0, "output to this file insteadof overwrite the neural network");
	CommandSpec_addArgument(nn_train, "Dataset", "ds", TYPE_FILE, FLAG_FILE_READ, "the dataset used to train neural network");
	CommandSpec_addArgument(nn_train, "NeuralNetwork", "nn", TYPE_STRING, 0, "neural network to train");

	CommandSpec *nn_stat = CommandSpec_createSubCommand(nn, "stat", nnSTAT, "neuralnetwork statistic");
	CommandSpec_addArgument(nn_stat, "NeuralNetwork", "nn", TYPE_FILE, FLAG_FILE_READ, "neural network");
	CommandSpec_addArgument(nn_stat, "Dataset", "ds", TYPE_FILE, FLAG_FILE_READ, "dataset");

	CommandSpec *nn_weight = CommandSpec_createSubCommand(nn, "weight", nnWEIGHT, "print neural network weight");
	CommandSpec_addArgument(nn_weight, "NeuralNetwork", "nn", TYPE_FILE, FLAG_FILE_READ, "neural network");

	CommandSpec *nn_info = CommandSpec_createSubCommand(nn, "info", nnINFO, "print neural network info");
	CommandSpec_addArgument(nn_info, "NeuralNetwork", "nn", TYPE_FILE, FLAG_FILE_READ, "neural network");

	CommandSpec *nn_activation = CommandSpec_createSubCommand(nn, "activation", nnActivation, "change activation function neural network");
	CommandSpec_addArgument(nn_activation, "NeuralNetwork", "nn", TYPE_STRING, 0, "neural network");
	CommandSpec_addArgument(nn_activation, "Layer", "layer", TYPE_INTEGER, 0, "layer");
	CommandSpec_addArgument(nn_activation, "Activation", "activation", TYPE_STRING, 0, "activation function name, one of: sigmoid, softmax");

	CommandSpec *ae = CommandSpec_createSubCommand(root, "ae", 0, 0);

	CommandSpec *ae_train = CommandSpec_createSubCommand(ae, "train", aeTRAIN, "train auto encoder");
	CommandSpec_addOption(ae_train, "cycle", "c", "cycle", TYPE_INTEGER, 0, defaultValueInteger(200), "number of training cycle");
	CommandSpec_addOption(ae_train, "subcycle", "sc", "subcycle", TYPE_INTEGER, 0, defaultValueInteger(50), "number of dataset used for trining per cycle");
	CommandSpec_addOption(ae_train, "learnrate", "lr", "learnrate", TYPE_FLOAT, 0, defaultValueFloat(0.5f), "adjust learning rate");
	CommandSpec_addOption(ae_train, "no-group", "n", "no-group", TYPE_BOOLEAN, 0, defaultValueInteger(0), "don't group the dataset");
	CommandSpec_addOption(ae_train, "output", "o", "output", TYPE_STRING, 0, 0, "output to this file insteadof overwrite the neural network");
	CommandSpec_addArgument(ae_train, "Dataset", "ds", TYPE_FILE, FLAG_FILE_READ, "the dataset used to train neural network");
	CommandSpec_addArgument(ae_train, "Auto Encoder", "nn", TYPE_STRING, 0, "neural network to train");

	CommandSpec *ae_stat = CommandSpec_createSubCommand(ae, "stat", aeSTAT, "auto encoder statistic");
	CommandSpec_addArgument(ae_stat, "NeuralNetwork", "nn", TYPE_FILE, FLAG_FILE_READ, "auto encoder");
	CommandSpec_addArgument(ae_stat, "Dataset", "ds", TYPE_FILE, FLAG_FILE_READ, "dataset");

	return CommandSpec_exec(root, ac, args);
// 	if(ac<2){
// 		fprintf(stderr,"\e[33mno subcommand given\e[0m\n");
// 		goto usg;
// 	}
// 	goto start;

// 	usg:
// 	fprintf(stderr, USAGE);
// 	return -1;

// 	start:
// 	layer_init();
// 	if(!strcmp(args[1], "train")){
// 		if(nnTRAIN(ac-2, &args[2]))
// 			goto usg;
// 		return 0;
// 	}
// 	if(!strcmp(args[1], "stat")){
// 		if(nnSTAT(ac-2, &args[2]))
// 			goto usg;
// 		return 0;
// 	}
// 	if(!strcmp(args[1], "new")){
// 		if(nnNEW(ac-2, &args[2]))
// 			goto usg;
// 		return 0;
// 	}
// 	if(!strcmp(args[1], "info")){
// 		if(nnINFO(ac-2, &args[2]))
// 			goto usg;
// 		return 0;
// 	}
// 	if(!strcmp(args[1], "weight")){
// 		if(nnWEIGHT(ac-2, &args[2]))
// 			goto usg;
// 		return 0;
// 	}
// 	if(!strcmp(args[1], "sep")){
// 		if(nnSEP(ac-2, &args[2]))
// 			goto usg;
// 		return 0;
// 	}
	
// 	if(!strcmp(args[1], "ae")){
// #ifndef _WIN32
// 		if(!strcmp(args[2], "view")){
// 			if(aeVIEW(ac-3, &args[3]))
// 				goto usg;
// 			return 0;
// 		}
// #endif
// 		if(!strcmp(args[2], "train")){
// 			if(aeTRAIN(ac-3, &args[3]))
// 				goto usg;
// 			return 0;
// 		}
// 		if(!strcmp(args[2], "stat")){
// 			if(aeSTAT(ac-3, &args[3]))
// 				goto usg;
// 			return 0;
// 		}
// 		goto usg;
// 		return 0;
// 	}
	
// 	if(!strcmp(args[1], "ds")){
// 		if(!strcmp(args[2], "encode")){
// 			if(dsENCODE(ac-3, &args[3]))
// 				goto usg;
// 			return 0;
// 		}
// 		goto usg;
// 		return 0;
// 	}
	
// 	fprintf(stderr,"\e[33munknown subcommand '%s'\e[0m\n", args[1]);
// 	goto usg;
}