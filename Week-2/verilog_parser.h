#ifndef VERILOG_PARSER_H
#define VERILOG_PARSER_H

// Define maximums for names and list sizes
// These are now part of the public interface of your parser
#define MAX_NAME_LENGTH 64
#define MAX_PORTS 200
#define MAX_SIGNALS 500 // Applies to inputs, outputs, or wires lists

// Structure to hold a signal/port name
typedef struct
{
    char name[MAX_NAME_LENGTH];
} Signal;

// --- Extern Declarations for Parsed Data ---
// These variables will be defined in verilog_parser.c
// and will hold the results of the parsing.
extern char module_name[MAX_NAME_LENGTH];
extern Signal module_ports[MAX_PORTS];
extern int module_port_count;

extern Signal input_signals[MAX_SIGNALS];
extern int input_count;

extern Signal output_signals[MAX_SIGNALS];
extern int output_count;

extern Signal wire_signals[MAX_SIGNALS];
extern int wire_count;

// --- Public Function Prototypes ---

/**
 * @brief Parses the specified gate-level Verilog file.
 *
 * This function reads the Verilog file, parses module declarations,
 * inputs, outputs, and wires, and populates the global data structures
 * (module_name, module_ports, input_signals, etc.).
 *
 * @param filename The path to the Verilog file to be parsed.
 * @return 0 on successful parsing, 1 if the file cannot be opened.
 *         Other non-zero values might indicate other parsing errors in future versions.
 */
int parse_verilog_header_declarations(const char *filename);

/**
 * @brief Resets all global parsed data structures to their initial empty state.
 *
 * Call this function if you intend to parse another file using the same
 * global data structures, to avoid data from a previous parse operation
 * from carrying over.
 */
void reset_parsed_data(void);

#endif // VERILOG_PARSER_H
