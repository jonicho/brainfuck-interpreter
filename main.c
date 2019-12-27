#include <stdlib.h>
#include <stdio.h>

char data[2048];
int data_ptr = 0;
FILE *program_file;

enum InstructionType { Nop, Left, Right, Add, Subtract, Print, Read, Loop };

typedef struct Instruction {
	enum InstructionType type;
	int value;
	struct Instruction *next;
	struct Instruction *loop;
} Instruction;

Instruction *program;

Instruction *parse_program()
{
	Instruction *first_inst = malloc(sizeof(Instruction));
	first_inst->type = Nop;
	Instruction *current_inst = first_inst;
	int value;
	char c;
	while (!feof(program_file)) {
		c = fgetc(program_file);
		if (c == ']') {
			current_inst->next = NULL;
			return first_inst;
		}
		Instruction *next_inst = malloc(sizeof(Instruction));
		value = 1;
		switch (c) {
		case '<':
			while (fgetc(program_file) == '<') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_inst->type = Left;
			next_inst->value = value;
			break;
		case '>':
			while (fgetc(program_file) == '>') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_inst->type = Right;
			next_inst->value = value;
			break;
		case '+':
			while (fgetc(program_file) == '+') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_inst->type = Add;
			next_inst->value = value;
			break;
		case '-':
			while (fgetc(program_file) == '-') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_inst->type = Subtract;
			next_inst->value = value;
			break;
		case '.':
			next_inst->type = Print;
			break;
		case ',':
			next_inst->type = Read;
			break;
		case '[':
			next_inst->type = Loop;
			next_inst->loop = parse_program();
			break;
		default:
			break;
		}
		current_inst->next = next_inst;
		current_inst = next_inst;
	}
	return first_inst;
}

void run_instruction(Instruction *instruction)
{
	while (instruction != NULL) {
		switch (instruction->type) {
		case Nop:
			break;
		case Left:
			data_ptr -= instruction->value;
			break;
		case Right:
			data_ptr += instruction->value;
			break;
		case Add:
			data[data_ptr] += instruction->value;
			break;
		case Subtract:
			data[data_ptr] -= instruction->value;
			break;
		case Print:
			putchar(data[data_ptr]);
			break;
		case Read:
			data[data_ptr] = getchar();
			break;
		case Loop:
			while (data[data_ptr]) {
				run_instruction(instruction->loop);
			}
			break;
		default:
			break;
		}
		instruction = instruction->next;
	}
}

Instruction *remove_nops(Instruction *instruction)
{
	if (instruction == NULL) {
		return NULL;
	}
	switch (instruction->type) {
	case Nop: {
		Instruction *tmp = instruction->next;
		free(instruction);
		return remove_nops(tmp);
	}
	case Loop:
		instruction->loop = remove_nops(instruction->loop);
	default:
		instruction->next = remove_nops(instruction->next);
		return instruction;
	}
}

void optimize()
{
	program = remove_nops(program);
}

void run_program()
{
	program = parse_program();
	for (int i = 0; i < sizeof data; i++) {
		data[i] = 0;
	}
	optimize();
	run_instruction(program);
}

void main(int argc, char const *argv[])
{
	if (argc < 2) {
		printf("Usage: %s file\n", argv[0]);
		exit(0);
	}

	program_file = fopen(argv[1], "r");
	if (program_file == NULL) {
		printf("Failed to open file!\n");
		exit(EXIT_FAILURE);
	}

	run_program();
}
