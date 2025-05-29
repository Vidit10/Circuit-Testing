#include "verilog_parser.h" // Your new header file
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// --- Internal Constants and Typedefs ---
#define MAX_LINE_LENGTH 1024
#define MAX_ACCUMULATOR_SIZE (MAX_LINE_LENGTH * 10) // For multi-line declarations

// Parser state for handling multi-line declarations
typedef enum {
    PARSING_NONE,
    PARSING_MODULE_PORTS,
    PARSING_INPUTS,
    PARSING_OUTPUTS,
    PARSING_WIRES
} ParseState;

// --- Global Variable Definitions (storage for extern variables) ---
char module_name[MAX_NAME_LENGTH];
Signal module_ports[MAX_PORTS];
int module_port_count = 0;

Signal input_signals[MAX_SIGNALS];
int input_count = 0;

Signal output_signals[MAX_SIGNALS];
int output_count = 0;

Signal wire_signals[MAX_SIGNALS];
int wire_count = 0;

// --- Internal State Variables (static to this file) ---
static ParseState current_parsing_state = PARSING_NONE;
static char accumulator[MAX_ACCUMULATOR_SIZE];
static int accumulator_len = 0;

// --- Static (Private) Helper Functions ---

// Trim leading/trailing whitespace and trailing commas/semicolons from a token
static char *trim_token(char *str) {
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    if (*start == 0) return start;
    char *end = start + strlen(start) - 1;
    while (end >= start && (isspace((unsigned char)*end) || *end == ',' || *end == ';')) {
        end--;
    }
    *(end + 1) = 0;
    return start;
}

// Trim only leading and trailing whitespace from a string
static char *trim_whitespace_only(char *str) {
    char *original_end, *new_end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    original_end = str + strlen(str) - 1;
    new_end = original_end;
    while (new_end > str && isspace((unsigned char)*new_end)) new_end--;
    *(new_end + 1) = 0;
    return str;
}

// Processes the content of the accumulator to extract signal names
static void process_accumulated_signals(Signal signals[], int *count, int max_elements) {
    char *token;
    char *trimmed_list_content = trim_whitespace_only(accumulator);

    token = strtok(trimmed_list_content, ", \t\n");
    while (token != NULL) {
        char* clean_token = trim_token(token);
        if (strlen(clean_token) > 0) {
            if (*count < max_elements) {
                 strncpy(signals[*count].name, clean_token, MAX_NAME_LENGTH - 1);
                 signals[*count].name[MAX_NAME_LENGTH - 1] = '\0';
                 (*count)++;
            } else {
                fprintf(stderr, "Warning: Parser exceeded max signals/ports capacity for current list.\n");
                break;
            }
        }
        token = strtok(NULL, ", \t\n");
    }
    accumulator[0] = '\0';
    accumulator_len = 0;
    current_parsing_state = PARSING_NONE; // Reset state after processing
}


// --- Public Function Implementations ---

void reset_parsed_data(void) {
    module_name[0] = '\0';
    module_port_count = 0;
    input_count = 0;
    output_count = 0;
    wire_count = 0;

    // Optionally, you might want to clear the Signal arrays themselves
    // if they contain sensitive data or for very strict re-initialization,
    // but typically resetting counts is sufficient.
    // For example:
    // memset(module_ports, 0, sizeof(module_ports));
    // memset(input_signals, 0, sizeof(input_signals));
    // ... etc.

    current_parsing_state = PARSING_NONE;
    accumulator[0] = '\0';
    accumulator_len = 0;
}

int parse_verilog_header_declarations(const char *filename) {
    FILE *file;
    char line_buffer[MAX_LINE_LENGTH];
    char *remaining_line_part = NULL;

    reset_parsed_data(); // Ensure a clean slate before parsing

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening Verilog file");
        return 1; // File open error
    }

    accumulator[0] = '\0'; // Initialize accumulator

    while (1) {
        char *current_line_ptr;
        if (remaining_line_part && *remaining_line_part != '\0') {
            current_line_ptr = remaining_line_part;
            remaining_line_part = NULL;
        } else {
            if (!fgets(line_buffer, sizeof(line_buffer), file)) {
                // If we are in the middle of accumulating and hit EOF, process what we have.
                if (current_parsing_state != PARSING_NONE && accumulator_len > 0) {
                    fprintf(stderr, "Warning: EOF reached while still accumulating a list. Attempting to process: %s\n", accumulator);
                    if (current_parsing_state == PARSING_MODULE_PORTS) {
                         // Module ports should end with ')', not just EOF
                         fprintf(stderr, "Error: Module port list did not end with ')' before EOF.\n");
                    } else { // Inputs, Outputs, Wires should ideally end with ';'
                         fprintf(stderr, "Error: Input/Output/Wire list did not end with ';' before EOF.\n");
                    }
                    // Decide if you want to attempt processing or just error out
                    // For now, we will clear the state and break.
                    // A more robust parser might try to salvage, but it indicates a syntax error.
                }
                break; // End of file or error
            }
            current_line_ptr = line_buffer;
        }

        char *segment_to_process = trim_whitespace_only(current_line_ptr);

        if (current_parsing_state != PARSING_NONE) {
            if (accumulator_len > 0 && accumulator_len < MAX_ACCUMULATOR_SIZE -1 && strlen(segment_to_process) > 0) {
                accumulator[accumulator_len++] = ' '; // Add space between accumulated parts
                accumulator[accumulator_len] = '\0';
            }
            strncat(accumulator, segment_to_process, MAX_ACCUMULATOR_SIZE - accumulator_len - 1);
            accumulator_len = strlen(accumulator);
            accumulator[MAX_ACCUMULATOR_SIZE - 1] = '\0';

            char *terminator_found = NULL;

            if (current_parsing_state == PARSING_MODULE_PORTS) {
                terminator_found = strchr(accumulator, ')');
            } else if (current_parsing_state == PARSING_INPUTS ||
                       current_parsing_state == PARSING_OUTPUTS ||
                       current_parsing_state == PARSING_WIRES) {
                terminator_found = strchr(accumulator, ';');
            }

            if (terminator_found) {
                remaining_line_part = terminator_found + 1;
                *terminator_found = '\0';

                if (current_parsing_state == PARSING_MODULE_PORTS) process_accumulated_signals(module_ports, &module_port_count, MAX_PORTS);
                else if (current_parsing_state == PARSING_INPUTS) process_accumulated_signals(input_signals, &input_count, MAX_SIGNALS);
                else if (current_parsing_state == PARSING_OUTPUTS) process_accumulated_signals(output_signals, &output_count, MAX_SIGNALS);
                else if (current_parsing_state == PARSING_WIRES) process_accumulated_signals(wire_signals, &wire_count, MAX_SIGNALS);
                // current_parsing_state is reset inside process_accumulated_signals
            }
            continue;
        }

        if (strncmp(segment_to_process, "//", 2) == 0 || strlen(segment_to_process) == 0) {
            remaining_line_part = NULL;
            continue;
        }
        
        char temp_segment_for_strtok[MAX_LINE_LENGTH]; // strtok modifies its input
        strncpy(temp_segment_for_strtok, segment_to_process, MAX_LINE_LENGTH -1);
        temp_segment_for_strtok[MAX_LINE_LENGTH-1] = '\0';

        char *first_token = strtok(temp_segment_for_strtok, " \t(");

        if (first_token == NULL) {
            remaining_line_part = NULL;
            continue;
        }
        
        accumulator[0] = '\0'; accumulator_len = 0;

        if (strcmp(first_token, "module") == 0) {
            char *module_name_token = strtok(NULL, " \t(");
            if (module_name_token) {
                strncpy(module_name, module_name_token, MAX_NAME_LENGTH - 1);
                module_name[MAX_NAME_LENGTH - 1] = '\0';
            } else {
                fprintf(stderr, "Error: Module keyword found but name is missing.\n");
                remaining_line_part = NULL; continue;
            }

            char *ports_content_start = strchr(segment_to_process, '(');
            if (ports_content_start) {
                ports_content_start++;
                strncpy(accumulator, ports_content_start, MAX_ACCUMULATOR_SIZE -1);
                accumulator[MAX_ACCUMULATOR_SIZE-1] = '\0';
                accumulator_len = strlen(accumulator);

                char *ports_end = strchr(accumulator, ')');
                if (ports_end) {
                    remaining_line_part = ports_end + 1;
                    *ports_end = '\0';
                    process_accumulated_signals(module_ports, &module_port_count, MAX_PORTS);
                } else {
                    current_parsing_state = PARSING_MODULE_PORTS;
                    remaining_line_part = NULL;
                }
            } else {
                 fprintf(stderr, "Warning: Module declaration '%s' missing '('. Assuming ports follow on next lines.\n", module_name);
                 current_parsing_state = PARSING_MODULE_PORTS;
                 remaining_line_part = NULL;
            }
        } else if (strcmp(first_token, "input") == 0 || strcmp(first_token, "output") == 0 || strcmp(first_token, "wire") == 0) {
            char *list_content_start = segment_to_process + strlen(first_token);
            list_content_start = trim_whitespace_only(list_content_start);

            strncpy(accumulator, list_content_start, MAX_ACCUMULATOR_SIZE -1);
            accumulator[MAX_ACCUMULATOR_SIZE-1] = '\0';
            accumulator_len = strlen(accumulator);

            char *list_end = strchr(accumulator, ';');
            if (list_end) {
                remaining_line_part = list_end + 1;
                *list_end = '\0';
                if (strcmp(first_token, "input") == 0) process_accumulated_signals(input_signals, &input_count, MAX_SIGNALS);
                else if (strcmp(first_token, "output") == 0) process_accumulated_signals(output_signals, &output_count, MAX_SIGNALS);
                else if (strcmp(first_token, "wire") == 0) process_accumulated_signals(wire_signals, &wire_count, MAX_SIGNALS);
            } else {
                if (strcmp(first_token, "input") == 0) current_parsing_state = PARSING_INPUTS;
                else if (strcmp(first_token, "output") == 0) current_parsing_state = PARSING_OUTPUTS;
                else if (strcmp(first_token, "wire") == 0) current_parsing_state = PARSING_WIRES;
                remaining_line_part = NULL;
            }
        } else {
            // Not a recognized keyword, and not in an accumulation state.
            // This could be a gate instantiation, 'endmodule', or other Verilog construct.
            // For header parsing, we typically ignore these if they are not declarations.
            // Or, it could be an error or unexpected content.
            // For now, we just move to the next line/segment.
            if(strlen(trim_whitespace_only(first_token)) > 0 && strcmp(first_token, "endmodule") != 0) {
                 //fprintf(stderr, "Info: Skipping unrecognized line/token: %s\n", first_token);
            }
            remaining_line_part = NULL;
        }
    }

    fclose(file);
    return 0; // Success
}
