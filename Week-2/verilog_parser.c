#include "verilog_parser.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ACCUMULATOR_SIZE (MAX_LINE_LENGTH * 10) // For multi-line declarations

// --- Global Variable Definitions ---
// Header data
char module_name[MAX_NAME_LENGTH];
Signal module_ports[MAX_PORTS];
int module_port_count = 0;
Signal input_signals[MAX_SIGNALS];
int input_count = 0;
Signal output_signals[MAX_SIGNALS];
int output_count = 0;
Signal wire_signals[MAX_SIGNALS];
int wire_count = 0;

// Gate data
GateInstance parsed_gates[MAX_GATE_INSTANCES];
int parsed_gate_count = 0;

// --- Internal State Variables (static to this file) ---
typedef enum { // Keep ParseState local if only used here
    PARSING_NONE,
    PARSING_MODULE_PORTS,
    PARSING_INPUTS,
    PARSING_OUTPUTS,
    PARSING_WIRES
} ParseState;

static ParseState current_parsing_state = PARSING_NONE;
static char accumulator[MAX_ACCUMULATOR_SIZE]; // MAX_ACCUMULATOR_SIZE defined as in previous step
static int accumulator_len = 0;


// --- Static (Private) Helper Functions ---
// trim_token, trim_whitespace_only, process_accumulated_signals (from previous step)

// (Keep existing static helper functions: trim_token, trim_whitespace_only, process_accumulated_signals)
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

// Processes the content of the accumulator to extract signal names for header declarations
static void process_accumulated_signals(Signal signals[], int *count, int max_elements) {
    char *token;
    // Create a mutable copy of the accumulator for strtok
    char temp_accumulator[MAX_ACCUMULATOR_SIZE];
    strncpy(temp_accumulator, accumulator, MAX_ACCUMULATOR_SIZE -1);
    temp_accumulator[MAX_ACCUMULATOR_SIZE -1] = '\0';

    char *trimmed_list_content = trim_whitespace_only(temp_accumulator);


    char *saveptr; // For strtok_r
    token = strtok_r(trimmed_list_content, ", \t\n", &saveptr);
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
        token = strtok_r(NULL, ", \t\n", &saveptr);
    }
    accumulator[0] = '\0';
    accumulator_len = 0;
    current_parsing_state = PARSING_NONE; // Reset state after processing
}


// New helper to convert gate type string to enum (case-insensitive for keywords)
static GateType string_to_gate_type(const char* str) {
    // For robust comparison, convert str to lower case or use strcasecmp if available/portable
    // Assuming Verilog keywords are lowercase as in "nand" example:
    if (strcmp(str, "and") == 0) return GATE_AND;
    if (strcmp(str, "nand") == 0) return GATE_NAND;
    if (strcmp(str, "or") == 0) return GATE_OR;
    if (strcmp(str, "nor") == 0) return GATE_NOR;
    if (strcmp(str, "xor") == 0) return GATE_XOR;
    if (strcmp(str, "xnor") == 0) return GATE_XNOR;
    if (strcmp(str, "not") == 0) return GATE_NOT;
    if (strcmp(str, "buff") == 0 || strcmp(str, "buf") == 0) return GATE_BUFF; // Allow "buff" and "buf"
    return GATE_UNKNOWN;
}

// New helper to check if a token is a gate type
static int is_gate_type_keyword(const char* str) {
    return string_to_gate_type(str) != GATE_UNKNOWN;
}

// New helper to parse a gate instantiation line
static void parse_gate_instantiation_line(const char* line_content, const char* gate_keyword) {
    if (parsed_gate_count >= MAX_GATE_INSTANCES) {
        fprintf(stderr, "Warning: Maximum gate instances limit reached (%d).\n", MAX_GATE_INSTANCES);
        return;
    }
    GateInstance* current_gate = &parsed_gates[parsed_gate_count];
    current_gate->type = string_to_gate_type(gate_keyword);
    current_gate->input_signal_count = 0;
    current_gate->instance_number = -1; // Default if not found or invalid
    current_gate->base_name[0] = '\0';

    const char* ptr = line_content + strlen(gate_keyword); // Move past gate keyword

    // 1. Extract Instance Name
    while (*ptr && isspace((unsigned char)*ptr)) ptr++; // Skip spaces
    if (!*ptr || *ptr == '(') {
        fprintf(stderr, "Error: Missing instance name for gate type %s on line: %s\n", gate_keyword, line_content);
        return;
    }
    const char* instance_name_start = ptr;
    while (*ptr && !isspace((unsigned char)*ptr) && *ptr != '(') ptr++; // Read until space or '('
    size_t name_len = ptr - instance_name_start;
    if (name_len == 0) {
         fprintf(stderr, "Error: Empty instance name for gate type %s on line: %s\n", gate_keyword, line_content);
        return;
    }
    if (name_len >= MAX_GATE_INSTANCE_NAME_LEN) name_len = MAX_GATE_INSTANCE_NAME_LEN - 1;
    strncpy(current_gate->instance_name, instance_name_start, name_len);
    current_gate->instance_name[name_len] = '\0';

    // Parse instance_name into base_name and instance_number
    char* last_underscore = strrchr(current_gate->instance_name, '_');
    if (last_underscore) {
        size_t base_name_len = last_underscore - current_gate->instance_name;
        if (base_name_len < MAX_GATE_BASE_NAME_LEN) {
            strncpy(current_gate->base_name, current_gate->instance_name, base_name_len);
            current_gate->base_name[base_name_len] = '\0';
        } else {
            strncpy(current_gate->base_name, current_gate->instance_name, MAX_GATE_BASE_NAME_LEN - 1);
            current_gate->base_name[MAX_GATE_BASE_NAME_LEN - 1] = '\0';
            // fprintf(stderr, "Warning: Gate base name truncated: %s\n", current_gate->instance_name);
        }
        
        char* num_part = last_underscore + 1;
        if (strlen(num_part) > 0) {
            int is_all_digits = 1;
            for (size_t i = 0; i < strlen(num_part); ++i) {
                if (!isdigit((unsigned char)num_part[i])) {
                    is_all_digits = 0;
                    break;
                }
            }
            if (is_all_digits) {
                current_gate->instance_number = atoi(num_part);
            } else {
                 // Not a number, so treat full name as base, no instance number effectively
                strncpy(current_gate->base_name, current_gate->instance_name, MAX_GATE_BASE_NAME_LEN - 1);
                current_gate->base_name[MAX_GATE_BASE_NAME_LEN-1] = '\0';
                current_gate->instance_number = -1; // Or some other indicator for "not a numeric suffix"
            }
        }
    } else { // No underscore
        strncpy(current_gate->base_name, current_gate->instance_name, MAX_GATE_BASE_NAME_LEN - 1);
        current_gate->base_name[MAX_GATE_BASE_NAME_LEN-1] = '\0';
        current_gate->instance_number = 0; // Default if no underscore, or could be -1
    }


    // 2. Extract Port List
    while (*ptr && isspace((unsigned char)*ptr)) ptr++; // Skip spaces before '('
    if (*ptr != '(') {
        fprintf(stderr, "Error: Missing '(' for port list of gate instance %s on line: %s\n", current_gate->instance_name, line_content);
        return;
    }
    ptr++; // Skip '('
    
    const char* port_list_content_start = ptr;
    const char* port_list_content_end = strrchr(port_list_content_start, ')'); // Find last ')'
    if (!port_list_content_end) {
        fprintf(stderr, "Error: Missing ')' for port list of gate instance %s on line: %s\n", current_gate->instance_name, line_content);
        return;
    }

    char port_list_buffer[MAX_LINE_LENGTH]; // Assuming MAX_LINE_LENGTH is sufficient for the port list part
    size_t port_list_len = port_list_content_end - port_list_content_start;
    if (port_list_len >= MAX_LINE_LENGTH) port_list_len = MAX_LINE_LENGTH - 1;
    strncpy(port_list_buffer, port_list_content_start, port_list_len);
    port_list_buffer[port_list_len] = '\0';

    // Tokenize port_list_buffer
    char *port_saveptr;
    char *port_token = strtok_r(port_list_buffer, ", \t\n", &port_saveptr);

    // First token is the output signal
    if (!port_token) {
        fprintf(stderr, "Error: Empty port list for gate instance %s\n", current_gate->instance_name);
        return;
    }
    char* clean_port_token = trim_token(port_token); // Use existing trim_token
    strncpy(current_gate->output_signal, clean_port_token, MAX_NAME_LENGTH - 1);
    current_gate->output_signal[MAX_NAME_LENGTH - 1] = '\0';

    // Subsequent tokens are input signals
    port_token = strtok_r(NULL, ", \t\n", &port_saveptr);
    while(port_token) {
        if (current_gate->input_signal_count >= MAX_GATE_INPUTS) {
            fprintf(stderr, "Warning: Max gate inputs (%d) reached for instance %s.\n", MAX_GATE_INPUTS, current_gate->instance_name);
            break;
        }
        clean_port_token = trim_token(port_token);
        if (strlen(clean_port_token) > 0) {
            strncpy(current_gate->input_signals[current_gate->input_signal_count].name, clean_port_token, MAX_NAME_LENGTH - 1);
            current_gate->input_signals[current_gate->input_signal_count].name[MAX_NAME_LENGTH - 1] = '\0';
            current_gate->input_signal_count++;
        }
        port_token = strtok_r(NULL, ", \t\n", &port_saveptr);
    }
    parsed_gate_count++;
}


// --- Public Function Implementations ---

void reset_parsed_data(void) {
    module_name[0] = '\0';
    module_port_count = 0;
    input_count = 0;
    output_count = 0;
    wire_count = 0;
    parsed_gate_count = 0; // Reset gate count

    current_parsing_state = PARSING_NONE;
    accumulator[0] = '\0';
    accumulator_len = 0;
}

// Renamed from parse_verilog_header_declarations
int parse_verilog_module(const char *filename) {
    FILE *file;
    char line_buffer[MAX_LINE_LENGTH]; // MAX_LINE_LENGTH defined as in previous step
    char *remaining_line_part = NULL;

    reset_parsed_data();

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening Verilog file");
        return 1;
    }
    accumulator[0] = '\0';

    while (1) {
        char *current_line_ptr;
        if (remaining_line_part && *remaining_line_part != '\0') {
            current_line_ptr = remaining_line_part;
            remaining_line_part = NULL;
        } else {
            if (!fgets(line_buffer, sizeof(line_buffer), file)) {
                if (current_parsing_state != PARSING_NONE && accumulator_len > 0) {
                     fprintf(stderr, "Warning: EOF reached while still accumulating list for %d. Incomplete declaration.\n", current_parsing_state);
                }
                break;
            }
            current_line_ptr = line_buffer;
        }

        char *segment_to_process = trim_whitespace_only(current_line_ptr);

        if (current_parsing_state != PARSING_NONE) {
            // (Logic for accumulating multi-line header declarations - unchanged from previous)
            if (accumulator_len > 0 && accumulator_len < MAX_ACCUMULATOR_SIZE -1 && strlen(segment_to_process) > 0) {
                accumulator[accumulator_len++] = ' '; 
                accumulator[accumulator_len] = '\0';
            }
            strncat(accumulator, segment_to_process, MAX_ACCUMULATOR_SIZE - accumulator_len - 1);
            accumulator_len = strlen(accumulator);
            accumulator[MAX_ACCUMULATOR_SIZE - 1] = '\0';

            char *terminator_found = NULL;
            if (current_parsing_state == PARSING_MODULE_PORTS) terminator_found = strchr(accumulator, ')');
            else if (current_parsing_state == PARSING_INPUTS || current_parsing_state == PARSING_OUTPUTS || current_parsing_state == PARSING_WIRES) {
                terminator_found = strchr(accumulator, ';');
            }
            if (terminator_found) {
                remaining_line_part = terminator_found + 1;
                *terminator_found = '\0';
                if (current_parsing_state == PARSING_MODULE_PORTS) process_accumulated_signals(module_ports, &module_port_count, MAX_PORTS);
                else if (current_parsing_state == PARSING_INPUTS) process_accumulated_signals(input_signals, &input_count, MAX_SIGNALS);
                else if (current_parsing_state == PARSING_OUTPUTS) process_accumulated_signals(output_signals, &output_count, MAX_SIGNALS);
                else if (current_parsing_state == PARSING_WIRES) process_accumulated_signals(wire_signals, &wire_count, MAX_SIGNALS);
            }
            continue;
        }
        
        // If current_parsing_state == PARSING_NONE
        if (strncmp(segment_to_process, "//", 2) == 0 || strlen(segment_to_process) == 0) {
            remaining_line_part = NULL;
            continue;
        }
        
        char temp_segment_for_strtok[MAX_LINE_LENGTH];
        strncpy(temp_segment_for_strtok, segment_to_process, MAX_LINE_LENGTH -1);
        temp_segment_for_strtok[MAX_LINE_LENGTH-1] = '\0';

        char *first_token = strtok(temp_segment_for_strtok, " \t("); // Delimiters

        if (first_token == NULL) {
            remaining_line_part = NULL;
            continue;
        }
        
        accumulator[0] = '\0'; accumulator_len = 0; // Reset for any new list that might start

        if (strcmp(first_token, "module") == 0) {
            // (Module parsing logic - unchanged)
            char *module_name_token = strtok(NULL, " \t(");
            if (module_name_token) { strncpy(module_name, module_name_token, MAX_NAME_LENGTH - 1); module_name[MAX_NAME_LENGTH - 1] = '\0'; }
            else { fprintf(stderr, "Error: Module name missing.\n"); remaining_line_part = NULL; continue; }
            char *ports_content_start = strchr(segment_to_process, '(');
            if (ports_content_start) {
                ports_content_start++;
                strncpy(accumulator, ports_content_start, MAX_ACCUMULATOR_SIZE -1); accumulator[MAX_ACCUMULATOR_SIZE-1] = '\0'; accumulator_len = strlen(accumulator);
                char *ports_end = strchr(accumulator, ')');
                if (ports_end) { remaining_line_part = ports_end + 1; *ports_end = '\0'; process_accumulated_signals(module_ports, &module_port_count, MAX_PORTS); }
                else { current_parsing_state = PARSING_MODULE_PORTS; remaining_line_part = NULL; }
            } else { current_parsing_state = PARSING_MODULE_PORTS; remaining_line_part = NULL;}

        } else if (strcmp(first_token, "input") == 0 || strcmp(first_token, "output") == 0 || strcmp(first_token, "wire") == 0) {
            // (Input/output/wire parsing logic - unchanged)
            char *list_content_start = segment_to_process + strlen(first_token); list_content_start = trim_whitespace_only(list_content_start);
            strncpy(accumulator, list_content_start, MAX_ACCUMULATOR_SIZE -1); accumulator[MAX_ACCUMULATOR_SIZE-1] = '\0'; accumulator_len = strlen(accumulator);
            char *list_end = strchr(accumulator, ';');
            if (list_end) {
                remaining_line_part = list_end + 1; *list_end = '\0';
                if (strcmp(first_token, "input") == 0) process_accumulated_signals(input_signals, &input_count, MAX_SIGNALS);
                else if (strcmp(first_token, "output") == 0) process_accumulated_signals(output_signals, &output_count, MAX_SIGNALS);
                else if (strcmp(first_token, "wire") == 0) process_accumulated_signals(wire_signals, &wire_count, MAX_SIGNALS);
            } else {
                if (strcmp(first_token, "input") == 0) current_parsing_state = PARSING_INPUTS;
                else if (strcmp(first_token, "output") == 0) current_parsing_state = PARSING_OUTPUTS;
                else if (strcmp(first_token, "wire") == 0) current_parsing_state = PARSING_WIRES;
                remaining_line_part = NULL;
            }
        } else if (is_gate_type_keyword(first_token)) {
            // *** NEW: Handle Gate Instantiation ***
            // The entire gate instantiation is assumed to be on one line as per example.
            // We pass the original 'segment_to_process' because first_token came from a copy.
            parse_gate_instantiation_line(segment_to_process, first_token);
            remaining_line_part = NULL; // Assume gate line is fully consumed.
        } else if (strcmp(first_token, "endmodule") == 0) {
            // End of module, parsing for this module can stop.
            remaining_line_part = NULL; // Consume
            // Could optionally break from the while(1) loop here if desired.
        }
        else {
            // Unrecognized line when not in an accumulation state.
             if(strlen(trim_whitespace_only(first_token)) > 0) {
                 //fprintf(stderr, "Info: Skipping unrecognized line/token: %s\n", segment_to_process);
             }
            remaining_line_part = NULL;
        }
    }

    fclose(file);
    return 0;
}

const char* gate_type_to_string(GateType type) {
    switch (type) {
        case GATE_AND:  return "AND";
        case GATE_NAND: return "NAND";
        case GATE_OR:   return "OR";
        case GATE_NOR:  return "NOR";
        case GATE_XOR:  return "XOR";
        case GATE_XNOR: return "XNOR";
        case GATE_NOT:  return "NOT";
        case GATE_BUFF: return "BUFF";
        default:        return "UNKNOWN_GATE";
    }
}
