#include <stdlib.h>
#include <stdio.h>

char data[2048];
int data_ptr = 0;
FILE *program_file;

enum InstructionType { Nop, Left, Right, Add, Subtract, Print, Read, Loop };

typedef struct Instruction {
	enum InstructionType type;
	int value;
	struct Instruction *next_instruction;
	struct Instruction *loop_instruction;
} Instruction;

Instruction *program;

Instruction *parse_program()
{
	Instruction *first_instruction = malloc(sizeof(Instruction));
	first_instruction->type = Nop;
	Instruction *current_instruction = first_instruction;
	int value;
	char c;
	while (!feof(program_file)) {
		c = fgetc(program_file);
		if (c == ']') {
			current_instruction->next_instruction = NULL;
			return first_instruction;
		}
		Instruction *next_instruction = malloc(sizeof(Instruction));
		value = 1;
		switch (c) {
		case '<':
			while (fgetc(program_file) == '<') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_instruction->type = Left;
			next_instruction->value = value;
			break;
		case '>':
			while (fgetc(program_file) == '>') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_instruction->type = Right;
			next_instruction->value = value;
			break;
		case '+':
			while (fgetc(program_file) == '+') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_instruction->type = Add;
			next_instruction->value = value;
			break;
		case '-':
			while (fgetc(program_file) == '-') {
				value++;
			}
			fseek(program_file, -1L, SEEK_CUR);
			next_instruction->type = Subtract;
			next_instruction->value = value;
			break;
		case '.':
			next_instruction->type = Print;
			break;
		case ',':
			next_instruction->type = Read;
			break;
		case '[':
			next_instruction->type = Loop;
			next_instruction->loop_instruction = parse_program();
			break;
		default:
			break;
		}
		current_instruction->next_instruction = next_instruction;
		current_instruction = next_instruction;
	}
	return first_instruction;
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
				run_instruction(instruction->loop_instruction);
			}
			break;
		default:
			break;
		}
		instruction = instruction->next_instruction;
	}
}

void remove_nops_ins(Instruction *instruction)
{
	if (instruction == NULL) {
		return;
	}
	while (instruction->next_instruction != NULL &&
	       instruction->next_instruction->type == Nop) {
		Instruction *tmp = instruction->next_instruction;
		instruction->next_instruction =
			instruction->next_instruction->next_instruction;
		free(tmp);
	}
	if (instruction->type == Loop) {
		while (instruction->loop_instruction != NULL &&
		       instruction->loop_instruction->type == Nop) {
			Instruction *tmp = instruction->loop_instruction;
			instruction->loop_instruction =
				instruction->loop_instruction->next_instruction;
			free(tmp);
		}
		remove_nops_ins(instruction->loop_instruction);
	}
	remove_nops_ins(instruction->next_instruction);
}

void remove_nops()
{
	while (program->type == Nop && program->next_instruction != NULL) {
		Instruction *tmp = program;
		program = program->next_instruction;
		free(tmp);
	}
	remove_nops_ins(program);
}

void optimize()
{
	remove_nops();
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
