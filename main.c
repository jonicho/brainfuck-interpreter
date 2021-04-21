#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

enum InstructionType { Nop, Move, Add, Print, Read, Loop, Clear };

typedef struct Instruction {
	enum InstructionType type;
	int16_t value;
	int16_t offset;
	struct Instruction *next;
	struct Instruction *loop;
} Instruction;

Instruction *parse_program(FILE *program_file)
{
	Instruction *first_inst = malloc(sizeof(Instruction));
	first_inst->type = Nop;
	Instruction *current_inst = first_inst;
	int c;
	while (current_inst != NULL && (c = fgetc(program_file)) != EOF) {
		Instruction *next_inst = malloc(sizeof(Instruction));
		next_inst->offset = 0;
		next_inst->value = 1;
		switch (c) {
		case '<':
			next_inst->value = -1;
			// fallthrough
		case '>':
			next_inst->type = Move;
			break;
		case '-':
			next_inst->value = -1;
			// fallthrough
		case '+':
			next_inst->type = Add;
			break;
		case '.':
			next_inst->type = Print;
			break;
		case ',':
			next_inst->type = Read;
			break;
		case '[':
			next_inst->type = Loop;
			next_inst->loop = parse_program(program_file);
			break;
		case ']':
			next_inst = NULL;
			break;
		default:
			break;
		}
		current_inst->next = next_inst;
		current_inst = next_inst;
	}
	return first_inst;
}

void run_instruction(Instruction *instruction, uint8_t *data,
		     uint16_t *data_ptr)
{
	while (instruction != NULL) {
		switch (instruction->type) {
		case Nop:
			break;
		case Move:
			*data_ptr += instruction->value;
			break;
		case Add:
			data[*data_ptr + instruction->offset] +=
				instruction->value;
			break;
		case Print:
			putchar(data[*data_ptr + instruction->offset]);
			break;
		case Read:
			data[*data_ptr + instruction->offset] = getchar();
			break;
		case Loop:
			while (data[*data_ptr]) {
				run_instruction(instruction->loop, data,
						data_ptr);
			}
			break;
		case Clear:
			data[*data_ptr + instruction->offset] = 0;
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
		// fallthrough
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
	    instruction->loop->type == Add && instruction->loop->next == NULL) {
		instruction->type = Clear;
		free(instruction->loop);
	}
	instruction->next = optimize_clear_loops(instruction->next);
	if (instruction->type == Loop) {
		instruction->loop = optimize_clear_loops(instruction->loop);
	}
	return instruction;
}

void optimize_adjacent_instructions(Instruction *instruction)
{
	if (instruction == NULL) {
		return;
	}
	switch (instruction->type) {
	case Move:
	case Add:
		while (instruction->next != NULL &&
		       instruction->next->type == instruction->type &&
		       instruction->next->offset == instruction->offset) {
			Instruction *tmp = instruction->next;
			instruction->next = tmp->next;
			instruction->value += tmp->value;
			free(tmp);
		}
		break;
	case Loop:
		optimize_adjacent_instructions(instruction->loop);
		break;
	default:
		break;
	}
	optimize_adjacent_instructions(instruction->next);
}

Instruction *optimize_offsets(Instruction *instruction)
{
	if (instruction == NULL) {
		return NULL;
	}
	Instruction *current_inst = instruction;
	int offset = 0;
	while (current_inst != NULL && current_inst->type != Loop) {
		switch (current_inst->type) {
		case Move:
			offset += current_inst->value;
			current_inst->type = Nop;
			break;
		default:
			current_inst->offset = offset;
			break;
		}
		if (current_inst->next == NULL ||
		    current_inst->next->type == Loop) {
			break;
		}
		current_inst = current_inst->next;
	}
	if (offset != 0) {
		Instruction *tmp = current_inst;
		current_inst = malloc(sizeof(Instruction));
		current_inst->type = Move;
		current_inst->value = offset;
		current_inst->next = tmp->next;
		tmp->next = current_inst;
	}
	if (current_inst != NULL) {
		if (current_inst->type == Loop) {
			current_inst->loop =
				optimize_offsets(current_inst->loop);
		}
		current_inst->next = optimize_offsets(current_inst->next);
	}
	return instruction;
}

Instruction *optimize_program(Instruction *program)
{
	program = remove_nops(program);
	program = optimize_clear_loops(program);
	optimize_adjacent_instructions(program);
	program = optimize_offsets(program);
	// optimize_offsets generated nops; those must be removed
	program = remove_nops(program);
	return program;
}

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		printf("Usage: %s file\n", argv[0]);
		exit(0);
	}

	FILE *program_file = fopen(argv[1], "r");
	if (program_file == NULL) {
		printf("Failed to open file!\n");
		exit(EXIT_FAILURE);
	}

	Instruction *program = parse_program(program_file);
	fclose(program_file);
	program = optimize_program(program);
	uint8_t *data = malloc((UINT16_MAX + 1) * sizeof(data[0]));
	uint16_t data_ptr = 0;
	for (uint32_t i = 0; i < UINT16_MAX + 1; i++) {
		data[i] = 0;
	}
	run_instruction(program, data, &data_ptr);
}
