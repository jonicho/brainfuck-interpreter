#include <stdlib.h>
#include <stdio.h>

char data[2048];
int data_ptr = 0;
FILE *program_file;

enum InstructionType {
	Nop,
	Left,
	Right,
	Add,
	Subtract,
	Print,
	Read,
	Loop,
	Clear
};

typedef struct Instruction {
	enum InstructionType type;
	int value;
	struct Instruction *next;
	struct Instruction *loop;
} Instruction;

Instruction *program;

int nextc()
{
	int c;
	do {
		c = fgetc(program_file);
	} while (c != '>' && c != '<' && c != '+' && c != '-' && c != '[' &&
		 c != ']' && c != '.' && c != ',' && c != EOF);
	return c;
}

Instruction *parse_program()
{
	Instruction *first_inst = malloc(sizeof(Instruction));
	first_inst->type = Nop;
	Instruction *current_inst = first_inst;
	int value;
	int c;
	while (!feof(program_file)) {
		c = nextc();
		if (c == ']') {
			current_inst->next = NULL;
			return first_inst;
		}
		Instruction *next_inst = malloc(sizeof(Instruction));
		value = 1;
		switch (c) {
		case '<':
			while (nextc() == '<') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_inst->type = Left;
			next_inst->value = value;
			break;
		case '>':
			while (nextc() == '>') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_inst->type = Right;
			next_inst->value = value;
			break;
		case '+':
			while (nextc() == '+') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_inst->type = Add;
			next_inst->value = value;
			break;
		case '-':
			while (nextc() == '-') {
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
		case Clear:
			data[data_ptr] = 0;
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

Instruction *optimize_clear_loops(Instruction *instruction)
{
	if (instruction == NULL) {
		return NULL;
	}
	if (instruction->type == Loop && instruction->loop != NULL &&
	    (instruction->loop->type == Add ||
	     instruction->loop->type == Subtract) &&
	    instruction->loop->next == NULL) {
		instruction->type = Clear;
		free(instruction->loop);
	}
	instruction->next = optimize_clear_loops(instruction->next);
	if (instruction->type == Loop) {
		instruction->loop = optimize_clear_loops(instruction->loop);
	}
	return instruction;
}

void optimize()
{
	program = remove_nops(program);
	program = optimize_clear_loops(program);
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
	program = parse_program();
	fclose(program_file);
	for (int i = 0; i < sizeof data; i++) {
		data[i] = 0;
	}
	optimize();
	run_instruction(program);
}
