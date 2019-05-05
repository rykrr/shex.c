#include <ncurses.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


#define DEFAULT_COLUMN_WIDTH 8


typedef struct line {
	uint8_t *bytes,
			length;
	
	struct line *prev,
				*next;
} line_s;


typedef struct context {
	uint64_t file_size;
	
	uint8_t cursor_offset,
			column_width;
	
	struct line *file_head,
				*file_end,
				*window_head;
} context_s;


void ncinit() {
	initscr();
	noecho();
	cbreak();
	curs_set(0);
	keypad(stdscr, 1);
}


line_s *new_line(line_s *prev, line_s *next) {
	line_s *line = (line_s*) malloc(sizeof(line_s));
	
	*line = (line_s) { 
		.bytes  = malloc(sizeof(uint8_t) * DEFAULT_COLUMN_WIDTH),
		.length = 0,
		.prev   = prev,
		.next   = next
	};
	
	if((prev && prev->next != next) || (next && next->prev != prev))
		fprintf(stderr, "Warning: (new_line) Possible Memory Leak\n");
	
	if(prev)
		prev->next = line;
	
	if(next)
		next->prev = line;
	
	return line;
}


void *free_line(line_s *line) {
	if(!line)
		return NULL;
	
	if(line->prev)
		line->prev->next = line->next;
	
	if(line->next)
		line->next->prev = line->prev;
	
	free(line->bytes);
	free(line);
	return NULL;
}


void *free_all_lines(line_s *line) {
	if(!line)
		return NULL;
	
	if(line->prev) {
		line->prev->next = NULL;
		free_all_lines(line->prev);
	}
	
	if(line->next) {
		line->next->prev = NULL;
		free_all_lines(line->next);
	}
	
	free_line(line);
	return NULL;
}


context_s *new_context(const char *path) {
	FILE *file = fopen(path, "rb");
	uint8_t *buffer;
	size_t size = 0;
	
	context_s *context = (context_s*) malloc(sizeof(context_s));
	
	*context = (context_s) {
		.cursor_offset = 0,
		.column_width  = DEFAULT_COLUMN_WIDTH,
		.file_head     = NULL,
		.file_end      = NULL
	};
	
	fseek(file, 0l, SEEK_END);
	size = ftell(file);
	rewind(file);
	
	if(!size)
		goto finally;
	
	buffer = (uint8_t*) malloc(size);
	fread(buffer, size, 1, file);
	
	for(size_t pos = 0; pos < size; pos++) {
		if(!(pos % context->column_width))
			context->file_end = new_line(context->file_end, NULL);
		
		context->file_end->bytes[context->file_end->length++] = buffer[pos];
	}
	
	for(context->file_head = context->file_end;
		context->file_head->prev;
		context->file_head = context->file_head->prev);
	
	free(buffer);
	
finally:
	fclose(file);
	return context;
}


void *free_context(context_s *context) {
	if(!context)
		return NULL;
	
	free_all_lines(context->file_head);
	free(context);
	return NULL;
}


int main() {
	context_s *context = new_context("file");
	
	for(line_s *line = context->file_head; line; line = line->next) {
		for(int i = 0; i < line->length; i++)
			printf("%02X ", line->bytes[i]);
		
		for(int i = 0; i < DEFAULT_COLUMN_WIDTH-line->length; i++)
			printf("   ");
		printf("  ");
		
		for(int i = 0; i < line->length; i++) {
			char c = (char) line->bytes[i];
			
			if('0' <= c && c <= 'z')
				printf("%c ", c);
			else
				printf(". ");
		}
		
		puts("");
	}
	
	context = free_context(context);
	return 0;
}
