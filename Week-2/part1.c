#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 1024 // Increased for potentially long combined lines
#define MAX_ACCUMULATOR_SIZE (MAX_LINE_LENGTH * 10) // For multi-line declarations
#define MAX_NAME_LENGTH 64
#define MAX_PORTS 200   // Max ports in module declaration
#define MAX_SIGNALS 500 // Max inputs, outputs, or wires in their respective lists

typedef struct {
    char name[MAX_NAME_LENGTH];
} Signal;

// Global storage for parsed data
char module_name[MAX_NAME_LENGTH];
Signal module_ports[MAX_PORTS]; int module_port_count = 0;
Signal input_signals[MAX_SIGNALS]; int input_count = 0;
Signal output_signals[MAX_SIGNALS]; int output_count = 0;
Signal wire_signals[MAX_SIGNALS]; int wire_count = 0;

// Parser state
typedef enum {
    PARSING_NONE,
    PARSING_MODULE_PORTS,  // Accumulating module ports until ')'
    PARSING_INPUTS,        // Accumulating inputs until ';'
    PARSING_OUTPUTS,       // Accumulating outputs until ';'
    PARSING_WIRES          // Accumulating wires until ';'
} ParseState;

ParseState current_parsing_state = PARSING_NONE;
char accumulator[MAX_ACCUMULATOR_SIZE]; // Shared accumulator for the current multi-line list
int accumulator_len = 0;

// Function to trim leading/trailing whitespace and trailing commas/semicolons from a token
char *trim_token(char *str) {
    char *start = str;
    // Trim leading space
    while (isspace((unsigned char)*start)) start++;

    if (*start == 0) return start; // All spaces?

    // Trim trailing space and common Verilog list terminators like ',' or ';' from individual tokens
    char *end = start + strlen(start) - 1;
    while (end >= start && (isspace((unsigned char)*end) || *end == ',' || *end == ';')) {
        end--;
    }
    *(end + 1) = 0; // Write new null terminator

    return start;
}


// Function to trim only leading and trailing whitespace from a string
char *trim_whitespace_only(char *str) {
    char *original_end, *new_end;
    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    original_end = str + strlen(str) - 1;
    new_end = original_end;
    while (new_end > str && isspace((unsigned char)*new_end)) new_end--;

    // Write new null terminator
    *(new_end + 1) = 0;

    return str;
}


// Processes the content of the accumulator to extract signal names
void process_accumulated_signals(Signal signals[], int *count, int max_elements) {
    char *token;
    char *trimmed_list_content = trim_whitespace_only(accumulator); // Trim the whole accumulated string

    token = strtok(trimmed_list_content, ", \t\n"); // Delimiters: comma, space, tab, newline
    while (token != NULL) {
        char* clean_token = trim_token(token); // Trim whitespace and trailing punctuation from the token itself
        if (strlen(clean_token) > 0) {
            if (*count < max_elements) {
                 strncpy(signals[*count].name, clean_token, MAX_NAME_LENGTH - 1);
                 signals[*count].name[MAX_NAME_LENGTH - 1] = '\0'; // Ensure null termination
                 (*count)++;
            } else {
                fprintf(stderr, "Warning: Exceeded max signals/ports capacity for the current list.\n");
                break; // Stop processing if capacity is reached
            }
        }
        token = strtok(NULL, ", \t\n");
    }
    accumulator[0] = '\0'; // Clear accumulator for next use
    accumulator_len = 0;
    current_parsing_state = PARSING_NONE; // Reset state
}

int main(int argc, char *argv[]) {
    FILE *file;
    char line_buffer[MAX_LINE_LENGTH];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <verilog_file>\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    printf("Parsing Verilog file: %s\n\n", argv[1]);
    accumulator[0] = '\0';

    char *remaining_line_part = NULL; // To handle multiple declarations on one line

    while (1) {
        char *current_line_ptr;
        if (remaining_line_part && *remaining_line_part != '\0') {
            current_line_ptr = remaining_line_part;
            remaining_line_part = NULL; // Consume it
        } else {
            if (!fgets(line_buffer, sizeof(line_buffer), file)) {
                break; // End of file or error
            }
            current_line_ptr = line_buffer;
        }

        char *segment_to_process = trim_whitespace_only(current_line_ptr);

        // If currently accumulating a multi-line list
        if (current_parsing_state != PARSING_NONE) {
            // Append the new segment to the accumulator
            if (accumulator_len > 0 && accumulator_len < MAX_ACCUMULATOR_SIZE -1) { // Add a space if not the first part
                accumulator[accumulator_len++] = ' ';
                accumulator[accumulator_len] = '\0';
            }
            strncat(accumulator, segment_to_process, MAX_ACCUMULATOR_SIZE - accumulator_len - 1);
            accumulator_len = strlen(accumulator);
            accumulator[MAX_ACCUMULATOR_SIZE - 1] = '\0'; // Ensure null termination

            char *terminator_found = NULL;
            char expected_terminator = ' ';
            int is_module_ports_list = 0;

            if (current_parsing_state == PARSING_MODULE_PORTS) {
                terminator_found = strchr(accumulator, ')');
                expected_terminator = ')';
                is_module_ports_list = 1;
            } else if (current_parsing_state == PARSING_INPUTS ||
                       current_parsing_state == PARSING_OUTPUTS ||
                       current_parsing_state == PARSING_WIRES) {
                terminator_found = strchr(accumulator, ';');
                expected_terminator = ';';
            }

            if (terminator_found) {
                // The rest of the string after the terminator might be part of the next declaration
                remaining_line_part = terminator_found + 1;
                *terminator_found = '\0'; // Null-terminate the accumulator at the terminator

                if (is_module_ports_list) {
                    // If it was "module (...);", the semicolon is now in remaining_line_part
                    // The accumulator for module ports should only contain the ports.
                    process_accumulated_signals(module_ports, &module_port_count, MAX_PORTS);
                } else if (current_parsing_state == PARSING_INPUTS) {
                    process_accumulated_signals(input_signals, &input_count, MAX_SIGNALS);
                } else if (current_parsing_state == PARSING_OUTPUTS) {
                    process_accumulated_signals(output_signals, &output_count, MAX_SIGNALS);
                } else if (current_parsing_state == PARSING_WIRES) {
                    process_accumulated_signals(wire_signals, &wire_count, MAX_SIGNALS);
                }
                 // current_parsing_state is reset by process_accumulated_signals
            }
            // If terminator not found, or if it was found and processed, continue to next iteration
            // which will either read a new line or process 'remaining_line_part'.
            continue;
        }

        // If not in a multi-line accumulation state (i.e., PARSING_NONE)
        if (strncmp(segment_to_process, "//", 2) == 0 || strlen(segment_to_process) == 0) {
            remaining_line_part = NULL; // Discard any remainder if current line is comment/empty
            continue;
        }
        
        char temp_segment_for_strtok[MAX_LINE_LENGTH];
        strncpy(temp_segment_for_strtok, segment_to_process, MAX_LINE_LENGTH -1);
        temp_segment_for_strtok[MAX_LINE_LENGTH-1] = '\0';

        char *first_token = strtok(temp_segment_for_strtok, " \t(");

        if (first_token == NULL) {
            remaining_line_part = NULL;
            continue;
        }
        
        accumulator[0] = '\0'; accumulator_len = 0; // Reset accumulator for new list

        if (strcmp(first_token, "module") == 0) {
            char *module_name_token = strtok(NULL, " \t("); // Get module name
            if (module_name_token) {
                strncpy(module_name, module_name_token, MAX_NAME_LENGTH - 1);
                module_name[MAX_NAME_LENGTH - 1] = '\0';
            } else {
                fprintf(stderr, "Error: Module name missing.\n");
                remaining_line_part = NULL; continue;
            }

            // Find '(' in the original segment_to_process (not temp_segment_for_strtok)
            char *ports_content_start = strchr(segment_to_process, '(');
            if (ports_content_start) {
                ports_content_start++; // Move past '('
                strncpy(accumulator, ports_content_start, MAX_ACCUMULATOR_SIZE -1);
                accumulator[MAX_ACCUMULATOR_SIZE-1] = '\0';
                accumulator_len = strlen(accumulator);

                char *ports_end = strchr(accumulator, ')');
                if (ports_end) { // ')' is on the same line
                    remaining_line_part = ports_end + 1; // Save part after ')'
                    *ports_end = '\0'; // Terminate accumulator at ')'
                    process_accumulated_signals(module_ports, &module_port_count, MAX_PORTS);
                } else { // ')' not on this line, start accumulating
                    current_parsing_state = PARSING_MODULE_PORTS;
                    remaining_line_part = NULL;
                }
            } else {
                 fprintf(stderr, "Warning: Module declaration missing '('. Assuming ports follow on next lines.\n");
                 current_parsing_state = PARSING_MODULE_PORTS; // Expect '(' and ports on subsequent lines
                 remaining_line_part = NULL;
            }
        } else if (strcmp(first_token, "input") == 0 || strcmp(first_token, "output") == 0 || strcmp(first_token, "wire") == 0) {
            char *list_content_start = segment_to_process + strlen(first_token);
            list_content_start = trim_whitespace_only(list_content_start); // Trim leading space after keyword

            strncpy(accumulator, list_content_start, MAX_ACCUMULATOR_SIZE -1);
            accumulator[MAX_ACCUMULATOR_SIZE-1] = '\0';
            accumulator_len = strlen(accumulator);

            char *list_end = strchr(accumulator, ';');
            if (list_end) { // ';' is on the same line
                remaining_line_part = list_end + 1; // Save part after ';'
                *list_end = '\0'; // Terminate accumulator at ';'
                if (strcmp(first_token, "input") == 0) process_accumulated_signals(input_signals, &input_count, MAX_SIGNALS);
                else if (strcmp(first_token, "output") == 0) process_accumulated_signals(output_signals, &output_count, MAX_SIGNALS);
                else if (strcmp(first_token, "wire") == 0) process_accumulated_signals(wire_signals, &wire_count, MAX_SIGNALS);
            } else { // ';' not on this line, start accumulating
                if (strcmp(first_token, "input") == 0) current_parsing_state = PARSING_INPUTS;
                else if (strcmp(first_token, "output") == 0) current_parsing_state = PARSING_OUTPUTS;
                else if (strcmp(first_token, "wire") == 0) current_parsing_state = PARSING_WIRES;
                remaining_line_part = NULL;
            }
        } else {
            // Not a recognized keyword for starting a new list, and not in an accumulation state.
            // Could be a gate instantiation or other Verilog construct. Ignored by this parser.
            remaining_line_part = NULL; // Discard if not recognized.
        }
    }

    fclose(file);

    // Print parsed information
    printf("## Parsed Verilog Information\n\n");

    printf("### Module Name:\n%s\n\n", module_name);

    printf("### Module Ports (from module declaration):\n");
    if (module_port_count == 0) printf("None\n");
    for (int i = 0; i < module_port_count; i++) {
        printf("- %s\n", module_ports[i].name);
    }
    printf("\n");

    printf("### Input Signals:\n");
    if (input_count == 0) printf("None\n");
    for (int i = 0; i < input_count; i++) {
        printf("- %s\n", input_signals[i].name);
    }
    printf("\n");

    printf("### Output Signals:\n");
    if (output_count == 0) printf("None\n");
    for (int i = 0; i < output_count; i++) {
        printf("- %s\n", output_signals[i].name); // Print name
    }
    printf("\n");

    printf("### Wire Signals:\n");
    if (wire_count == 0) printf("None\n");
    for (int i = 0; i < wire_count; i++) {
        printf("- %s\n", wire_signals[i].name);
    }
    printf("\n");

    return 0;
}
